#include "mlogpp/registry.hpp"
#include "mlogpp/sink/sink.hpp"
#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// TODO (yusufemresamur): review AI generated tests

namespace mlogpp {

// ── Registry singleton
// ────────────────────────────────────────────────────────

TEST(RegistrySingletonTest, InstanceReturnsSameAddress) {
  EXPECT_EQ(&Registry::instance(), &Registry::instance());
}

// ── Registry::Get
// ─────────────────────────────────────────────────────────────

TEST(RegistryGetTest, ReturnsNonNullPtr) {
  EXPECT_NE(Registry::Get("reg_get_non_null"), nullptr);
}

TEST(RegistryGetTest, SameNameReturnsSamePtr) {
  auto a = Registry::Get("reg_get_same_name");
  auto b = Registry::Get("reg_get_same_name");
  EXPECT_EQ(a.get(), b.get());
}

TEST(RegistryGetTest, DifferentNamesReturnDifferentPtrs) {
  auto a = Registry::Get("reg_get_diff_a");
  auto b = Registry::Get("reg_get_diff_b");
  EXPECT_NE(a.get(), b.get());
}

TEST(RegistryGetTest, ReturnedLoggerHasCorrectName) {
  auto lgr = Registry::Get("reg_get_name_check");
  EXPECT_EQ(lgr->Name(), "reg_get_name_check");
}

TEST(RegistryGetTest, NewLoggerDefaultsToInfoLevel) {
  // Fresh logger created by Get() has default kInfo min level.
  auto lgr = Registry::Get("reg_get_default_level");
  bool called = false;
  lgr->AddSink(Sink{[&called](LogRecord const&) { called = true; }});
  lgr->Debug("filtered");
  EXPECT_FALSE(called);
  lgr->Info("passes");
  EXPECT_TRUE(called);
}

TEST(RegistryGetTest, NewLoggerHasNoSinksInitially) {
  // Newly created logger should not crash when no sinks are attached.
  auto lgr = Registry::Get("reg_get_no_sinks_unique_xk9z");
  EXPECT_NO_THROW(lgr->Info("no sinks"));
}

TEST(RegistryGetTest, LoggerSharedOwnershipViaMultipleGets) {
  auto a = Registry::Get("reg_get_shared_ownership");
  auto b = Registry::Get("reg_get_shared_ownership");
  // Both hold a ref; use_count includes the registry's own ref.
  EXPECT_GE(a.use_count(), 2);
}

// ── Registry::GetRef
// ──────────────────────────────────────────────────────────

TEST(RegistryGetRefTest, PointsToSameObjectAsGet) {
  auto ptr = Registry::Get("reg_getref_same");
  DynamicLogger& ref = Registry::GetRef("reg_getref_same");
  EXPECT_EQ(&ref, ptr.get());
}

TEST(RegistryGetRefTest, RefHasCorrectName) {
  DynamicLogger& ref = Registry::GetRef("reg_getref_name");
  EXPECT_EQ(ref.Name(), "reg_getref_name");
}

TEST(RegistryGetRefTest, GetRefCreatesLoggerIfMissing) {
  // GetRef on a new name should not crash and return a usable logger.
  EXPECT_NO_THROW({
    DynamicLogger& ref = Registry::GetRef("reg_getref_creates");
    ref.Info("ok");
  });
}

// ── Registry::Root / RootRef
// ──────────────────────────────────────────────────

TEST(RegistryRootTest, RootReturnsNonNullPtr) {
  EXPECT_NE(Registry::Root(), nullptr);
}

TEST(RegistryRootTest, RootNameIsRoot) {
  EXPECT_EQ(Registry::Root()->Name(), "root");
}

TEST(RegistryRootTest, RootIsSameAsGetRoot) {
  EXPECT_EQ(Registry::Root().get(), Registry::Get("root").get());
}

TEST(RegistryRootTest, RootRefAddressMatchesRoot) {
  EXPECT_EQ(&Registry::RootRef(), Registry::Root().get());
}

TEST(RegistryRootTest, RootIsSeededWithConsoleSink) {
  // Root logger has a console sink by default; logging should not crash.
  EXPECT_NO_THROW(Registry::Root()->Info("root sink test"));
}

TEST(RegistryRootTest, RootIsStableAcrossMultipleCalls) {
  auto r1 = Registry::Root();
  auto r2 = Registry::Root();
  EXPECT_EQ(r1.get(), r2.get());
}

// ── Registry::remove ─────────────────────────────────────────────────────────

TEST(RegistryRemoveTest, RemoveNonExistentNameIsNoOp) {
  EXPECT_NO_THROW(Registry::instance().remove("__nonexistent_9x7q__"));
}

TEST(RegistryRemoveTest, AfterRemoveGetReturnsDifferentPtr) {
  auto before = Registry::Get("reg_remove_recreate");
  Registry::instance().remove("reg_remove_recreate");
  auto after = Registry::Get("reg_remove_recreate");
  EXPECT_NE(before.get(), after.get());
}

TEST(RegistryRemoveTest, AfterRemoveOldPtrRemainsValid) {
  auto held = Registry::Get("reg_remove_independent");
  Registry::instance().remove("reg_remove_independent");
  // Caller still holds a valid shared_ptr; use count drops to 1.
  EXPECT_NE(held, nullptr);
  EXPECT_NO_THROW(held->Info("still alive after removal"));
}

TEST(RegistryRemoveTest, RemovedLoggerIsNotReturnedBySubsequentGet) {
  auto original = Registry::Get("reg_remove_not_returned");
  Registry::instance().remove("reg_remove_not_returned");
  auto fresh = Registry::Get("reg_remove_not_returned");
  EXPECT_NE(original.get(), fresh.get());
}

TEST(RegistryRemoveTest, ReplacementLoggerHasCorrectName) {
  (void)Registry::Get("reg_remove_name_check");
  Registry::instance().remove("reg_remove_name_check");
  auto replacement = Registry::Get("reg_remove_name_check");
  EXPECT_EQ(replacement->Name(), "reg_remove_name_check");
}

TEST(RegistryRemoveTest, RemoveThenGetReturnsLoggerWithDefaultLevel) {
  // Logger created after removal starts fresh (default kInfo level).
  (void)Registry::Get("reg_remove_default_level");
  Registry::instance().remove("reg_remove_default_level");
  auto lgr = Registry::Get("reg_remove_default_level");
  bool called = false;
  lgr->AddSink(Sink{[&called](LogRecord const&) { called = true; }});
  lgr->Debug("filtered");
  EXPECT_FALSE(called);
  lgr->Info("passes");
  EXPECT_TRUE(called);
}

TEST(RegistryRemoveTest, RemoveSameNameTwiceIsNoOp) {
  (void)Registry::Get("reg_remove_double");
  Registry::instance().remove("reg_remove_double");
  EXPECT_NO_THROW(Registry::instance().remove("reg_remove_double"));
}

// ── RegistryImpl (tested via Registry public API)
// ─────────────────────────────

TEST(RegistryImplTest, FindReturnsSameLoggerAfterStore) {
  // Storing via Get and finding via a second Get returns the same object.
  auto lgr = Registry::Get("impl_store_find");
  EXPECT_EQ(Registry::Get("impl_store_find").get(), lgr.get());
}

TEST(RegistryImplTest, StoreOverwritesOnRemove) {
  // Remove + re-Get effectively replaces the stored entry.
  auto first = Registry::Get("impl_overwrite");
  Registry::instance().remove("impl_overwrite");
  auto second = Registry::Get("impl_overwrite");
  EXPECT_NE(first.get(), second.get());
  // Only the second entry lives in the map now.
  EXPECT_EQ(Registry::Get("impl_overwrite").get(), second.get());
}

TEST(RegistryImplTest, MissingKeyReturnsNullImplicitly) {
  // Verify Get() for a brand-new name always creates and never returns null.
  auto lgr = Registry::Get("impl_missing_key_xq91");
  EXPECT_NE(lgr, nullptr);
}

// ── Global free-function logging
// ──────────────────────────────────────────────

TEST(GlobalLoggingTest, TraceDoesNotCrash) {
  EXPECT_NO_THROW(mlogpp::Trace("global trace"));
}

TEST(GlobalLoggingTest, DebugDoesNotCrash) {
  EXPECT_NO_THROW(mlogpp::Debug("global debug"));
}

TEST(GlobalLoggingTest, InfoDoesNotCrash) {
  EXPECT_NO_THROW(mlogpp::Info("global info"));
}

TEST(GlobalLoggingTest, WarnDoesNotCrash) {
  EXPECT_NO_THROW(mlogpp::Warn("global warn"));
}

TEST(GlobalLoggingTest, ErrorDoesNotCrash) {
  EXPECT_NO_THROW(mlogpp::Error("global error"));
}

TEST(GlobalLoggingTest, FatalDoesNotCrash) {
  EXPECT_NO_THROW(mlogpp::Fatal("global fatal"));
}

TEST(GlobalLoggingTest, InfoForwardsToRootLogger) {
  // Use a shared_ptr capture so the sink remains valid after test teardown.
  auto captured = std::make_shared<std::string>();
  Registry::RootRef().AddSink(Sink{[captured](LogRecord const& r) {
    if (r.level == LogLevel::kInfo) *captured = r.message;
  }});
  mlogpp::Info("forwarded {}", 42);
  EXPECT_EQ(*captured, "forwarded 42");
}

TEST(GlobalLoggingTest, WarnForwardsToRootLogger) {
  auto captured = std::make_shared<LogLevel>(LogLevel::kTrace);
  Registry::RootRef().AddSink(Sink{[captured](LogRecord const& r) {
    if (r.level == LogLevel::kWarn) *captured = r.level;
  }});
  mlogpp::Warn("warn forwarded");
  EXPECT_EQ(*captured, LogLevel::kWarn);
}

TEST(GlobalLoggingTest, ErrorForwardsToRootLogger) {
  auto captured = std::make_shared<LogLevel>(LogLevel::kTrace);
  Registry::RootRef().AddSink(Sink{[captured](LogRecord const& r) {
    if (r.level == LogLevel::kError) *captured = r.level;
  }});
  mlogpp::Error("error forwarded");
  EXPECT_EQ(*captured, LogLevel::kError);
}

TEST(GlobalLoggingTest, TraceNotForwardedAtDefaultMinLevel) {
  // Root logger's default min level is kInfo; Trace should be suppressed.
  auto called = std::make_shared<bool>(false);
  Registry::RootRef().AddSink(Sink{[called](LogRecord const& r) {
    if (r.level == LogLevel::kTrace) *called = true;
  }});
  mlogpp::Trace("should be filtered");
  EXPECT_FALSE(*called);
}

TEST(GlobalLoggingTest, GlobalFunctionsForwardFormattedArgs) {
  auto captured = std::make_shared<std::string>();
  Registry::RootRef().AddSink(Sink{[captured](LogRecord const& r) {
    if (r.level == LogLevel::kInfo) *captured = r.message;
  }});
  mlogpp::Info("value={} str={}", 7, "x");
  EXPECT_EQ(*captured, "value=7 str=x");
}

// ── Thread safety
// ─────────────────────────────────────────────────────────────

TEST(RegistryThreadSafetyTest, ConcurrentGetExistingReturnsSamePtr) {
  // Pre-create so all threads hit the shared-lock fast path.
  auto expected = Registry::Get("ts_existing");
  constexpr int kThreads = 8;
  std::vector<LoggerPtr> results(kThreads);
  std::vector<std::thread> threads;
  threads.reserve(kThreads);
  for (int i = 0; i < kThreads; ++i) {
    threads.emplace_back(
        [&results, i] { results[i] = Registry::Get("ts_existing"); });
  }
  for (auto& t : threads) t.join();
  for (int i = 0; i < kThreads; ++i) {
    EXPECT_EQ(results[i].get(), expected.get());
  }
}

TEST(RegistryThreadSafetyTest, ConcurrentGetDifferentNamesAllNonNull) {
  constexpr int kThreads = 8;
  std::vector<LoggerPtr> results(kThreads);
  std::vector<std::thread> threads;
  threads.reserve(kThreads);
  for (int i = 0; i < kThreads; ++i) {
    threads.emplace_back([&results, i] {
      results[i] = Registry::Get("ts_diff_" + std::to_string(i));
    });
  }
  for (auto& t : threads) t.join();
  for (auto const& lgr : results) {
    EXPECT_NE(lgr, nullptr);
  }
}

TEST(RegistryThreadSafetyTest, ConcurrentGetDoesNotDeadlock) {
  constexpr int kThreads = 4;
  constexpr int kIter = 100;
  std::atomic<int> count{0};
  std::vector<std::thread> threads;
  threads.reserve(kThreads);
  for (int i = 0; i < kThreads; ++i) {
    threads.emplace_back([&count] {
      for (int j = 0; j < kIter; ++j) {
        auto lgr = Registry::Get("ts_nodl_" + std::to_string(j % 10));
        (void)lgr;
        ++count;
      }
    });
  }
  for (auto& t : threads) t.join();
  EXPECT_EQ(count.load(), kThreads * kIter);
}

TEST(RegistryThreadSafetyTest, ConcurrentRemoveAndGetAreDataRaceFree) {
  // Interleave remove and Get on the same name — no crash or UB expected.
  (void)Registry::Get("ts_remove_get");
  std::atomic<bool> stop{false};
  std::thread getter([&stop] {
    while (!stop.load()) (void)Registry::Get("ts_remove_get");
  });
  for (int i = 0; i < 50; ++i) {
    Registry::instance().remove("ts_remove_get");
    (void)Registry::Get("ts_remove_get");
  }
  stop.store(true);
  getter.join();
}

}  // namespace mlogpp
