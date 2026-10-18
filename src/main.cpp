#include <graph.hpp>
#include <loader.hpp>
#include <print>

int main() {
  graph::Graph graph = loader::load_json("data/relations.json");

  [[maybe_unused]] const auto bfs = graph.bfs(0);
  [[maybe_unused]] const auto dfs = graph.dfs(0);
  for (const auto &l : dfs) {
    for (const auto n : l) {
      std::print("{} ", n);
    }
    std::println();
  }
  [[maybe_unused]] const auto path = graph.shortest_path(0, 1);
}
