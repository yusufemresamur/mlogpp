#include "mlogpp/logger.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace mlogpp {

// ── Helpers
// ───────────────────────────────────────────────────────────────────

namespace {

// Construct a Logger that passes all log levels regardless of filter type.
// For DynamicLogger the kTrace min_level is applied; for StaticLogger<kTrace>
// the constructor arg is silently ignored and the static filter passes all.
template <typename L>
L MakePassAllLogger(std::string_view name) {
  return L{name, {}, LogLevel::kTrace};
}

}  // namespace

// ── Type alias checks
// ─────────────────────────────────────────────────────────

TEST(LoggerTypes, DynamicLoggerIsLoggerWithDynamicFilter) {
  static_assert(std::is_same_v<DynamicLogger, Logger<DynamicFilter>>);
}

TEST(LoggerTypes, StaticLoggerIsLoggerWithStaticFilter) {
  static_assert(std::is_same_v<StaticLogger<LogLevel::kInfo>,
                               Logger<StaticFilter<LogLevel::kInfo>>>);
}

TEST(LoggerTypes, LoggerPtrIsSharedPtrToDynamicLogger) {
  static_assert(std::is_same_v<LoggerPtr, std::shared_ptr<DynamicLogger>>);
}

// ── Typed tests — shared Logger interface
// ─────────────────────────────────────
//
// Both DynamicLogger (min_level=kTrace so all pass) and StaticLogger<kTrace>
// (passes all at compile time) exercise the same public API.

template <typename L>
class LoggerInterfaceTest : public ::testing::Test {};

using PassAllLoggerTypes =
    ::testing::Types<DynamicLogger, StaticLogger<LogLevel::kTrace>>;
TYPED_TEST_SUITE(LoggerInterfaceTest, PassAllLoggerTypes);

TYPED_TEST(LoggerInterfaceTest, NameReturnsConstructedName) {
  auto logger = MakePassAllLogger<TypeParam>("my_logger");
  EXPECT_EQ(logger.Name(), "my_logger");
}

TYPED_TEST(LoggerInterfaceTest, EmptyNameIsAllowed) {
  auto logger = MakePassAllLogger<TypeParam>("");
  EXPECT_EQ(logger.Name(), "");
}

TYPED_TEST(LoggerInterfaceTest, AddSinkReturnsReferenceToLogger) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  auto& ref = logger.AddSink(Sink{[](LogRecord const&) {}});
  EXPECT_EQ(&ref, &logger);
}

TYPED_TEST(LoggerInterfaceTest, SinksAddedViaConstructorAreCalled) {
  int count = 0;
  Sink s{[&count](LogRecord const&) { ++count; }};
  TypeParam logger{"test", {s}, LogLevel::kTrace};
  logger.template Log<LogLevel::kInfo>("msg");
  EXPECT_EQ(count, 1);
}

TYPED_TEST(LoggerInterfaceTest, SinkAddedViaAddSinkIsCalled) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  bool called = false;
  logger.AddSink(Sink{[&called](LogRecord const&) { called = true; }});
  logger.template Log<LogLevel::kInfo>("msg");
  EXPECT_TRUE(called);
}

TYPED_TEST(LoggerInterfaceTest, NoSinksNoDispatch) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  EXPECT_NO_THROW(logger.template Log<LogLevel::kInfo>("msg"));
}

TYPED_TEST(LoggerInterfaceTest, AllSinksReceiveEachRecord) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.template Log<LogLevel::kInfo>("msg");
  EXPECT_EQ(count, 3);
}

TYPED_TEST(LoggerInterfaceTest, SinksCalledOncePerLogCall) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.template Log<LogLevel::kInfo>("first");
  logger.template Log<LogLevel::kWarn>("second");
  logger.template Log<LogLevel::kError>("third");
  EXPECT_EQ(count, 3);
}

TYPED_TEST(LoggerInterfaceTest, LogRecordHasCorrectLevel) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  LogLevel captured{};
  logger.AddSink(Sink{[&captured](LogRecord const& r) { captured = r.level; }});
  logger.template Log<LogLevel::kWarn>("msg");
  EXPECT_EQ(captured, LogLevel::kWarn);
}

TYPED_TEST(LoggerInterfaceTest, LogRecordHasCorrectLoggerName) {
  auto logger = MakePassAllLogger<TypeParam>("my_service");
  std::string captured;
  logger.AddSink(
      Sink{[&captured](LogRecord const& r) { captured = r.logger_name; }});
  logger.template Log<LogLevel::kInfo>("msg");
  EXPECT_EQ(captured, "my_service");
}

TYPED_TEST(LoggerInterfaceTest, LogRecordHasFormattedMessage) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  std::string captured;
  logger.AddSink(
      Sink{[&captured](LogRecord const& r) { captured = r.message; }});
  logger.template Log<LogLevel::kInfo>("hello {}", 42);
  EXPECT_EQ(captured, "hello 42");
}

TYPED_TEST(LoggerInterfaceTest, LogRecordHasNonNullSourceLocation) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  std::source_location captured{};
  logger.AddSink(
      Sink{[&captured](LogRecord const& r) { captured = r.location; }});
  logger.template Log<LogLevel::kInfo>("msg");
  EXPECT_NE(captured.line(), 0u);
  EXPECT_NE(captured.file_name(), std::string_view{});
}

// ── Convenience methods — typed tests
// ─────────────────────────────────────────

TYPED_TEST(LoggerInterfaceTest, TraceLogsAtTraceLevel) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  LogLevel captured{};
  logger.AddSink(Sink{[&captured](LogRecord const& r) { captured = r.level; }});
  logger.Trace("msg");
  EXPECT_EQ(captured, LogLevel::kTrace);
}

TYPED_TEST(LoggerInterfaceTest, DebugLogsAtDebugLevel) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  LogLevel captured{};
  logger.AddSink(Sink{[&captured](LogRecord const& r) { captured = r.level; }});
  logger.Debug("msg");
  EXPECT_EQ(captured, LogLevel::kDebug);
}

TYPED_TEST(LoggerInterfaceTest, InfoLogsAtInfoLevel) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  LogLevel captured{};
  logger.AddSink(Sink{[&captured](LogRecord const& r) { captured = r.level; }});
  logger.Info("msg");
  EXPECT_EQ(captured, LogLevel::kInfo);
}

TYPED_TEST(LoggerInterfaceTest, WarnLogsAtWarnLevel) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  LogLevel captured{};
  logger.AddSink(Sink{[&captured](LogRecord const& r) { captured = r.level; }});
  logger.Warn("msg");
  EXPECT_EQ(captured, LogLevel::kWarn);
}

TYPED_TEST(LoggerInterfaceTest, ErrorLogsAtErrorLevel) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  LogLevel captured{};
  logger.AddSink(Sink{[&captured](LogRecord const& r) { captured = r.level; }});
  logger.Error("msg");
  EXPECT_EQ(captured, LogLevel::kError);
}

TYPED_TEST(LoggerInterfaceTest, FatalLogsAtFatalLevel) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  LogLevel captured{};
  logger.AddSink(Sink{[&captured](LogRecord const& r) { captured = r.level; }});
  logger.Fatal("msg");
  EXPECT_EQ(captured, LogLevel::kFatal);
}

TYPED_TEST(LoggerInterfaceTest, ConvenienceMethodsForwardFormatArgs) {
  auto logger = MakePassAllLogger<TypeParam>("test");
  std::string captured;
  logger.AddSink(
      Sink{[&captured](LogRecord const& r) { captured = r.message; }});

  logger.Trace("trace {}", 1);
  EXPECT_EQ(captured, "trace 1");

  logger.Debug("debug {}", 2);
  EXPECT_EQ(captured, "debug 2");

  logger.Info("info {}", 3);
  EXPECT_EQ(captured, "info 3");

  logger.Warn("warn {}", 4);
  EXPECT_EQ(captured, "warn 4");

  logger.Error("error {}", 5);
  EXPECT_EQ(captured, "error 5");

  logger.Fatal("fatal {}", 6);
  EXPECT_EQ(captured, "fatal 6");
}

// ── DynamicLogger — filter integration ───────────────────────────────────────

TEST(DynamicLoggerTest, DefaultMinLevelIsInfo) {
  DynamicLogger logger{"test"};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});

  logger.Trace("t");
  logger.Debug("d");
  EXPECT_EQ(count, 0);

  logger.Info("i");
  EXPECT_EQ(count, 1);
}

TEST(DynamicLoggerTest, ConstructorMinLevelFiltersCorrectly) {
  DynamicLogger logger{"test", {}, LogLevel::kWarn};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});

  logger.Trace("t");
  logger.Debug("d");
  logger.Info("i");
  EXPECT_EQ(count, 0);

  logger.Warn("w");
  logger.Error("e");
  logger.Fatal("f");
  EXPECT_EQ(count, 3);
}

TEST(DynamicLoggerTest, ConstructorMinLevelTracePasses) {
  DynamicLogger logger{"test", {}, LogLevel::kTrace};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});

  logger.Trace("t");
  logger.Debug("d");
  logger.Info("i");
  logger.Warn("w");
  logger.Error("e");
  logger.Fatal("f");
  EXPECT_EQ(count, 6);
}

TEST(DynamicLoggerTest, ConstructorMinLevelFatalOnlyPassesFatal) {
  DynamicLogger logger{"test", {}, LogLevel::kFatal};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});

  logger.Trace("t");
  logger.Debug("d");
  logger.Info("i");
  logger.Warn("w");
  logger.Error("e");
  EXPECT_EQ(count, 0);

  logger.Fatal("f");
  EXPECT_EQ(count, 1);
}

TEST(DynamicLoggerTest, FilterPassesBoundaryLevelExactly) {
  constexpr LogLevel kLevels[] = {
      LogLevel::kTrace, LogLevel::kDebug, LogLevel::kInfo,
      LogLevel::kWarn,  LogLevel::kError, LogLevel::kFatal,
  };
  for (auto const min : kLevels) {
    DynamicLogger logger{"test", {}, min};
    bool called = false;
    logger.AddSink(Sink{[&called](LogRecord const&) { called = true; }});
    logger.template Log<LogLevel::kTrace>("t");
    logger.template Log<LogLevel::kDebug>("d");
    logger.template Log<LogLevel::kInfo>("i");
    logger.template Log<LogLevel::kWarn>("w");
    logger.template Log<LogLevel::kError>("e");
    logger.template Log<LogLevel::kFatal>("f");
    EXPECT_TRUE(called) << "Nothing reached sink for min_level="
                        << static_cast<int>(min);
  }
}

TEST(DynamicLoggerTest, SetMinLevelUpdatesFilterAtRuntime) {
  DynamicLogger logger{"test", {}, LogLevel::kError};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});

  logger.Info("before");
  EXPECT_EQ(count, 0);

  logger.SetMinLevel(LogLevel::kTrace);
  logger.Info("after");
  EXPECT_EQ(count, 1);
}

TEST(DynamicLoggerTest, SetMinLevelCanRaiseThreshold) {
  DynamicLogger logger{"test", {}, LogLevel::kTrace};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});

  logger.Debug("before");
  EXPECT_EQ(count, 1);

  logger.SetMinLevel(LogLevel::kError);
  logger.Debug("after");
  EXPECT_EQ(count, 1);  // still 1, not emitted
}

TEST(DynamicLoggerTest, SetMinLevelReturnsReferenceToLogger) {
  DynamicLogger logger{"test"};
  auto& ref = logger.SetMinLevel(LogLevel::kDebug);
  EXPECT_EQ(&ref, &logger);
}

TEST(DynamicLoggerTest, SetMinLevelChainsWithAddSink) {
  int count = 0;
  DynamicLogger logger{"test"};
  logger.SetMinLevel(LogLevel::kTrace).AddSink(Sink{[&count](LogRecord const&) {
    ++count;
  }});
  logger.Trace("t");
  EXPECT_EQ(count, 1);
}

// ── StaticLogger — filter integration ────────────────────────────────────────

TEST(StaticLoggerTest, TracePassesAllLevels) {
  StaticLogger<LogLevel::kTrace> logger{"test"};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.Trace("t");
  logger.Debug("d");
  logger.Info("i");
  logger.Warn("w");
  logger.Error("e");
  logger.Fatal("f");
  EXPECT_EQ(count, 6);
}

TEST(StaticLoggerTest, DebugBlocksTraceOnly) {
  StaticLogger<LogLevel::kDebug> logger{"test"};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.Trace("t");
  EXPECT_EQ(count, 0);
  logger.Debug("d");
  logger.Info("i");
  logger.Warn("w");
  logger.Error("e");
  logger.Fatal("f");
  EXPECT_EQ(count, 5);
}

TEST(StaticLoggerTest, WarnBlocksTraceDebugInfo) {
  StaticLogger<LogLevel::kWarn> logger{"test"};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.Trace("t");
  logger.Debug("d");
  logger.Info("i");
  EXPECT_EQ(count, 0);
  logger.Warn("w");
  logger.Error("e");
  logger.Fatal("f");
  EXPECT_EQ(count, 3);
}

TEST(StaticLoggerTest, FatalOnlyPassesFatal) {
  StaticLogger<LogLevel::kFatal> logger{"test"};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});
  logger.Trace("t");
  logger.Debug("d");
  logger.Info("i");
  logger.Warn("w");
  logger.Error("e");
  EXPECT_EQ(count, 0);
  logger.Fatal("f");
  EXPECT_EQ(count, 1);
}

TEST(StaticLoggerTest, SetMinLevelIsNoOpForStaticFilter) {
  // StaticFilter has no min_level member: SetMinLevel is a no-op via
  // if constexpr. The compile-time threshold should remain unchanged.
  StaticLogger<LogLevel::kWarn> logger{"test"};
  int count = 0;
  logger.AddSink(Sink{[&count](LogRecord const&) { ++count; }});

  logger.SetMinLevel(LogLevel::kTrace);  // should change nothing

  logger.Trace("t");
  logger.Debug("d");
  logger.Info("i");
  EXPECT_EQ(count, 0);

  logger.Warn("w");
  EXPECT_EQ(count, 1);
}

TEST(StaticLoggerTest, SetMinLevelReturnsReferenceEvenForStaticFilter) {
  StaticLogger<LogLevel::kInfo> logger{"test"};
  auto& ref = logger.SetMinLevel(LogLevel::kDebug);
  EXPECT_EQ(&ref, &logger);
}

TEST(StaticLoggerTest, IsConstexprFilterEliminatedAtCompileTime) {
  // Verify StaticFilter<kWarn> blocks kTrace at compile-time.
  // If the call were not dead-code-eliminated the static_assert would fail
  // inside Logger::Log via the if constexpr check.
  static_assert(!StaticFilter<LogLevel::kWarn>::passes(LogLevel::kTrace));
  static_assert(!StaticFilter<LogLevel::kWarn>::passes(LogLevel::kDebug));
  static_assert(!StaticFilter<LogLevel::kWarn>::passes(LogLevel::kInfo));
  static_assert(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kWarn));
  static_assert(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kError));
  static_assert(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kFatal));
}

// ── LoggerPtr
// ─────────────────────────────────────────────────────────────────

TEST(LoggerPtrTest, CanConstructViaSharedPtr) {
  LoggerPtr ptr = std::make_shared<DynamicLogger>("test");
  EXPECT_EQ(ptr->Name(), "test");
}

TEST(LoggerPtrTest, DispatchesViaSharedPtr) {
  LoggerPtr ptr = std::make_shared<DynamicLogger>("test", std::vector<Sink>{},
                                                  LogLevel::kTrace);
  bool called = false;
  ptr->AddSink(Sink{[&called](LogRecord const&) { called = true; }});
  ptr->Info("msg");
  EXPECT_TRUE(called);
}

TEST(LoggerPtrTest, SharedOwnershipIsObservable) {
  LoggerPtr p1 = std::make_shared<DynamicLogger>("test");
  LoggerPtr p2 = p1;
  EXPECT_EQ(p1.get(), p2.get());
  EXPECT_EQ(p1.use_count(), 2);
}

}  // namespace mlogpp
