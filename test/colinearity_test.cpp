#include <verification/collinear.hpp>
#include <common/assignment.hpp>
#include <common/instance.hpp>
#include <gd_types.hpp>

#include <test_helper/mock_data.hpp>

#include <gtest/gtest.h>

using namespace gd;


TEST(VerifierTest, SimpleColinearityTestGrid)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(0, 2),
      PAIR(0, 3),
      PAIR(1, 3),
      PAIR(1, 4),
      PAIR(2, 3),
      PAIR(2, 4),
      PAIR(3, 4),
      PAIR(4, 0),
    },
    {
      Point(0, 0, 0),
      Point(1, 1, 0),
      Point(2, 2, 0),
      Point(3, 3, 0),
      Point(4, 0, 1),
      Point(5, 1, 1),
      Point(6, 2, 1),
      Point(7, 3, 1),
      Point(8, 0, 2),
      Point(9, 1, 2),
      Point(10, 2, 2),
      Point(11, 3, 2),
      Point(12, 0, 3),
      Point(13, 1, 3),
      Point(14, 2, 3),
      Point(15, 3, 3),
    });
  VertexAssignment assignment = create_mock_assignment(instance, {
    PAIR(0, 0),
    PAIR(1, 3),
    PAIR(2, 12),
    PAIR(3, 15),
    PAIR(4, 6)
  });
  EXPECT_FALSE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 9);
  EXPECT_FALSE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 1);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 2);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 4);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 5);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 7);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 8);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 10);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 11);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 13);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 14);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
}
TEST(VerifierTest, SimpleColinearityTestGridNegativ)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(0, 2),
      PAIR(0, 3),
      PAIR(1, 3),
      PAIR(1, 4),
      PAIR(2, 3),
      PAIR(2, 4),
      PAIR(3, 4),
      PAIR(4, 0),
    },
    {
      Point(0, -2, -2),
      Point(1, -1, -2),
      Point(2, 0, -2),
      Point(3, 1, -2),
      Point(4, -2, -1),
      Point(5, -1, -1),
      Point(6, 0, -1),
      Point(7, 1, -1),
      Point(8, -2, 0),
      Point(9, -1, 0),
      Point(10, 0, 0),
      Point(11, 1, 0),
      Point(12, -2, 1),
      Point(13, -1, 1),
      Point(14, 0, 1),
      Point(15, 1, 1),
    });
  VertexAssignment assignment = create_mock_assignment(instance, {
    PAIR(0, 0),
    PAIR(1, 3),
    PAIR(2, 12),
    PAIR(3, 15),
    PAIR(4, 6)
  });
  EXPECT_FALSE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 9);
  EXPECT_FALSE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 1);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 2);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 4);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 5);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 7);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 8);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 10);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 11);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 13);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
  assignment.assign(4, 14);
  EXPECT_TRUE(gd::trivialCollinearityCheck(instance, assignment));
}

TEST(VerifierTest, NoColinearityLongEdge)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(2, 3)
    },
    {
      Point(0, 0, 0),
      Point(1, 2e8, 2),
      Point(2, 1e8 + 1, 0),
      Point(3, 1e8 + 1, 1)
    });
  VertexAssignment assignment = create_mock_assignment(instance, {
    PAIR(0, 0),
    PAIR(1, 1),
    PAIR(2, 2),
    PAIR(3, 3)
  });
  EXPECT_FALSE(gd::trivialCollinearityCheck(instance, assignment));
}