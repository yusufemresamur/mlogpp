#include "mlogpp/logger.hpp"
#include "mlogpp/sink/async_sink.hpp"
#include "mlogpp/sink/file_sink.hpp"
#include "mlogpp/sink/sink.hpp"
#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <iostream>

namespace {

// Logger with a sink that does nothing. Isolates the work that happens on the
// calling thread regardless of the sink: filter checks + LogRecord construction
// (which eagerly runs std::vformat, the timestamp and the thread-id capture).
void BM_NullSinkLog(benchmark::State& state) {
  mlogpp::DynamicLogger logger{"bench"};
  logger.AddSink(mlogpp::Sink{[](mlogpp::LogRecord const&) {}});

  int i = 0;
  for (auto _ : state) {
    logger.Info("benchmark message {} {}", i++, "abcd");
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_NullSinkLog);

// Logger with a synchronous console sink. Each Info() call constructs the
// LogRecord, formats it and writes it to stdout on the calling thread, so the
// measured latency includes the full I/O cost of the write.
void BM_ConsoleSyncLog(benchmark::State& state) {
  mlogpp::DynamicLogger logger{"bench"};
  logger.AddSink(mlogpp::MakeConsoleSink());

  int i = 0;
  for (auto _ : state) {
    logger.Info("benchmark message {} {}", i++, "abcd");
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ConsoleSyncLog);

// Logger with an async sink wrapping the same console sink. Each Info() call
// constructs the LogRecord and enqueues it; a dedicated background thread does
// the formatting and writing. The measured latency is the enqueue cost only.
void BM_ConsoleAsyncLog(benchmark::State& state) {
  mlogpp::DynamicLogger logger{"bench"};
  logger.AddSink(mlogpp::MakeAsyncSink(mlogpp::MakeConsoleSink()));

  int i = 0;
  for (auto _ : state) {
    logger.Info("benchmark message {} {}", i++, "abcd");
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ConsoleAsyncLog);

// Same comparison against a real file sink, where there is genuine I/O latency
// to defer (FileSink flushes every line via std::endl). This is where the async
// wrapper is meant to pay off — unlike /dev/null, the write actually costs.
void BM_FileSyncLog(benchmark::State& state) {
  auto const path = std::filesystem::temp_directory_path() / "mlogpp_sync.log";
  mlogpp::DynamicLogger logger{"bench"};
  logger.AddSink(mlogpp::MakeFileSink(path));

  int i = 0;
  for (auto _ : state) {
    logger.Info("benchmark message {} {}", i++, "abcd");
  }
  std::filesystem::remove(path);
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_FileSyncLog);

void BM_FileAsyncLog(benchmark::State& state) {
  auto const path = std::filesystem::temp_directory_path() / "mlogpp_async.log";
  mlogpp::DynamicLogger logger{"bench"};
  logger.AddSink(mlogpp::MakeAsyncSink(mlogpp::MakeFileSink(path)));

  int i = 0;
  for (auto _ : state) {
    logger.Info("benchmark message {} {}", i++, "abcd");
  }
  std::filesystem::remove(path);
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_FileAsyncLog);

}  // namespace

int main(int argc, char** argv) {
  // Try to disable ASLR
  benchmark::MaybeReenterWithoutASLR(argc, argv);
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }

  // The console sink writes to stdout. Redirect stdout to /dev/null so the
  // terminal isn't flooded with log lines while still paying the formatting and
  // write-syscall cost. Benchmark results are routed to stderr to stay visible.
  if (std::freopen("/dev/null", "w", stdout) == nullptr) {
    std::cerr << "failed to redirect stdout to /dev/null\n";
    return 1;
  }

  benchmark::ConsoleReporter reporter;
  reporter.SetOutputStream(&std::cerr);
  reporter.SetErrorStream(&std::cerr);
  benchmark::RunSpecifiedBenchmarks(&reporter);
  benchmark::Shutdown();
  return 0;
}
