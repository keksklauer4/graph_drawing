#include "common/instance.hpp"
#include "verification/incremental_crossing.hpp"
#include <cmath>
#include <gd_types.hpp>

#include <initializer_list>
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
  point_pair_t line = PAIR(Point(0,0,0), Point(1, 100,100));

  Set<Point> points_coll{};
  CollinearityTestStruct coll{points_coll};
  IncrementalCollinear checker{instance, assignment, coll};
  checker.findCollinear(line);

  EXPECT_EQ(points_coll.size(), 2);
  EXPECT_TRUE(points_coll.contains(points_on_line[0]));
  EXPECT_TRUE(points_coll.contains(points_on_line[1]));
}


TEST(IncrementalCollinearTest, DoLinesIntersect_SimpleIntersection)
{
  point_pair_t line1 = point_pair_t { Point{0, 0, 0}, Point{1, 10, 10}};
  point_pair_t line2 = point_pair_t { Point{2, 10, 0}, Point{3, 0, 10}};
  EXPECT_TRUE(gd::intersect(line1.first, line1.second, line2.first, line2.second));
}

TEST(IncrementalCollinearTest, DoLinesIntersect_Collinear)
{
  point_pair_t line1 = point_pair_t { Point{0, 0, 0}, Point{1, 10, 10}};
  point_pair_t line2 = point_pair_t { Point{2, 10, 0}, Point{3, 20, 10}};
  EXPECT_FALSE(gd::intersect(line1.first, line1.second, line2.first, line2.second));
}

TEST(IncrementalCollinearTest, DoLinesIntersect_SlightIntersection)
{
  point_pair_t line1 = point_pair_t { Point{0, 0, 0}, Point{1, (int)2e8, 2}};
  point_pair_t line2 = point_pair_t { Point{2, ((int)1e8) - 1, 0}, Point{3, ((int)1e8) - 1, 1}};
  EXPECT_TRUE(gd::intersect(line1.first, line1.second, line2.first, line2.second));
}

TEST(IncrementalCollinearTest, DoLinesIntersect_WeirdBug)
{
  point_pair_t line1 = point_pair_t { Point{15, 1, 0}, Point{40, 2, 10}};
  point_pair_t line2 = point_pair_t { Point{32, 2, 2}, Point{48, 3, 3}};
  EXPECT_FALSE(gd::intersect(line1.first, line1.second, line2.first, line2.second));
  EXPECT_FALSE(gd::intersect(line2.first, line2.second, line1.first, line1.second));
}

namespace
{
  void check_same_crossings(const instance_t& instance, IncrementalCrossing& inc1, IncrementalCrossing& inc2)
  {
    for (vertex_t v = 0; v != instance.m_graph.getNbVertices(); ++v)
    {
      EXPECT_EQ(inc1.getNumCrossings(v), inc2.getNumCrossings(v));
    }
  }

  void built_different_permutations(const instance_t& instance, const VertexAssignment& assignment,
    std::initializer_list<vertex_t> permutation1, std::initializer_list<vertex_t> permutation2)
  {
    VertexAssignment assign1 {instance.m_graph.getNbVertices()};
    VertexAssignment assign2 {instance.m_graph.getNbVertices()};
    IncrementalCrossing inc1{instance, assign1};
    IncrementalCrossing inc2{instance, assign2};
    for (vertex_t v : permutation1)
    {
      inc1.place(v, assignment.getAssigned(v));
      assign1.assign(v, assignment.getAssigned(v));
    }
    for (vertex_t v : permutation2)
    {
      inc2.place(v, assignment.getAssigned(v));
      assign2.assign(v, assignment.getAssigned(v));
    }
    check_same_crossings(instance, inc1, inc2);
  }
}

TEST(IncrementalCrossingTest, SameCrossings_Different_Permutations)
{
  instance_t instance = create_mock_instance(
    {
      PAIR(0, 1),
      PAIR(2, 3),
      PAIR(4, 5)
    },
    {
      Point(0, 1, 0),
      Point(1, -1, 0),

      Point(2, 0, 1),
      Point(3, 0, -1),

      Point(4, 1, 1),
      Point(5, -1, -1)
    });
  VertexAssignment assignment = create_mock_assignment(6, {
    PAIR(0, 0),
    PAIR(1, 1),
    PAIR(2, 2),
    PAIR(3, 3),
    PAIR(4, 4),
    PAIR(5, 5),
  });

  built_different_permutations(instance, assignment,
    {0,1,2,3,4,5},
    {5,4,3,2,1,0});

  VertexAssignment as{instance.m_graph.getNbVertices()};
  IncrementalCrossing inc{instance, as};
  IncrementalCrossing inc_static{instance, as};

  for (vertex_t v = 0; v < instance.m_graph.getNbVertices(); ++v)
  {
    inc.place(v, assignment.getAssigned(v));
    inc_static.place(v, assignment.getAssigned(v));
    as.assign(v, assignment.getAssigned(v));
  }
  // deplace and place now
  inc.deplace(3, 3);
  inc.place(3, 3);
  check_same_crossings(instance, inc, inc_static);
}
