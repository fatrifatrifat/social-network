#include <graph.hpp>
#include <gtest/gtest.h>
#include <loader.hpp>
#include <ranges>
#include <utility>

TEST(GraphTest, GraphLoading) {
  const auto &graph = loader::load_json("data/relations.json");
  EXPECT_EQ(graph.nodes.size(), 6);
  EXPECT_EQ(graph.edges.size(), 12);
}

TEST(GraphTest, GraphBFS) {
  const auto &graph = loader::load_json("data/kosaraju.json");
  const std::array<std::vector<graph::VertexId>, 8> expected = {{
      {0},
      {7, 1},
      {6, 8},
      {2},
      {9},
      {5},
      {4},
      {3},
  }};

  const auto &bfs = graph.bfs(graph::NodeId{0});
  for (const auto &[level_index, levels] : std::views::enumerate(bfs)) {
    for (const auto [node_idx, node] : std::views::enumerate(levels)) {
      EXPECT_EQ(std::to_underlying(bfs[level_index][node_idx]),
                expected[level_index][node_idx]);
    }
  }
}

TEST(GraphTest, GraphDFS) {
  const auto &graph = loader::load_json("data/kosaraju.json");
  const std::array<std::vector<graph::VertexId>, 8> expected = {{
      {0},
      {7},
      {6, 8},
      {2},
      {1, 5},
      {3},
      {4},
      {9},
  }};

  const auto &dfs = graph.dfs(graph::NodeId{0});
  for (const auto &[level_index, levels] : std::views::enumerate(dfs)) {
    for (const auto [node_idx, node] : std::views::enumerate(levels)) {
      EXPECT_EQ(std::to_underlying(dfs[level_index][node_idx]),
                expected[level_index][node_idx]);
    }
  }
}

TEST(GraphTest, GraphKosaraju) {
  const auto &graph = loader::load_json("data/kosaraju.json");
  const std::array<std::vector<graph::VertexId>, 8> expected = {{
      {7, 0},
      {8},
      {1, 3, 2, 5, 6},
      {9, 4},
  }};

  const auto &kosaraju = graph.kosaraju();
  for (const auto &[level_index, levels] : std::views::enumerate(kosaraju)) {
    for (const auto [node_idx, node] : std::views::enumerate(levels)) {
      EXPECT_EQ(std::to_underlying(kosaraju[level_index][node_idx]),
                expected[level_index][node_idx]);
    }
  }
}

TEST(GraphTest, GraphShortestPath) {
  const auto &graph = loader::load_json("data/kosaraju.json");
  const std::array<graph::VertexId, 5> expected = {{0, 7, 8, 9, 4}};

  const auto &shortest_path =
      graph.shortest_path(graph::NodeId{0}, graph::NodeId{4});
  for (const auto [node_idx, node] :
       std::views::enumerate(shortest_path.value())) {
    EXPECT_EQ(std::to_underlying(shortest_path.value()[node_idx]),
              expected[node_idx]);
  }
}
