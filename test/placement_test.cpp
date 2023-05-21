#include "common/instance.hpp"
#include "common/misc.hpp"
#include "io/json_handling.hpp"
#include "placement/greedy_placement.hpp"
#include "verification/verifier.hpp"
#include <gd_types.hpp>
#include <stdexcept>
#include <test_helper/mock_data.hpp>
#include<io/placement_visualizer.hpp>

#include <gtest/gtest.h>

using namespace gd;

TEST(PlacementTest, TrivialPlacement_K4)
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
      Point(1, 1, 0),
      Point(2, 1, 1),
      Point(3, 0, 1)
  });

  PlacementVisualizer visualizer {instance, "Greedy/TrivialPlacement_K4/TrivialPlacement_K4"};
  GreedyPlacement placer {instance, &visualizer};
  const auto& assignment = placer.findPlacement();
  Verifier verifier {instance, assignment};
  size_t num_crossings;
  bool valid = verifier.verify(num_crossings);
  EXPECT_TRUE(valid);
  EXPECT_TRUE(isDefined(num_crossings));
  EXPECT_EQ(placer.getNumCrossings(), num_crossings);
}


TEST(PlacementTest, ImpossiblePlacement_AllCollinear)
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
      Point(1, 1, 0),
      Point(2, 2, 0),
      Point(3, 3, 0)
  });

  GreedyPlacement placer {instance};
  EXPECT_THROW(placer.findPlacement(), std::runtime_error);
}

TEST(PlacementTest, GData_Graph)
{
  instance_t instance = gd::parseInstanceFromFile("test/gdata/rome/grafo118.43.graphml.json");
  PlacementVisualizer visualizer {instance, "Greedy/Rome118.43Graph/Rome118.43Graph"};
  GreedyPlacement placer {instance, &visualizer};
  const auto& assignment = placer.findPlacement();
  Verifier verifier {instance, assignment};

  placer.improve(1000);

  size_t num_crossings;
  bool valid = verifier.verify(num_crossings);
  EXPECT_TRUE(valid);
  EXPECT_TRUE(isDefined(num_crossings));
  EXPECT_EQ(placer.getNumCrossings(), num_crossings);
}



TEST(PlacementTest, GData_Graph_4K4)
{
  instance_t instance = gd::parseInstanceFromFile("data/4K4.json");
  PlacementVisualizer visualizer {instance, "Greedy/4K4/4K4"};
  GreedyPlacement placer {instance, &visualizer};
  const auto& assignment = placer.findPlacement();
  Verifier verifier {instance, assignment};

  placer.improve(1000);

  size_t num_crossings;
  bool valid = verifier.verify(num_crossings);
  EXPECT_TRUE(valid);
  EXPECT_TRUE(isDefined(num_crossings));
  EXPECT_EQ(placer.getNumCrossings(), num_crossings);

}
