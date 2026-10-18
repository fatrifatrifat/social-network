#include <graph.hpp>

int main() {
  graph::Graph graph;
  graph.parse_json("data/huge_relations.json");
  graph.bfs(0);
  graph.dfs(0);
  [[maybe_unused]] const auto path = graph.shortest_path(0, 1);
}
