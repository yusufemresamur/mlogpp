# mlogpp

[![Coverage](https://codecov.io/gh/yusufemresamur/mlogpp/graph/badge.svg)](https://codecov.io/gh/yusufemresamur/mlogpp)

A lightweight modern C++ logging library with type-safe sinks, log levels, and thread-safe named loggers.

## Features

- **Type-safe sinks**: Add any callable as a sink without wrapper boilerplate
- **Log levels**: Trace, Debug, Info, Warn, Error, Fatal with dynamic filtering during runtime or static filtering during compile time
- **Named loggers**: Registry-based logger management with shared instances
- **Thread-safe**: Concurrent access to loggers and sinks
- **Location tracking**: Automatic log records with `std::source_location`

## Quick Start

The registry hands out named logger instances that are shared across the application. The root logger comes pre-configured with a console sink, so it is ready to use without any setup. Additional loggers can attach multiple sinks with different formatters - here a human-readable console sink and a JSON file sink run side by side. `SetMinLevel` controls the runtime threshold; messages below it are discarded before reaching any sink.

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
  root.Warn("This is a warning from root logger");

  // Named loggers are created on first access and reused on subsequent calls
  // to GetRef with the same name — useful for sharing a logger across
  // translation units without passing it around explicitly.
  auto& app_logger = mlogpp::Registry::GetRef("app");
  app_logger
      // Console sink uses default text formatter
      .AddSink(mlogpp::MakeConsoleSink<mlogpp::DefaultFormatter>())
      // File sink uses JSON formatter for structured logging
      .AddSink(mlogpp::MakeFileSink<mlogpp::JSONFormatter>("app.jsonl"))
      // Runtime threshold: Debug and Trace calls return immediately without
      // formatting or touching the sinks.
      .SetMinLevel(mlogpp::LogLevel::kInfo);

  app_logger.Info("Initializing application components");
  app_logger.Debug("This call is runtime-filtered and never reaches a sink");
  app_logger.Warn("Configuration loaded with 2 warnings");
  app_logger.Error("Failed to connect to secondary database");

  return 0;
}
```

## Static filtering

`StaticLogger<L>` bakes the minimum level into the type via `StaticFilter<L>`. The compiler sees the `if constexpr` branch as a compile-time constant, so every call below `L` is dead-code eliminated - no branch instruction, no format string in the binary, no call site overhead at all. This makes `StaticLogger` the right choice for performance-critical paths where the verbosity will never change at runtime.

```cpp
#include "mlogpp/level.hpp"
#include "mlogpp/logger.hpp"
#include "mlogpp/sink/sink.hpp"

int main() {
  // The threshold kWarn is part of the type — it cannot be changed at runtime.
  // The compiler eliminates Trace/Debug/Info call sites entirely, which means
  // there is zero overhead compared to guarding every log call with a manual
  // #ifdef or a constexpr constant.
  mlogpp::StaticLogger<mlogpp::LogLevel::kWarn> logger{"app"};
  logger.AddSink(mlogpp::MakeConsoleSink<mlogpp::DefaultFormatter>());

  logger.Trace("never compiled in");  // dead code: eliminated at compile time
  logger.Debug("never compiled in");  // dead code: eliminated at compile time
  logger.Info("never compiled in");   // dead code: eliminated at compile time
  logger.Warn("disk usage at {}%", 85);        // emitted
  logger.Error("write failed: {}", "ENOSPC");  // emitted
  logger.Fatal("unrecoverable state");         // emitted
}
```

## Building

```bash
bazel build //...
```

## Running Examples

```bash
bazel run //examples:example
bazel run //examples:static_filter
```

## Compile and run unit tests

```bash
bazel test //...
```
