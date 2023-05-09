#include "gd_types.hpp"
#include <test_helper/mock_data.hpp>

#include <gtest/gtest.h>
#include <common/graph.hpp>

using namespace gd;


TEST(GraphTest, graph_K3_2)
{
  vertex_t v0 = 0;
  vertex_t v1 = 1;
  vertex_t v2 = 2;
  vertex_t v3 = 3;
  vertex_t v4 = 4;
  
  instance_t instance = create_mock_instance(
    {
      PAIR(v0, v3),
      PAIR(v0, v4),
      PAIR(v1, v3),
      PAIR(v1, v4),
      PAIR(v2, v3),
      PAIR(v2, v4)
    },
    {
      Point(0, 0, 0),
      Point(1, 1, 0),
      Point(2, 2, 0),
      Point(3, 0, 1),
      Point(4, 1, 1),
      Point(5, 2, 1),
      Point(6, 0, 2),
      Point(7, 1, 2),
      Point(8, 2, 2)
    });
  
  EXPECT_EQ(instance.m_graph.getNbVertices(), 5);

  EXPECT_EQ(instance.m_graph.getNbEdges(), 6);

  EXPECT_EQ(instance.m_graph.getDegree(0), 2);
  EXPECT_EQ(instance.m_graph.getDegree(1), 2);
  EXPECT_EQ(instance.m_graph.getDegree(2), 2);
  EXPECT_EQ(instance.m_graph.getDegree(3), 3);
  EXPECT_EQ(instance.m_graph.getDegree(4), 3);

  gd::Vector<vertex_t> neighbors_0{v3,v4};
  gd::Vector<vertex_t> neighbors_0_computed;
  auto range = instance.m_graph.getNeighborIterator(0);
  for(auto it = range.first; it != range.second; ++it){
    neighbors_0_computed.push_back(*it);
  }
  EXPECT_EQ(neighbors_0, neighbors_0_computed);

  gd::Vector<vertex_t> neighbors_1{v3,v4};
  gd::Vector<vertex_t> neighbors_1_computed;
  auto range1 = instance.m_graph.getNeighborIterator(1);
  for(auto it = range1.first; it != range1.second; ++it){
    neighbors_1_computed.push_back(*it);
  }
  EXPECT_EQ(neighbors_1, neighbors_1_computed);

  gd::Vector<vertex_t> neighbors_2{v3,v4};
  gd::Vector<vertex_t> neighbors_2_computed;
  auto range2 = instance.m_graph.getNeighborIterator(2);
  for(auto it = range2.first; it != range2.second; ++it){
    neighbors_2_computed.push_back(*it);
  }
  EXPECT_EQ(neighbors_2, neighbors_2_computed);

  gd::Vector<vertex_t> neighbors_3{v0,v1,v2};
  gd::Vector<vertex_t> neighbors_3_computed;
  auto range3 = instance.m_graph.getNeighborIterator(3);
  for(auto it = range3.first; it != range3.second; ++it){
    neighbors_3_computed.push_back(*it);
  }
  EXPECT_EQ(neighbors_3, neighbors_3_computed);

  gd::Vector<vertex_t> neighbors_4{v0,v1,v2};
  gd::Vector<vertex_t> neighbors_4_computed;
  auto range4 = instance.m_graph.getNeighborIterator(4);
  for(auto it = range4.first; it != range4.second; ++it){
    neighbors_4_computed.push_back(*it);
  }
  EXPECT_EQ(neighbors_4, neighbors_4_computed);
}