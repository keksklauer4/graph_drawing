#include <cmath>
#include <gd_types.hpp>

#include <verification/verification_utils.hpp>
#include <test_helper/mock_data.hpp>

#include <gtest/gtest.h>
#include <common/graph.hpp>
#include <verification/incremental_collinear.hpp>

using namespace gd;

TEST(IncrementalCollinearTest, Check_LineEndpoint)
{
  Point p_not_endpoint{0, 1, 1};
  Point p_endpoint{1, 2, 2};
  auto line = PAIR(PAIR(0,0), PAIR(2,2));
  EXPECT_TRUE(gd::isLineEndpoint(line, p_endpoint));
  EXPECT_FALSE(gd::isLineEndpoint(line, p_not_endpoint));
}


TEST(IncrementalCollinearTest, Check_Collinear)
{
  Point p_on_line{0, 1, 1};
  Point p_not_on_line_but_collinear{1, 3, 3};
  Point p_not_on_line{2, 0, 3};

  auto line = PAIR(PAIR(0,0), PAIR(2,2));
  EXPECT_TRUE(gd::isCollinear(line, p_on_line));
  EXPECT_TRUE(gd::isCollinear(line, p_not_on_line_but_collinear));
  EXPECT_FALSE(gd::isCollinear(line, p_not_on_line));
}


TEST(IncrementalCollinearTest, Check_On_Line)
{
  Point p_on_line{0, 1, 1};
  Point p_not_on_line_but_collinear{1, 3, 3};
  Point p_not_on_line{2, 0, 3};
  auto line = PAIR(PAIR(0,0), PAIR(2,2));
  EXPECT_TRUE(gd::isOnLine(line, p_on_line));
  EXPECT_FALSE(gd::isOnLine(line, p_not_on_line_but_collinear));
  EXPECT_FALSE(gd::isOnLine(line, p_not_on_line));
}



TEST(IncrementalCollinearTest, Check_On_Line_ConstCoordinate)
{
  Point p_not_on_x_line1{0, 10, 21}; // above line
  Point p_not_on_x_line2{1, 11, 10}; // right to line

  // (10,0) - (10,20)
  auto line_x_const = PAIR(PAIR(10,0), PAIR(10,20));
  EXPECT_FALSE(gd::isOnLine(line_x_const, p_not_on_x_line1));
  EXPECT_FALSE(gd::isOnLine(line_x_const, p_not_on_x_line2));

  Point p_not_on_y_line1{0, 10, 11}; // above line
  Point p_not_on_y_line2{1, 21, 10}; // right to line

  // (0,10) - (20,10)
  auto line_y_const = PAIR(PAIR(0,10), PAIR(20,10));
  EXPECT_FALSE(gd::isOnLine(line_y_const, p_not_on_y_line1));
  EXPECT_FALSE(gd::isOnLine(line_y_const, p_not_on_y_line2));
}

struct CollinearityTestStruct : CollinearFunction
{
  CollinearityTestStruct(Set<Point>& points_coll)
    : CollinearFunction(), m_points_coll(points_coll) {}
  bool operator()(const Point& p) override
  {
    EXPECT_FALSE(m_points_coll.contains(p)); // should only be inserted/checked at most once!
    m_points_coll.insert(p);
    return true;
  }

  Set<Point>& m_points_coll;
};


TEST(IncrementalCollinearTest, FindCollinearPoints_Grid)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(2, 3),
      PAIR(4, 5),

    },
    {
      Point(0, 0, 0),
      Point(1, 100, 100), // line points

      Point(2, 50, 50), // middle of line
      Point(3, 101, 101), // not on line
      Point(4, 10, 10), // also on line
      Point(5, 80, 80), // on line but will not be assigned
      Point(6, 51, 50)  // not on line
    });
  VertexAssignment assignment = create_mock_assignment(6, {
    PAIR(0, 0),
    PAIR(1, 1),
    PAIR(2, 2),
    PAIR(3, 3),
    PAIR(4, 4),
    PAIR(5, 6)
  });

  Vector<Point> points_on_line {
    Point(2, 50, 50),
    Point(4, 10, 10)};
  line_2d_t line = PAIR(PAIR(0,0), PAIR(100,100));

  Set<Point> points_coll{};
  CollinearityTestStruct coll{points_coll};
  IncrementalCollinear checker{instance, assignment, coll};
  checker.findCollinear(line);

  EXPECT_EQ(points_coll.size(), 2);
  EXPECT_TRUE(points_coll.contains(points_on_line[0]));
  EXPECT_TRUE(points_coll.contains(points_on_line[1]));
}
