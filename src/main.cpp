#include <exception>
#include <format>
#include <graph.hpp>
#include <loader.hpp>
#include <print>
#include <string_view>

namespace {} // namespace

int main() {
  try {
    const graph::Graph graph = loader::load_json("data/huge_relations.json");
    loader::to_psql(graph, "social-network-graph");
  } catch (const std::exception &e) {
    std::println(stderr, "{}", e.what());
    return 1;
  }
}
