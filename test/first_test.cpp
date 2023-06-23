#include <gtest/gtest.h>
#include <common/verifier.hpp>

using namespace gd;

TEST(FirstTest, Trivial_Larger_Zero)
{
  ASSERT_TRUE(gd::test_func(3));
}


TEST(FirstTest, Less_eq_Zero)
{
  ASSERT_FALSE(gd::test_func(-1));
}


TEST(FirstTest, Not_Div_Three)
{
  ASSERT_FALSE(gd::test_func(7));
}