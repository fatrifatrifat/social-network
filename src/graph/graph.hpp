#pragma once
#include <map>
#include <print>
#include <simdjson.h>

namespace graph {

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

  void parse_nodes(auto json_nodes) {
    for (auto value : json_nodes) {
      auto obj = value.get_object();

      const VertexId id = static_cast<VertexId>(obj["id"].get_uint64());
      const std::string_view name = obj["name"].get_string();
      const std::uint16_t age =
          static_cast<std::uint16_t>(obj["age"].get_uint32());

      nodes.push_back(Node{
          .id = id,
          .name = std::string(name),
          .age = age,
      });
    }
  }

  void process_offsets(std::span<const uint64_t> degrees) {
    offsets.resize(nodes.size() + 1);
    for (auto i{0uz}; i < degrees.size(); ++i)
      offsets[i + 1] = offsets[i] + degrees[i];
  }

  void parse_edges_per_id(auto json_edges) {
    std::map<VertexId, std::vector<Edge>> edges_per_id;
    std::vector<std::uint64_t> degrees(nodes.size());

    for (auto value : json_edges) {
      auto obj = value.get_object();

      const VertexId source = static_cast<VertexId>(obj["source"].get_uint64());
      const VertexId target = static_cast<VertexId>(obj["target"].get_uint64());
      const Relation relation = Graph::parse_relation(obj["type"].get_string());

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

    process_offsets(std::span{degrees});

    for (const auto &[_, edges_] : edges_per_id) {
      for (const auto &e : edges_) {
        edges.push_back(e);
      }
    }
  }

  void parse_raw_edges(auto json_edges) {
    std::vector<Edge> raw_edges;

    for (auto value : json_edges) {
      auto obj = value.get_object();

      const VertexId source = static_cast<VertexId>(obj["source"].get_uint64());
      const VertexId target = static_cast<VertexId>(obj["target"].get_uint64());
      const Relation relation = Graph::parse_relation(obj["type"].get_string());

      raw_edges.push_back(Edge{
          .from = source,
          .to = target,
          .relation = relation,
      });

      if (relation != Relation::Follows) {
        raw_edges.push_back(Edge{
            .from = target,
            .to = source,
            .relation = relation,
        });
      }
    }

    {
      std::vector<std::uint64_t> degrees(nodes.size());

      for (const auto &e : raw_edges)
        ++degrees[e.from];

      process_offsets(std::span{degrees});

      edges.resize(raw_edges.size());
      auto positions = offsets;

      for (const auto &e : raw_edges)
        edges[positions[e.from]++] = e;
    }
  }

  void parse_json(std::string_view str) {
    using namespace simdjson;
    ondemand::parser parser;
    auto json = simdjson::padded_string::load(str);
    ondemand::document doc = parser.iterate(json);

    parse_nodes(doc["nodes"].get_array());
    // parse_raw_edges(doc["edges"].get_array());
    parse_edges_per_id(doc["edges"].get_array());

    for (const auto &node : nodes) {
      std::println("\n{}'s relations:", node.name);
      for (const auto &n : neighbors(node.id))
        std::println("{} -> {} by {}", n.from, n.to,
                     static_cast<std::uint8_t>(n.relation));
    }
  }

  Relation parse_relation(std::string_view str) {
    if (str == "friend")
      return Relation::Friend;

    if (str == "follows")
      return Relation::Follows;

    if (str == "coworker")
      return Relation::Coworker;

    std::terminate();
  }
};

} // namespace graph
