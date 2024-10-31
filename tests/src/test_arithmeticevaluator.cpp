#include <gtest/gtest.h>
#include <contextualplanner/util/arithmeticevaluator.hpp>

using namespace cp;

TEST(Tool, test_arithmeticEvaluator)
{ 
  EXPECT_EQ(2, evalute("1+1"));
  EXPECT_EQ(6, evalute("5+1"));
  EXPECT_EQ(16, evalute("15+1"));
  try {
    evalute("`15+1`");
    assert(false);
  } catch (...) {}
  EXPECT_EQ(14, evalute("`13+1`", 1));
  EXPECT_EQ("14", evaluteToStr("`13+1`", 1));
  EXPECT_EQ("_", evaluteToStr("`13+1`"));
}
