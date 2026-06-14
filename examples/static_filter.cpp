#include "mlogpp/level.hpp"
#include "mlogpp/logger.hpp"
#include "mlogpp/sink/sink.hpp"

int main() {
  // StaticLogger<kWarn> bakes the threshold into the type.
  // Any call below kWarn is removed entirely by the compiler - no branch,
  // no string, no code at the call site.
  mlogpp::StaticLogger<mlogpp::LogLevel::kWarn> logger{"app"};
  logger.AddSink(mlogpp::MakeConsoleSink<mlogpp::DefaultFormatter>());

  logger.Trace("never compiled in");  // dead code: eliminated at compile time
  logger.Debug("never compiled in");  // dead code: eliminated at compile time
  logger.Info("never compiled in");   // dead code: eliminated at compile time
  logger.Warn("disk usage at {}%", 85);        // emitted
  logger.Error("write failed: {}", "ENOSPC");  // emitted
  logger.Fatal("unrecoverable state");         // emitted
}
