#include "mlogpp/filter.hpp"
#include <gtest/gtest.h>

namespace mlogpp {

// ── LevelFilter concept ──────────────────────────────────────────────────────

TEST(LevelFilterConcept, DynamicFilterSatisfiesConcept) {
  static_assert(LevelFilter<DynamicFilter>);
}

TEST(LevelFilterConcept, StaticFilterSatisfiesConcept) {
  static_assert(LevelFilter<StaticFilter<LogLevel::kTrace>>);
  static_assert(LevelFilter<StaticFilter<LogLevel::kDebug>>);
  static_assert(LevelFilter<StaticFilter<LogLevel::kInfo>>);
  static_assert(LevelFilter<StaticFilter<LogLevel::kWarn>>);
  static_assert(LevelFilter<StaticFilter<LogLevel::kError>>);
  static_assert(LevelFilter<StaticFilter<LogLevel::kFatal>>);
}

struct NoPasses {};
struct WrongReturn {
  int passes(LogLevel) const { return 1; }
};
struct CorrectFilter {
  bool passes(LogLevel) const { return true; }
};

TEST(LevelFilterConcept, TypeWithoutPassesDoesNotSatisfy) {
  static_assert(!LevelFilter<NoPasses>);
}

TEST(LevelFilterConcept, TypeWithWrongReturnSatisfies) {
  // int is convertible_to<bool>, so WrongReturn must satisfy the concept
  static_assert(LevelFilter<WrongReturn>);
}

TEST(LevelFilterConcept, CorrectFilterSatisfies) {
  static_assert(LevelFilter<CorrectFilter>);
}

// ── DynamicFilter ────────────────────────────────────────────────────────────

TEST(DynamicFilterTest, DefaultMinLevelIsTrace) {
  DynamicFilter const f;
  EXPECT_EQ(f.min_level, LogLevel::kTrace);
}

TEST(DynamicFilterTest, DefaultPassesAllLevels) {
  DynamicFilter const f;
  EXPECT_TRUE(f.passes(LogLevel::kTrace));
  EXPECT_TRUE(f.passes(LogLevel::kDebug));
  EXPECT_TRUE(f.passes(LogLevel::kInfo));
  EXPECT_TRUE(f.passes(LogLevel::kWarn));
  EXPECT_TRUE(f.passes(LogLevel::kError));
  EXPECT_TRUE(f.passes(LogLevel::kFatal));
}

TEST(DynamicFilterTest, PassesExactMinLevel) {
  constexpr LogLevel kLevels[] = {
      LogLevel::kTrace, LogLevel::kDebug, LogLevel::kInfo,
      LogLevel::kWarn,  LogLevel::kError, LogLevel::kFatal,
  };
  for (auto const min : kLevels) {
    DynamicFilter f{min};
    EXPECT_TRUE(f.passes(min)) << "min_level == " << static_cast<int>(min);
  }
}

TEST(DynamicFilterTest, RejectsLevelsBelowMin) {
  DynamicFilter f{LogLevel::kInfo};
  EXPECT_FALSE(f.passes(LogLevel::kTrace));
  EXPECT_FALSE(f.passes(LogLevel::kDebug));
}

TEST(DynamicFilterTest, PassesLevelsAboveMin) {
  DynamicFilter f{LogLevel::kInfo};
  EXPECT_TRUE(f.passes(LogLevel::kInfo));
  EXPECT_TRUE(f.passes(LogLevel::kWarn));
  EXPECT_TRUE(f.passes(LogLevel::kError));
  EXPECT_TRUE(f.passes(LogLevel::kFatal));
}

TEST(DynamicFilterTest, MinLevelWarnFiltersLower) {
  DynamicFilter f{LogLevel::kWarn};
  EXPECT_FALSE(f.passes(LogLevel::kTrace));
  EXPECT_FALSE(f.passes(LogLevel::kDebug));
  EXPECT_FALSE(f.passes(LogLevel::kInfo));
  EXPECT_TRUE(f.passes(LogLevel::kWarn));
  EXPECT_TRUE(f.passes(LogLevel::kError));
  EXPECT_TRUE(f.passes(LogLevel::kFatal));
}

TEST(DynamicFilterTest, MinLevelFatalOnlyPassesFatal) {
  DynamicFilter f{LogLevel::kFatal};
  EXPECT_FALSE(f.passes(LogLevel::kTrace));
  EXPECT_FALSE(f.passes(LogLevel::kDebug));
  EXPECT_FALSE(f.passes(LogLevel::kInfo));
  EXPECT_FALSE(f.passes(LogLevel::kWarn));
  EXPECT_FALSE(f.passes(LogLevel::kError));
  EXPECT_TRUE(f.passes(LogLevel::kFatal));
}

TEST(DynamicFilterTest, RuntimeUpdate) {
  DynamicFilter f{LogLevel::kDebug};
  EXPECT_FALSE(f.passes(LogLevel::kTrace));
  EXPECT_TRUE(f.passes(LogLevel::kDebug));

  f.min_level = LogLevel::kError;
  EXPECT_FALSE(f.passes(LogLevel::kDebug));
  EXPECT_FALSE(f.passes(LogLevel::kInfo));
  EXPECT_TRUE(f.passes(LogLevel::kError));
  EXPECT_TRUE(f.passes(LogLevel::kFatal));
}

TEST(DynamicFilterTest, IsConstexprEvaluable) {
  constexpr DynamicFilter f{LogLevel::kInfo};
  static_assert(f.passes(LogLevel::kInfo));
  static_assert(f.passes(LogLevel::kFatal));
  static_assert(!f.passes(LogLevel::kTrace));
  static_assert(!f.passes(LogLevel::kDebug));
}

// ── StaticFilter ─────────────────────────────────────────────────────────────

TEST(StaticFilterTest, TracePassesAll) {
  EXPECT_TRUE(StaticFilter<LogLevel::kTrace>::passes(LogLevel::kTrace));
  EXPECT_TRUE(StaticFilter<LogLevel::kTrace>::passes(LogLevel::kDebug));
  EXPECT_TRUE(StaticFilter<LogLevel::kTrace>::passes(LogLevel::kInfo));
  EXPECT_TRUE(StaticFilter<LogLevel::kTrace>::passes(LogLevel::kWarn));
  EXPECT_TRUE(StaticFilter<LogLevel::kTrace>::passes(LogLevel::kError));
  EXPECT_TRUE(StaticFilter<LogLevel::kTrace>::passes(LogLevel::kFatal));
}

TEST(StaticFilterTest, DebugPassesDebugAndAbove) {
  EXPECT_FALSE(StaticFilter<LogLevel::kDebug>::passes(LogLevel::kTrace));
  EXPECT_TRUE(StaticFilter<LogLevel::kDebug>::passes(LogLevel::kDebug));
  EXPECT_TRUE(StaticFilter<LogLevel::kDebug>::passes(LogLevel::kInfo));
  EXPECT_TRUE(StaticFilter<LogLevel::kDebug>::passes(LogLevel::kWarn));
  EXPECT_TRUE(StaticFilter<LogLevel::kDebug>::passes(LogLevel::kError));
  EXPECT_TRUE(StaticFilter<LogLevel::kDebug>::passes(LogLevel::kFatal));
}

TEST(StaticFilterTest, InfoPassesInfoAndAbove) {
  EXPECT_FALSE(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kTrace));
  EXPECT_FALSE(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kDebug));
  EXPECT_TRUE(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kInfo));
  EXPECT_TRUE(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kWarn));
  EXPECT_TRUE(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kError));
  EXPECT_TRUE(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kFatal));
}

TEST(StaticFilterTest, WarnPassesWarnAndAbove) {
  EXPECT_FALSE(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kTrace));
  EXPECT_FALSE(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kDebug));
  EXPECT_FALSE(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kInfo));
  EXPECT_TRUE(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kWarn));
  EXPECT_TRUE(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kError));
  EXPECT_TRUE(StaticFilter<LogLevel::kWarn>::passes(LogLevel::kFatal));
}

TEST(StaticFilterTest, ErrorPassesErrorAndAbove) {
  EXPECT_FALSE(StaticFilter<LogLevel::kError>::passes(LogLevel::kTrace));
  EXPECT_FALSE(StaticFilter<LogLevel::kError>::passes(LogLevel::kDebug));
  EXPECT_FALSE(StaticFilter<LogLevel::kError>::passes(LogLevel::kInfo));
  EXPECT_FALSE(StaticFilter<LogLevel::kError>::passes(LogLevel::kWarn));
  EXPECT_TRUE(StaticFilter<LogLevel::kError>::passes(LogLevel::kError));
  EXPECT_TRUE(StaticFilter<LogLevel::kError>::passes(LogLevel::kFatal));
}

TEST(StaticFilterTest, FatalOnlyPassesFatal) {
  EXPECT_FALSE(StaticFilter<LogLevel::kFatal>::passes(LogLevel::kTrace));
  EXPECT_FALSE(StaticFilter<LogLevel::kFatal>::passes(LogLevel::kDebug));
  EXPECT_FALSE(StaticFilter<LogLevel::kFatal>::passes(LogLevel::kInfo));
  EXPECT_FALSE(StaticFilter<LogLevel::kFatal>::passes(LogLevel::kWarn));
  EXPECT_FALSE(StaticFilter<LogLevel::kFatal>::passes(LogLevel::kError));
  EXPECT_TRUE(StaticFilter<LogLevel::kFatal>::passes(LogLevel::kFatal));
}

TEST(StaticFilterTest, IsConstexprEvaluable) {
  static_assert(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kInfo));
  static_assert(StaticFilter<LogLevel::kInfo>::passes(LogLevel::kFatal));
  static_assert(!StaticFilter<LogLevel::kInfo>::passes(LogLevel::kTrace));
  static_assert(!StaticFilter<LogLevel::kInfo>::passes(LogLevel::kDebug));
}

TEST(StaticFilterTest, PassesIsStaticMethod) {
  // passes() should be callable on the type without an instance
  constexpr bool result =
      StaticFilter<LogLevel::kWarn>::passes(LogLevel::kError);
  static_assert(result);
}

}  // namespace mlogpp
