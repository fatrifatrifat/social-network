#include <fstream>
#include <graph.hpp>
#include <loader.hpp>
#include <print>
#include <utility>

constexpr std::string_view to_string(graph::Relation rel) {
  switch (rel) {
  case graph::Relation::Coworker:
    return "coworker";
  case graph::Relation::Follows:
    return "follows";
  case graph::Relation::Friend:
    return "friend";
  }
}

void write_dot(const graph::Graph &g, std::ostream &os,
               std::span<const int> color = {}) {
  os << "digraph G {\n";
  for (uint32_t u = 0; u < g.nodes.size(); ++u) {
    os << " " << u << " [label=" << g.nodes[u].name << "";
    if (!color.empty())
      os << " style=filled colorscheme=set312 fillcolor="
         << (color[u] % 12) + 1;
    os << "];\n";
    for (const auto &e : g.neighbors(u))
      os << " " << u << " -> " << e.to << " [label=" << to_string(e.relation)
         << "];\n";
  }
  os << "}\n";
}

int main() {
  graph::Graph graph = loader::load_json("data/kosaraju.json");

  [[maybe_unused]] const auto bfs = graph.bfs(graph::NodeId{0});

  [[maybe_unused]] const auto dfs = graph.kosaraju();
  for (const auto &l : dfs) {
    for (const auto &v : l) {
      std::print("{} ", std::to_underlying(v));
    }
    std::println();
  }

  [[maybe_unused]] const auto path =
      graph.shortest_path(graph::NodeId{0}, graph::NodeId{4});

  std::ofstream file("graph.txt");
  write_dot(graph, file);
}
