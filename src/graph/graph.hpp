#pragma once
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

  void parse_nodes(auto json_nodes);
  void parse_raw_edges(auto json_edges);
  void parse_json(std::string_view str);
  Relation parse_relation(std::string_view str);
  void process_offsets(std::span<const uint64_t> degrees);

  std::span<const Edge> neighbors(const VertexId id) const;
  void bfs(VertexId a) const;
  void dfs(VertexId a) const;
  void shortest_path(VertexId a, VertexId b) const;
};

} // namespace graph
