#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace graph {

// External id, as stored in the database. May have gaps and is never used to
// index an array. An enum class so it can't be mixed up with VertexId: the
// compiler rejects `VertexId v = node_id;` and vice versa
enum class NodeId : std::int64_t {};

// Internal index into the CSR arrays, always dense in [0, nodes.size())
using VertexId = std::uint32_t;
using Weight = float;

// When persisting node data to the psql database, with time and multiple
// deletes, the ids of the nodes may not be sequential anymore, and would have
// gaps between each other. So we can't rely on those ids to build our CSR graph
// (especially the offset array)
// So here we map the ids we read to actual sequential ids with no gaps that
// won't change
struct IdMapper {
  std::vector<NodeId> to_ext;                  // Mapping index -> id
  std::unordered_map<NodeId, VertexId> to_idx; // Mapping id -> index
  [[nodiscard]] VertexId add(const NodeId ext_id) {
    const auto [idx_it, inserted] =
        to_idx.try_emplace(ext_id, static_cast<VertexId>(to_ext.size()));
    if (inserted)
      to_ext.push_back(ext_id);
    return idx_it->second;
  }
};

enum class Relation : std::uint8_t { Friend, Follows, Coworker };

struct Node {
  NodeId id;
  std::string name;
  std::uint16_t age;
};

// Edge as read from the outside world, endpoints are external ids
struct ExternalEdge {
  NodeId from;
  NodeId to;
  Relation relation;
};

// Edge stored inside the graph, endpoints are internal indices
struct Edge {
  VertexId from;
  VertexId to;
  Relation relation;
};

struct Graph {
  std::vector<Node> nodes; // nodes[i] is the node of internal index i
  std::vector<Edge> edges;
  std::vector<std::uint64_t> offsets;
  IdMapper id_mapper;

  [[nodiscard]] static Graph
  from_edges(std::vector<Node> nodes, std::span<const ExternalEdge> relations);

  // Public API speaks external ids only
  [[nodiscard]] std::vector<std::vector<NodeId>> bfs(NodeId a) const;
  [[nodiscard]] std::vector<std::vector<NodeId>> dfs(NodeId a) const;
  [[nodiscard]] std::vector<NodeId> out_dfs(NodeId a) const;
  [[nodiscard]] std::optional<std::vector<NodeId>>
  shortest_path(NodeId a, NodeId b) const;
  std::vector<std::vector<NodeId>> kasoraju() const;

private:
  // Internal helpers speak indices only
  [[nodiscard]] std::span<const Edge> neighbors(VertexId idx) const;
  [[nodiscard]] std::vector<std::vector<NodeId>>
  to_external(const std::vector<std::vector<VertexId>> &levels) const;
  [[nodiscard]] std::vector<NodeId>
  to_external(const std::vector<VertexId> &levels) const;
};

} // namespace graph
