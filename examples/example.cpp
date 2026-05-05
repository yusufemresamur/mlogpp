
#include "src/level.hpp"
#include "src/logger.hpp"
#include "src/registry.hpp"
#include "src/sink.hpp"

int main() {
  // Log a message using the root logger.
  mlogpp::Registry::instance().root()->Info("Hello, {}!", "world");
  // Or simply:
  mlogpp::Info("Hello {}", "world!");

  auto logger = mlogpp::Registry::instance()
                    .Get("app")
                    ->AddSink(mlogpp::MakeConsoleSink())
                    .SetMinLevel(mlogpp::LogLevel::kWarn);
  logger.Info("Hello, {}!", "World");  // Will no be shown
  logger.Warn("Hello, {}!", "World 2");

  return 0;
}
