
#include "src/file_sink.hpp"
#include "src/formatter.hpp"
#include "src/level.hpp"
#include "src/logger.hpp"
#include "src/registry.hpp"
#include "src/sink.hpp"

int main() {
  // Example 1: Root logger with default console sink
  std::println("1. Root Logger (Console Output):");
  auto& root = mlogpp::Registry::RootRef();
  root.Info("Application started");
  root.Warn("This is a warning from root logger");
  std::println("");

  // Example 2: Application logger with multiple sinks and different formatters
  std::println("2. Application Logger (Console + JSON File):");
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
  std::println("");

  return 0;
}
