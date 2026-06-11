
#include "mlogpp/format/json_formatter.hpp"
#include "mlogpp/level.hpp"
#include "mlogpp/logger.hpp"
#include "mlogpp/registry.hpp"
#include "mlogpp/sink/file_sink.hpp"
#include "mlogpp/sink/sink.hpp"

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
