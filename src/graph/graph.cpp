#include <graph.hpp>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

namespace graph {

Graph Graph::from_edges(std::vector<Node> nodes,
                        std::span<const ExternalEdge> relations) {
  IdMapper id_mapper;
  for (auto i{0uz}; i < nodes.size(); ++i)
    if (id_mapper.add(nodes[i].id) != i)
      throw std::invalid_argument("duplicate node id");

  const auto to_edge = [&](NodeId from, NodeId to, Relation relation) {
    return Edge{.from = id_mapper.to_idx.at(from),
                .to = id_mapper.to_idx.at(to),
                .relation = relation};
  };

  std::vector<Edge> directed;
  directed.reserve(relations.size() * 2);

  for (const auto &e : relations) {
    directed.push_back(to_edge(e.from, e.to, e.relation));
    if (e.relation != Relation::Follows)
      directed.push_back(to_edge(e.to, e.from, e.relation));
  }

  Graph graph{.nodes = std::move(nodes),
              .edges = {},
              .offsets = {},
              .id_mapper = std::move(id_mapper)};

  graph.offsets.resize(graph.nodes.size() + 1);
  for (const auto &e : directed)
    ++graph.offsets[e.from + 1];
  for (auto i{1uz}; i < graph.offsets.size(); ++i)
    graph.offsets[i] += graph.offsets[i - 1];

  graph.edges.resize(directed.size());
  auto positions = graph.offsets;
  for (const auto &e : directed)
    graph.edges[positions[e.from]++] = e;

  return graph;
}

std::span<const Edge> Graph::neighbors(const VertexId idx) const {
  const auto begin = offsets[idx];
  const auto end = offsets[idx + 1];

  return std::span{edges.begin() + begin, end - begin};
}

std::vector<std::vector<NodeId>>
Graph::to_external(const std::vector<std::vector<VertexId>> &levels) const {
  return levels | std::views::transform([&](const auto &level) {
           return level | std::views::transform([&](VertexId idx) {
                    return id_mapper.to_ext[idx];
                  }) |
                  std::ranges::to<std::vector>();
         }) |
         std::ranges::to<std::vector>();
}

std::vector<std::vector<NodeId>> Graph::bfs(NodeId a_id) const {
  const VertexId a = id_mapper.to_idx.at(a_id);

  std::queue<VertexId> q;
  std::vector<bool> visited(nodes.size(), false);
  visited[a] = true;
  q.push(a);

  std::vector<std::vector<VertexId>> result{{a}};

  while (!q.empty()) {
    const auto q_size = q.size();

    for (auto i{0uz}; i < q_size; ++i) {
      const auto front = q.front();
      q.pop();

      std::vector<VertexId> level;
      for (const auto &n : neighbors(front)) {
        if (visited[n.to])
          continue;
        visited[n.to] = true;
        q.push(n.to);
        level.push_back(n.to);
      }
      if (!level.empty())
        result.push_back(std::move(level));
    }
  }

  return to_external(result);
}

std::vector<std::vector<NodeId>> Graph::dfs(NodeId a_id) const {
  const VertexId a = id_mapper.to_idx.at(a_id);

  std::vector<bool> visited(nodes.size(), false);
  visited[a] = true;

  std::vector<std::vector<VertexId>> result{{a}};

  auto recurse = [&](this auto &&self, const VertexId &top,
                     const std::size_t depth = 0) -> void {
    for (const auto &e : neighbors(top)) {
      if (visited[e.to])
        continue;
      visited[e.to] = true;

      if (result.size() <= depth + 1) {
        result.push_back({e.to});
      } else {
        result[depth + 1].push_back(e.to);
      }

      self(e.to, depth + 1);
    }
  };

  recurse(a);

  return to_external(result);
}

std::optional<std::vector<NodeId>> Graph::shortest_path(NodeId a_id,
                                                        NodeId b_id) const {
  const VertexId a = id_mapper.to_idx.at(a_id);
  const VertexId b = id_mapper.to_idx.at(b_id);

  std::queue<VertexId> q;
  std::vector<bool> visited(nodes.size(), false);
  std::vector<std::optional<VertexId>> parent(nodes.size());
  visited[a] = true;
  parent[a] = std::nullopt;
  q.push(a);

  std::vector<NodeId> lineage;
  while (!q.empty()) {
    const auto q_size = q.size();

    for (auto i{0uz}; i < q_size; ++i) {
      const auto front = q.front();
      q.pop();

      for (const auto &n : neighbors(front)) {
        if (visited[n.to])
          continue;

        parent[n.to] = n.from;
        visited[n.to] = true;
        q.push(n.to);

        if (n.to == b) {
          auto curr = n.to;
          while (parent[curr].has_value()) {
            lineage.push_back(id_mapper.to_ext[curr]);
            curr = parent[curr].value();
          }
          lineage.push_back(id_mapper.to_ext[curr]);

          return std::make_optional(lineage | std::views::reverse |
                                    std::ranges::to<std::vector>());
        }
      }
    }
  }

  return std::nullopt;
}

} // namespace graph
