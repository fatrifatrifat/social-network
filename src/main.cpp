#include <graph.hpp>
#include <loader.hpp>

int main() {
  graph::Graph graph = loader::load_json("data/relations.json");

  [[maybe_unused]] const auto bfs = graph.bfs(0);
  [[maybe_unused]] const auto dfs = graph.dfs(0);
  [[maybe_unused]] const auto path = graph.shortest_path(0, 1);
}
