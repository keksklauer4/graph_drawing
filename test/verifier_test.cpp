#include "verification/collinear.hpp"
#include <common/assignment.hpp>
#include <common/instance.hpp>
#include <gd_types.hpp>
#include <test_helper/mock_data.hpp>
#include <gtest/gtest.h>
#include <io/json_handling.hpp>

using namespace gd;


TEST(VerifierTest, SimpleNoCollinearity)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(1, 2)
    },
    {
      Point(0, 0, 0),
      Point(1, 1, 0),
      Point(2, 2, 0)
    });
  VertexAssignment assignment = create_mock_assignment(3, {
    PAIR(0, 0),
    PAIR(1, 1),
    PAIR(2, 2)
  });
  EXPECT_FALSE(gd::trivialCollinearityCheck(instance, assignment));
}

TEST(VerifierTest, SimpleCollinearity)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(1, 2)
    },
    {
      Point(0, 0, 0),
      Point(1, 1, 0),
      Point(2, 2, 0)
    });
  VertexAssignment assignment = create_mock_assignment(3, {
    PAIR(0, 1),
    PAIR(1, 0), // vertices 0 and 1 are mixed (bad!)
    PAIR(2, 2)
  });
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
}