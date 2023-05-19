#include <verification/collinear.hpp>
#include <verification/line_crossings.hpp>
#include <common/assignment.hpp>
#include <common/instance.hpp>
#include <gd_types.hpp>
#include <io/json_handling.hpp>

#include <test_helper/mock_data.hpp>

#include <gtest/gtest.h>

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


TEST(VerifierTest, SimpleNoCollinearityShifted)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(1, 2)
    },
    {
      Point(0, 0, 0),
      Point(1, 2, 1), // not collinear ( since its not (2,2) )
      Point(2, 4, 4)
    });
  VertexAssignment assignment = create_mock_assignment(3, {
    PAIR(0, 1),
    PAIR(1, 0),
    PAIR(2, 2)
  });
  EXPECT_FALSE(gd::trivialCollinearityCheck(instance, assignment));
}

TEST(VerifierTest, CrossingCounting_SimpleK4)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(0, 2),
      PAIR(0, 3),
      PAIR(1, 2),
      PAIR(1, 3),
      PAIR(2, 3)
    },
    {
      Point(0, 0, 0),
      Point(1, 5, 0),
      Point(2, 5, 5),
      Point(3, 0, 5)
    });
  VertexAssignment assignment = create_mock_assignment(4, {
    PAIR(0, 0),
    PAIR(1, 1),
    PAIR(2, 2),
    PAIR(3, 3)
  });
  EXPECT_EQ(gd::countCrossings(instance, assignment), 1);
}

TEST(VerifierTest, DISABLED_CrossingCounting_Degenerate_SamePoint)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(2, 3),
      PAIR(4, 5),
      PAIR(6, 7)
    },
    {
      Point(0, 1, 0),
      Point(1, -1, 0),

      Point(2, 0, 1),
      Point(3, 0, -1),

      Point(4, 1, 1),
      Point(5, -1, -1),

      Point(6, -1, 1),
      Point(7, 1, -1),
    });
  VertexAssignment assignment = create_mock_assignment(8, {
    PAIR(0, 0),
    PAIR(1, 1),
    PAIR(2, 2),
    PAIR(3, 3),
    PAIR(4, 4),
    PAIR(5, 5),
    PAIR(6, 6),
    PAIR(7, 7)
  });
  EXPECT_EQ(gd::countCrossings(instance, assignment), 4);
}

TEST(VerifierTest, CrossingCounting_NoCrossings)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(0, 3),
      PAIR(1, 2),
      PAIR(1, 3),
      PAIR(2, 3)
    },
    {
      Point(0, 0, 0),
      Point(1, 5, 0),
      Point(2, 5, 5),
      Point(3, 0, 5)
    });
  VertexAssignment assignment = create_mock_assignment(4, {
    PAIR(0, 0),
    PAIR(1, 1),
    PAIR(2, 2),
    PAIR(3, 3)
  });
  EXPECT_EQ(gd::countCrossings(instance, assignment), 0);
}
