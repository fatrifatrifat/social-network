#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

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

  [[nodiscard]] static Graph from_edges(std::vector<Node> nodes,
                                        std::span<const Edge> relations);

  [[nodiscard]] std::span<const Edge> neighbors(const VertexId id) const;
  std::vector<std::vector<VertexId>> bfs(VertexId a) const;
  std::vector<std::vector<VertexId>> dfs(VertexId a) const;
  [[nodiscard]] std::optional<std::vector<VertexId>>
  shortest_path(VertexId a, VertexId b) const;
};

} // namespace graph
