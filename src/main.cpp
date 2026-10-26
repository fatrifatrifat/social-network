#include <fstream>
#include <graph.hpp>
#include <loader.hpp>
#include <print>
#include <utility>

int main() {
  graph::Graph graph = loader::load_json("data/huge_relations.json");

  [[maybe_unused]] const auto bfs = graph.bfs(graph::NodeId{0});
  std::println("[BFS]:");
  for (const auto &l : bfs) {
    for (const auto &v : l) {
      std::print("{} ", std::to_underlying(v));
    }
    std::println();
  }

  [[maybe_unused]] const auto dfs = graph.dfs(graph::NodeId{0});
  std::println("[DFS]:");
  for (const auto &l : dfs) {
    for (const auto &v : l) {
      std::print("{} ", std::to_underlying(v));
    }
    std::println();
  }

  std::println("[KOSARAJU]:");
  [[maybe_unused]] const auto kosaraju = graph.kosaraju();
  for (const auto &l : kosaraju) {
    for (const auto &v : l) {
      std::print("{} ", std::to_underlying(v));
    }
    std::println();
  }

  std::println("[SHORTEST_PATH]");
  [[maybe_unused]] const auto path =
      graph.shortest_path(graph::NodeId{0}, graph::NodeId{4});
  for (const auto &v : path.value()) {
    std::print("{} ", std::to_underlying(v));
  }

  std::ofstream file("graph.dot");
  file << graph;
}
