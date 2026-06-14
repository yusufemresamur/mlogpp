#include "mlogpp/sink/async_sink.hpp"
#include "mlogpp/logger.hpp"
#include "src/sink/sink.hpp"
#include <array>
#include <atomic>
#include <csignal>
#include <thread>

namespace {
std::atomic<bool> running{true};
constexpr size_t kNumThreads{10};
}  // namespace

int main() {
  using namespace std::chrono_literals;

  std::signal(SIGINT, [](int) { running.store(false); });
  std::signal(SIGTERM, [](int) { running.store(false); });

  mlogpp::DynamicLogger logger{"app"};

  // Wrap a console sink in an async sink — the calling thread enqueues records
  // and returns immediately. a background thread handles the actual writes.
  logger.AddSink(mlogpp::MakeAsyncSink(mlogpp::MakeConsoleSink()));

  // threads log continuously until SIGTERM/SIGINT.
  std::array<std::jthread, kNumThreads> threads;
  size_t thread_id{0};
  for (std::jthread& t : threads) {
    t = std::jthread([&logger, thread_id] {
      size_t count{0};
      while (running.load()) {
        logger.Info("thread {} — message {}", thread_id, count++);
        std::this_thread::sleep_for(100ms);
      }
    });
    ++thread_id;
  }

  // Block until signal received, then let jthreads drain and AsyncSink flush.
  while (running.load()) {
    std::this_thread::sleep_for(100ms);
  }
}
