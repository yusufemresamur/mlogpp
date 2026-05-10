
#include "src/file_sink.hpp"
#include "src/formatter.hpp"
#include "src/level.hpp"
#include "src/logger.hpp"
#include "src/registry.hpp"
#include "src/sink.hpp"
#include <cassert>

int main() {
  // Log a message using the root logger (access as reference).
  mlogpp::Registry::RootRef().Info("Hello, {}!", "world");
  // Log a message using the root logger (access as shared pointer).
  mlogpp::Registry::Root()->Info("Hello, {}!", "world 2");

  // Or simply:
  mlogpp::Info("Hello {}", "world 3");

  mlogpp::Logger& logger =
      mlogpp::Registry::GetRef("app")
          .AddSink(mlogpp::MakeConsoleSink())
          .AddSink(mlogpp::MakeFileSink<mlogpp::JSONFormatter>("app.jsonl"))
          .SetMinLevel(mlogpp::LogLevel::kWarn);
  logger.Info("Hello, {}!", "world 4");  // Will no be shown
  logger.Warn("Hello, {}!", "world 5");
  logger.Error("Hello, {}!", "world 6");

  // Get the same logger again as shared_ptr
  mlogpp::LoggerPtr logger_ptr = mlogpp::Registry::Get("app");
  assert(logger_ptr.get() == &logger);

  return 0;
}
