#include "mlogpp/sink/file_sink.hpp"
#include "mlogpp/sink/sink.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

// TODO (yusufemresamur): review AI generated tests

namespace mlogpp {

// ── Helpers
// ───────────────────────────────────────────────────────────────────

namespace {

LogRecord MakeRecord(LogLevel level = LogLevel::kInfo,
                     std::string_view msg = "test message",
                     std::string_view logger = "test") {
  return LogRecord{level, std::source_location::current(), logger,
                   std::string(msg)};
}

struct FixedFormatter {
  std::string operator()(LogRecord const&) const { return "FIXED"; }
};

}  // namespace

// ── SinkFunction concept
// ──────────────────────────────────────────────────────

struct ValidSinkFn {
  void operator()(LogRecord const&) const {}
};
struct NonVoidReturnSinkFn {
  int operator()(LogRecord const&) const { return 0; }
};
struct NoCallOpSinkFn {};

TEST(SinkFunctionConcept, LambdaReturningVoidSatisfies) {
  static_assert(SinkFunction<decltype([](LogRecord const&) {})>);
}

TEST(SinkFunctionConcept, FunctorReturningVoidSatisfies) {
  static_assert(SinkFunction<ValidSinkFn>);
}

TEST(SinkFunctionConcept, NonVoidReturnDoesNotSatisfy) {
  static_assert(!SinkFunction<NonVoidReturnSinkFn>);
}

TEST(SinkFunctionConcept, TypeWithNoCallOperatorDoesNotSatisfy) {
  static_assert(!SinkFunction<NoCallOpSinkFn>);
}

TEST(SinkFunctionConcept, SinkItselfSatisfiesConcept) {
  // Sink::operator()(LogRecord const&) returns void — concept is satisfied
  static_assert(SinkFunction<Sink>);
}

TEST(SinkFunctionConcept, FileSinkSatisfiesConcept) {
  static_assert(SinkFunction<FileSink<>>);
  static_assert(SinkFunction<FileSink<FixedFormatter>>);
}

// ── Sink — construction & call
// ────────────────────────────────────────────────

TEST(SinkTest, ConstructFromLambdaAndCall) {
  bool called = false;
  Sink s{[&called](LogRecord const&) { called = true; }};
  s(MakeRecord());
  EXPECT_TRUE(called);
}

TEST(SinkTest, ConstructFromFunctorAndForwardsRecord) {
  std::string captured;
  struct Cap {
    std::string& out;
    void operator()(LogRecord const& r) const { out = r.message(); }
  };
  Sink s{Cap{captured}};
  s(MakeRecord(LogLevel::kInfo, "hello"));
  EXPECT_EQ(captured, "hello");
}

TEST(SinkTest, CallInvokesWrappedFunctionRepeatedly) {
  int count = 0;
  Sink s{[&count](LogRecord const&) { ++count; }};
  s(MakeRecord());
  s(MakeRecord());
  s(MakeRecord());
  EXPECT_EQ(count, 3);
}

// ── Sink — copy / move semantics ─────────────────────────────────────────────

TEST(SinkTest, CopySharesUnderlyingImpl) {
  // shared_ptr semantics: both copies invoke the same closure
  int count = 0;
  Sink s1{[&count](LogRecord const&) { ++count; }};
  Sink s2 = s1;
  s1(MakeRecord());
  s2(MakeRecord());
  EXPECT_EQ(count, 2);
}

TEST(SinkTest, CopyAssignmentSharesImpl) {
  int count = 0;
  Sink s1{[&count](LogRecord const&) { ++count; }};
  Sink s2{[](LogRecord const&) {}};
  s2 = s1;
  s1(MakeRecord());
  s2(MakeRecord());
  EXPECT_EQ(count, 2);
}

TEST(SinkTest, MoveConstructLeavesCallableInDestination) {
  bool called = false;
  Sink s1{[&called](LogRecord const&) { called = true; }};
  Sink s2 = std::move(s1);
  s2(MakeRecord());
  EXPECT_TRUE(called);
}

TEST(SinkTest, MoveAssignmentLeavesCallableInDestination) {
  bool called = false;
  Sink s1{[&called](LogRecord const&) { called = true; }};
  Sink s2{[](LogRecord const&) {}};
  s2 = std::move(s1);
  s2(MakeRecord());
  EXPECT_TRUE(called);
}

// ── MakeConsoleSink
// ───────────────────────────────────────────────────────────

TEST(MakeConsoleSinkTest, ReturnsSinkThatIsCallable) {
  Sink s = MakeConsoleSink();
  EXPECT_NO_THROW(s(MakeRecord()));
}

TEST(MakeConsoleSinkTest, AcceptsCustomFormatter) {
  Sink s = MakeConsoleSink<FixedFormatter>();
  EXPECT_NO_THROW(s(MakeRecord()));
}

// ── FileSink fixture
// ──────────────────────────────────────────────────────────

class FileSinkTest : public ::testing::Test {
 protected:
  void SetUp() override {
    path_ = std::filesystem::temp_directory_path() /
            ("mlogpp_test_" + std::to_string(counter_++) + ".log");
    std::filesystem::remove(path_);
  }

  void TearDown() override { std::filesystem::remove(path_); }

  std::string ReadAll() const {
    std::ifstream f(path_);
    return {std::istreambuf_iterator<char>(f), {}};
  }

  std::filesystem::path path_;
  static int counter_;
};
int FileSinkTest::counter_ = 0;

// ── FileSink — construction
// ───────────────────────────────────────────────────

TEST_F(FileSinkTest, ConstructsWithValidPath) {
  EXPECT_NO_THROW(FileSink<> sink{path_});
}

TEST_F(FileSinkTest, ThrowsRuntimeErrorForInvalidPath) {
  EXPECT_THROW(FileSink<>{"/nonexistent_mlogpp_dir/sub/x.log"},
               std::runtime_error);
}

TEST_F(FileSinkTest, ErrorMessageContainsPath) {
  std::filesystem::path const bad{"/nonexistent_mlogpp_dir/sub/x.log"};
  try {
    FileSink<> sink{bad};
    FAIL() << "expected std::runtime_error";
  } catch (std::runtime_error const& e) {
    EXPECT_NE(std::string{e.what()}.find(bad.string()), std::string::npos);
  }
}

TEST_F(FileSinkTest, CreatesFileOnConstruction) {
  {
    FileSink<> sink{path_};
  }
  EXPECT_TRUE(std::filesystem::exists(path_));
}

// ── FileSink — copyability / moveability ─────────────────────────────────────

TEST_F(FileSinkTest, IsNotCopyConstructible) {
  static_assert(!std::is_copy_constructible_v<FileSink<>>);
}

TEST_F(FileSinkTest, IsNotCopyAssignable) {
  static_assert(!std::is_copy_assignable_v<FileSink<>>);
}

TEST_F(FileSinkTest, IsMoveConstructible) {
  static_assert(std::is_move_constructible_v<FileSink<>>);
}

TEST_F(FileSinkTest, IsMoveAssignable) {
  static_assert(std::is_move_assignable_v<FileSink<>>);
}

// ── FileSink — writing
// ────────────────────────────────────────────────────────

TEST_F(FileSinkTest, WritesSingleRecord) {
  {
    FileSink<FixedFormatter> sink{path_};
    sink(MakeRecord());
  }
  EXPECT_EQ(ReadAll(), "FIXED\n");
}

TEST_F(FileSinkTest, EachRecordEndsWithNewline) {
  {
    FileSink<FixedFormatter> sink{path_};
    sink(MakeRecord());
    sink(MakeRecord());
  }
  std::string const content = ReadAll();
  ASSERT_GE(content.size(), 2U);
  // Both lines end with '\n' — last char of file is '\n'
  EXPECT_EQ(content.back(), '\n');
  // Two "FIXED" lines means two newlines
  EXPECT_EQ(std::count(content.begin(), content.end(), '\n'), 2);
}

TEST_F(FileSinkTest, WritesMultipleRecordsInOrder) {
  {
    FileSink<> sink{path_};
    sink(MakeRecord(LogLevel::kInfo, "alpha"));
    sink(MakeRecord(LogLevel::kWarn, "beta"));
    sink(MakeRecord(LogLevel::kError, "gamma"));
  }
  std::string const content = ReadAll();
  auto const p1 = content.find("alpha");
  auto const p2 = content.find("beta");
  auto const p3 = content.find("gamma");
  ASSERT_NE(p1, std::string::npos);
  ASSERT_NE(p2, std::string::npos);
  ASSERT_NE(p3, std::string::npos);
  EXPECT_LT(p1, p2);
  EXPECT_LT(p2, p3);
}

TEST_F(FileSinkTest, AppendsToExistingFile) {
  // Pre-populate
  {
    std::ofstream{path_} << "PRE_EXISTING\n";
  }
  {
    FileSink<FixedFormatter> sink{path_};
    sink(MakeRecord());
  }
  std::string const content = ReadAll();
  auto const pre = content.find("PRE_EXISTING");
  auto const fix = content.find("FIXED");
  ASSERT_NE(pre, std::string::npos);
  ASSERT_NE(fix, std::string::npos);
  EXPECT_LT(pre, fix);
}

TEST_F(FileSinkTest, DefaultFormatterIncludesLevelMessageAndLogger) {
  {
    FileSink<> sink{path_};
    sink(MakeRecord(LogLevel::kWarn, "payload", "mylogger"));
  }
  std::string const content = ReadAll();
  EXPECT_NE(content.find("WARN"), std::string::npos);
  EXPECT_NE(content.find("payload"), std::string::npos);
  EXPECT_NE(content.find("mylogger"), std::string::npos);
}

TEST_F(FileSinkTest, CustomFormatterOutputOverridesDefault) {
  {
    FileSink<FixedFormatter> sink{path_};
    sink(MakeRecord(LogLevel::kError, "should not appear"));
  }
  std::string const content = ReadAll();
  EXPECT_NE(content.find("FIXED"), std::string::npos);
  EXPECT_EQ(content.find("should not appear"), std::string::npos);
}

TEST_F(FileSinkTest, MoveConstructedSinkWritesToSameFile) {
  FileSink<FixedFormatter> s1{path_};
  FileSink<FixedFormatter> s2{std::move(s1)};
  s2(MakeRecord());
  EXPECT_EQ(ReadAll(), "FIXED\n");
}

// ── MakeFileSink
// ──────────────────────────────────────────────────────────────

TEST_F(FileSinkTest, MakeFileSinkReturnsSinkThatWrites) {
  {
    Sink s = MakeFileSink(path_);
    s(MakeRecord(LogLevel::kInfo, "via_make"));
  }
  EXPECT_NE(ReadAll().find("via_make"), std::string::npos);
}

TEST_F(FileSinkTest, MakeFileSinkThrowsForInvalidPath) {
  EXPECT_THROW(
      { [[maybe_unused]] auto s = MakeFileSink("/bad_mlogpp_dir/x.log"); },
      std::runtime_error);
}

TEST_F(FileSinkTest, MakeFileSinkWithCustomFormatter) {
  {
    Sink s = MakeFileSink<FixedFormatter>(path_);
    s(MakeRecord());
  }
  EXPECT_EQ(ReadAll(), "FIXED\n");
}

TEST_F(FileSinkTest, MakeFileSinkCopiedSinkSharesFile) {
  Sink s1 = MakeFileSink<FixedFormatter>(path_);
  Sink s2 = s1;  // shared_ptr copy — same underlying FileSink
  s1(MakeRecord());
  s2(MakeRecord());
  // Two lines written
  std::string const content = ReadAll();
  EXPECT_EQ(std::count(content.begin(), content.end(), '\n'), 2);
}

}  // namespace mlogpp
