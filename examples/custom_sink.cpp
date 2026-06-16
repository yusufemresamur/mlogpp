#include "mlogpp/logger.hpp"
#include "mlogpp/sink/sink.hpp"
#include <print>

int main() {
  mlogpp::DynamicLogger logger{"app"};

  // Lambda sink — forwards errors to an external alerting system.
  logger.AddSink(mlogpp::Sink{[](mlogpp::LogRecord const& r) {
    if (r.level >= mlogpp::LogLevel::kError) {
      std::println(stderr, "[ALERT] {}", r.message());
    }
  }});

  logger.Info("started");
  logger.Error("disk full");  // triggers the alert
}
