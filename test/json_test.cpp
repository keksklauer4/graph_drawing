#include <gtest/gtest.h>
#include <io/json_handling.hpp>

using namespace gd;

TEST(JSONTest, ParseTriangle)
{
  instance_t triangle = gd::parseInstanceFromFile("data/triangle.json");
  ASSERT_EQ(triangle.m_graph.getNbVertices(), 3);
  ASSERT_EQ(triangle.m_graph.getNbEdges(), 3);
  ASSERT_EQ(triangle.m_points.getNumPoints(), 3);
}
