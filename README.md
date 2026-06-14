# mlogpp

[![Coverage](https://codecov.io/gh/yusufemresamur/mlogpp/graph/badge.svg)](https://codecov.io/gh/yusufemresamur/mlogpp)

A lightweight modern C++ logging library with type-safe sinks, log levels, and thread-safe named loggers.

## Features

- **Type-safe sinks**: Add any callable as a sink without wrapper boilerplate
- **Async sink**: Wrap any sink for non-blocking background I/O with bounded queue and clean shutdown
- **Log levels**: Trace, Debug, Info, Warn, Error, Fatal with dynamic filtering at runtime or static filtering at compile time
- **Named loggers**: Registry-based logger management with shared instances
- **Thread-safe**: Concurrent access to loggers and sinks
- **Location tracking**: Automatic source location via `std::source_location`

## Requirements

- C++23 or later
- Bazel (the only supported build system)

## Integration

Add mlogpp to your `MODULE.bazel`:

```starlark
# latest by 2026-06-14: bf363d6c5eab11f9547df3c64703cf5920fe3109
# https://github.com/yusufemresamur/mlogpp/commits/main/
bazel_dep(name = "mlogpp", version = "0.0.1")
git_override(
    module_name = "mlogpp",
    remote = "https://github.com/yusufemresamur/mlogpp.git",
    commit = "bf363d6c5eab11f9547df3c64703cf5920fe3109",
)
```

Then depend on it from your target:

```starlark
cc_binary(
    name = "my_app",
    srcs = ["main.cpp"],
    deps = ["@mlogpp//:mlogpp"],
)
```

## Dynamic filtering

The registry hands out named logger instances shared across the application. The root logger is pre-configured with a console sink, so it works out of the box. Additional loggers can attach multiple sinks with independent formatters. `SetMinLevel` controls the runtime threshold — messages below it are discarded before reaching any sink, but the call sites are still compiled in (the check is a runtime branch).

Use `DynamicLogger` (the default) when the verbosity level needs to change at runtime, e.g. toggled via a flag or config reload.

```cpp
#include "mlogpp/format/json_formatter.hpp"
#include "mlogpp/level.hpp"
#include "mlogpp/logger.hpp"
#include "mlogpp/registry.hpp"
#include "mlogpp/sink/file_sink.hpp"

int main() {
  // The root logger is always available and writes to stdout out of the box.
  auto& root = mlogpp::Registry::RootRef();
  root.Info("Application started");
  root.Warn("Low memory");

  // Named loggers are created on first access and reused on subsequent calls
  // to GetRef with the same name — useful for sharing a logger across
  // translation units without passing it around explicitly.
  auto& app_logger = mlogpp::Registry::GetRef("app");
  app_logger
      .AddSink(mlogpp::MakeConsoleSink<mlogpp::DefaultFormatter>())
      .AddSink(mlogpp::MakeFileSink<mlogpp::JSONFormatter>("app.jsonl"))
      // Runtime threshold: calls below kInfo return immediately without
      // formatting or touching the sinks.
      .SetMinLevel(mlogpp::LogLevel::kInfo);

  app_logger.Info("Initializing application components");
  app_logger.Debug("This call is runtime-filtered and never reaches a sink");
  app_logger.Warn("Configuration loaded with 2 warnings");
  app_logger.Error("Failed to connect to secondary database");
}
```

Console output (DefaultFormatter):

```
[1749123456789] [INFO] [root] [main.cpp:9] - Application started
[1749123456789] [WARN] [root] [main.cpp:10] - Low memory
[1749123456790] [INFO] [app] [main.cpp:22] - Initializing application components
[1749123456790] [WARN] [app] [main.cpp:24] - Configuration loaded with 2 warnings
[1749123456790] [ERROR] [app] [main.cpp:25] - Failed to connect to secondary database
```

File output (JSONFormatter, `app.jsonl`):

```json
{"timestamp": 1749123456790, "level": "INFO", "logger_name": "app", "file": "main.cpp", "line": 22, "message": "Initializing application components"}
{"timestamp": 1749123456790, "level": "WARN", "logger_name": "app", "file": "main.cpp", "line": 24, "message": "Configuration loaded with 2 warnings"}
{"timestamp": 1749123456790, "level": "ERROR", "logger_name": "app", "file": "main.cpp", "line": 25, "message": "Failed to connect to secondary database"}
```

## Static filtering

`StaticLogger<L>` bakes the minimum level into the type via `StaticFilter<L>`. The compiler sees the `if constexpr` branch as a compile-time constant, so every call below `L` is dead-code eliminated — no branch instruction, no format string in the binary, no overhead at the call site. This is equivalent to wrapping every low-level log call with `#ifdef`, but without the macro noise.

Use `StaticLogger` when the verbosity level is fixed for the lifetime of the binary, e.g. a production build that should never emit debug output regardless of runtime state.

```cpp
#include "mlogpp/level.hpp"
#include "mlogpp/logger.hpp"
#include "mlogpp/sink/sink.hpp"

int main() {
  // The threshold kWarn is part of the type and cannot be changed at runtime.
  // Trace/Debug/Info call sites are removed entirely by the compiler.
  mlogpp::StaticLogger<mlogpp::LogLevel::kWarn> logger{"app"};
  logger.AddSink(mlogpp::MakeConsoleSink<mlogpp::DefaultFormatter>());

  logger.Trace("never compiled in");  // eliminated at compile time
  logger.Debug("never compiled in");  // eliminated at compile time
  logger.Info("never compiled in");   // eliminated at compile time
  logger.Warn("disk usage at {}%", 85);        // emitted
  logger.Error("write failed: {}", "ENOSPC");  // emitted
  logger.Fatal("unrecoverable state");         // emitted
}
```

## Async sink

`MakeAsyncSink` wraps any existing sink and moves its I/O off the calling thread. Log records are enqueued into a bounded queue. A dedicated `std::jthread` drains it in the background. Callers never block. If the queue is full, the record is dropped silently.

On destruction the background thread is stopped, all queued records are flushed, and the thread is joined before the destructor returns. No records are lost on clean shutdown.

```cpp
#include "mlogpp/logger.hpp"
#include "mlogpp/sink/async_sink.hpp"
#include "mlogpp/sink/file_sink.hpp"

int main() {
  mlogpp::DynamicLogger logger{"app"};

  // File writes happen on a background thread; callers return immediately.
  logger.AddSink(mlogpp::MakeAsyncSink(mlogpp::MakeFileSink("app.log")));

  logger.Info("this returns before the line is written to disk");
}
```

The queue capacity defaults to 8192 records and can be overridden:

```cpp
logger.AddSink(mlogpp::MakeAsyncSink(mlogpp::MakeFileSink("app.log"), 1024));
```

Any sink works as the inner target - console, file, or a custom lambda sink.

## Custom sinks

Any callable with signature `(const mlogpp::LogRecord&) -> void` satisfies the `SinkFunction` concept and can be passed directly to `AddSink`. No subclassing or wrapper is needed.

```cpp
#include "mlogpp/logger.hpp"
#include "mlogpp/sink/sink.hpp"

int main() {
  mlogpp::DynamicLogger logger{"app"};

  // Lambda sink — forwards errors to an external alerting system.
  logger.AddSink(mlogpp::Sink{[](mlogpp::LogRecord const& r) {
    if (r.level >= mlogpp::LogLevel::kError) {
      send_alert(r.message);  // your own function
    }
  }});

  logger.Info("started");
  logger.Error("disk full");  // triggers the alert
}
```

## Building

```bash
bazel build //...
```

## Running examples

```bash
bazel run //examples:example
bazel run //examples:static_filter
bazel run //examples:async_sink
```

## Tests

```bash
bazel test //...
```
