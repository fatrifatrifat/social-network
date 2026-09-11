#include <cassert>
#include <pqxx/pqxx>
#include <print>
#include <simdjson.h>
#include <string>
#include <vector>

using namespace simdjson;

using VertexId = std::uint32_t;
using Weight = float;

enum class Relation : std::uint8_t { Friend, Follows, Coworker };

struct Node {
  VertexId id;
  std::string name;
  std::uint16_t age;
};

struct Edge {
  VertexId from;
  VertexId to;
  Relation relation;
};

struct Graph {
  std::vector<Node> nodes;
  std::vector<Edge> edges;
  std::vector<std::uint64_t> offsets;

  std::span<const Edge> neighbors(const VertexId id) {
    const auto begin = offsets[id];
    const auto end = offsets[id + 1];

    return std::span{edges.begin() + begin, end - begin};
  }
};

Relation parse_relation(std::string_view str) {
  if (str == "friend")
    return Relation::Friend;

  if (str == "follows")
    return Relation::Follows;

  if (str == "coworker")
    return Relation::Coworker;

  std::terminate();
}

int main() {
  std::println("Welcome to the: Social Network Graph");

  ondemand::parser parser;
  auto json = simdjson::padded_string::load("data/relations.json");
  ondemand::document doc = parser.iterate(json);

  Graph graph;

  for (auto value : doc["nodes"].get_array()) {
    auto obj = value.get_object();

    const VertexId id = static_cast<VertexId>(obj["id"].get_uint64());
    const std::string_view name = obj["name"].get_string();
    const std::uint16_t age =
        static_cast<std::uint16_t>(obj["age"].get_uint32());

    graph.nodes.push_back(Node{
        .id = id,
        .name = std::string(name),
        .age = age,
    });
  }

  {
    std::vector<std::uint64_t> degrees(graph.nodes.size());
    std::map<VertexId, std::vector<Edge>> edges_per_id;

    for (auto value : doc["edges"].get_array()) {
      auto obj = value.get_object();

      const VertexId source = static_cast<VertexId>(obj["source"].get_uint64());
      const VertexId target = static_cast<VertexId>(obj["target"].get_uint64());
      const Relation relation = parse_relation(obj["type"].get_string());

      ++degrees[source];
      edges_per_id[source].push_back(Edge{
          .from = source,
          .to = target,
          .relation = relation,
      });

      if (relation != Relation::Follows) {
        ++degrees[target];
        edges_per_id[target].push_back(Edge{
            .from = target,
            .to = source,
            .relation = relation,
        });
      }
    }

    for (const auto &[_, edges] : edges_per_id) {
      for (const auto &e : edges) {
        graph.edges.push_back(Edge{
            .from = e.from,
            .to = e.to,
            .relation = e.relation,
        });
      }
    }

    graph.offsets.resize(graph.nodes.size() + 1);
    for (auto i{0uz}; i < degrees.size(); ++i)
      graph.offsets[i + 1] = graph.offsets[i] + degrees[i];
  }

  for (const auto &node : graph.nodes) {
    std::println("\n{}'s relations:", node.name);
    for (const auto &n : graph.neighbors(node.id))
      std::println("{} -> {} by {}", n.from, n.to,
                   static_cast<std::uint8_t>(n.relation));
  }
}
