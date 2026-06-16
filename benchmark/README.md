# Benchmarks

Microbenchmarks built with [Google Benchmark](https://github.com/google/benchmark).

## `log_benchmark`

Measures the per-call cost of `Logger::Info` across sink configurations. Every
iteration logs exactly one message, and each benchmark calls
`state.SetItemsProcessed(state.iterations())`, so the report includes an
`items_per_second` column — that is the **messages/sec** figure (see the caveats
below before quoting it).

| Benchmark | What it measures |
|---|---|
| `BM_NullSinkLog` | Calling-thread baseline: filter check + `LogRecord` construction (timestamp, thread id, and capturing the format args into the deferred-format thunk). The sink does nothing and never reads the message, so **no formatting happens** — this isolates the fixed per-call overhead. |
| `BM_ConsoleSyncLog` | Plain console sink. The calling thread formats the record and writes it, so the time includes formatting + the write. |
| `BM_ConsoleAsyncLog` | `AsyncSink` wrapping the console sink. The calling thread only enqueues and a background thread formats and writes. Measures **enqueue cost only**. |
| `BM_FileSyncLog` | Plain `FileSink`. Formats and writes to a real file on the calling thread. `FileSink` writes `'\n'` and flushes only on records at/above its flush level (default `kError`), not per line. |
| `BM_FileAsyncLog` | `AsyncSink` wrapping the `FileSink`. Enqueue cost only. Formatting and file I/O happen on the background thread. |

Message formatting is **deferred**: `LogRecord` stores a thunk over copies of the
arguments and only runs `std::vformat` when a sink reads `record.message()`. For
async sinks this means the formatting cost is paid by the background worker, not
the calling thread — which is why the async numbers are lower than their sync
counterparts.

The console sink writes to `stdout`, which is redirected to `/dev/null` for the
run so the terminal isn't flooded. Results print to `stderr` and stay visible.

## Running

```sh
# Optimized build — use these numbers (the default fastbuild is a DEBUG build).
bazel run -c opt //benchmark:log_benchmark
```

Useful flags:

```sh
# Stable medians + variance (results are noisy with CPU frequency scaling on).
bazel run -c opt //benchmark:log_benchmark -- \
    --benchmark_repetitions=5 --benchmark_report_aggregates_only=true

# Run a subset.
bazel run -c opt //benchmark:log_benchmark -- --benchmark_filter='Async'
```

Example output (numbers vary by machine, build mode and load):

```
-------------------------------------------------------------------------------
Benchmark                   Time             CPU   Iterations items_per_second
-------------------------------------------------------------------------------
BM_NullSinkLog           41.7 ns         41.7 ns     16691780        23.97 M/s
BM_ConsoleSyncLog         308 ns          308 ns      2254668         3.24 M/s
BM_ConsoleAsyncLog        225 ns          225 ns      3165817         4.45 M/s
BM_FileSyncLog            292 ns          292 ns      2400496         3.42 M/s
BM_FileAsyncLog           170 ns          170 ns      3869148         5.88 M/s
```

## Interpreting the numbers

- **`items_per_second` = `1 / time-per-op`** for a single thread. For the
  **synchronous** sinks this is a **sustained** rate: each call does all the work
  before returning.
- For the **async** sinks `items_per_second` is a **burst (enqueue) rate**, not a
  sustained one. The call only hands the record to the queue. The background
  worker still has to format and write it. Under sustained load the real ceiling
  is the **worker's drain rate**, and once the queue hits capacity `AsyncSink`
  **drops** records. Do not quote the async figure as sustained throughput — this
  benchmark does not measure queue drain.
- These are **single-thread** rates. Aggregate throughput under concurrent
  loggers is limited by lock contention and scales sub-linearly. Measure it with
  `BENCHMARK(...)->Threads(N)`.

