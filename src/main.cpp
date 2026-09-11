#include <graph.hpp>

using namespace simdjson;

int main() {
  graph::Graph graph;
  graph.parse_json("data/relations.json");
}
