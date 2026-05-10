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
#include "src/file_sink.hpp"
#include "src/formatter.hpp"
#include "src/level.hpp"
#include "src/logger.hpp"
#include "src/registry.hpp"
#include "src/sink.hpp"

int main() {
  // Example 1: Root logger with default console sink
  auto& root = mlogpp::Registry::RootRef();
  root.Info("Application started");
  root.Warn("This is a warning from root logger");

  // Example 2: Application logger with multiple sinks and different formatters
  auto& app_logger = mlogpp::Registry::GetRef("app");
  app_logger
      // Console sink uses default text formatter
      .AddSink(mlogpp::MakeConsoleSink<mlogpp::DefaultFormatter>())
      // File sink uses JSON formatter for structured logging
      .AddSink(mlogpp::MakeFileSink<mlogpp::JSONFormatter>("app.jsonl"))
      .SetMinLevel(mlogpp::LogLevel::kInfo);

  app_logger.Info("Initializing application components");
  app_logger.Debug("Debug message (will be filtered out - min level is Info)");
  app_logger.Warn("Configuration loaded with 2 warnings");
  app_logger.Error("Failed to connect to secondary database");

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
