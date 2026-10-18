#include <graph.hpp>
#include <loader.hpp>
#include <print>
#include <utility>

int main() {
  graph::Graph graph = loader::load_json("data/relations.json");

  [[maybe_unused]] const auto bfs = graph.bfs(graph::NodeId{0});
  [[maybe_unused]] const auto dfs = graph.dfs(graph::NodeId{0});
  for (const auto &l : dfs) {
    for (const auto n : l) {
      std::print("{} ", std::to_underlying(n));
    }
    std::println();
  }
  [[maybe_unused]] const auto path = graph.shortest_path(graph::NodeId{0}, graph::NodeId{4});
}
