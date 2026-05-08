# mlogpp

A lightweight modern C++ logging library with type-safe sinks, log levels, and thread-safe named loggers.

## Features

- **Type-safe sinks**: Add any callable as a sink without wrapper boilerplate
- **Log levels**: Trace, Debug, Info, Warn, Error, Fatal with filtering
- **Named loggers**: Registry-based logger management with shared instances
- **Thread-safe**: Concurrent access to loggers and sinks
- **Location tracking**: Automatic file/line information in log records

## Quick Start

```cpp
#include "src/registry.hpp"
#include "src/file_sink.hpp"

int main() {
  // Use root logger directly
  mlogpp::Info("Hello, {}", "world");
  
  // Create a named logger
  auto& logger = mlogpp::Registry::GetRef("app")
    .AddSink(mlogpp::MakeConsoleSink())
    .AddSink(mlogpp::MakeFileSink("app.log"))
    .SetMinLevel(mlogpp::LogLevel::kWarn);
  
  logger.Warn("Application started");
  return 0;
}
```

## Building

```bash
bazel build //...
```

## Running Examples

```bash
bazel run //examples:example
```
