#include <algorithm>
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

  std::vector<std::vector<VertexId>> result{{a}};

  auto recurse = [&](this auto &&self, const VertexId &top,
                     const std::size_t depth = 0) -> void {
    visited[top] = true;
    for (const auto &e : neighbors(top)) {
      if (visited[e.to])
        continue;

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

std::vector<std::vector<NodeId>> Graph::kosaraju() const {
  const auto node_count = nodes.size();

  std::vector<std::vector<VertexId>> components;
  std::vector<VertexId> order;
  std::vector<bool> visited(node_count, false);

  const auto adj = adjacency();

  auto out_dfs = [&visited, this](this auto &&self, const auto &top,
                                  const std::vector<std::vector<VertexId>> &adj,
                                  std::vector<VertexId> &output) -> void {
    visited[top] = true;
    for (const auto &v : adj[top]) {
      if (visited[v])
        continue;

      self(v, adj, output);
    }
    output.push_back(top);
  };

  for (auto idx{0uz}; idx < node_count; ++idx) {
    if (visited[idx])
      continue;
    out_dfs(idx, adj, order);
  }

  const auto adj_rev = reverse_adjacency();

  visited.assign(node_count, false);
  std::ranges::reverse(order);

  for (auto v : order) {
    if (!visited[v]) {
      std::vector<VertexId> component;
      out_dfs(v, adj_rev, component);
      components.push_back(component);
    }
  }

  return to_external(components);
}

std::vector<std::vector<VertexId>> Graph::adjacency() const {
  std::vector<std::vector<VertexId>> adj(nodes.size());
  for (auto v{0uz}; v < nodes.size(); ++v) {
    for (const auto &e : neighbors(v)) {
      adj[v].push_back(e.to);
    }
  }

  return adj;
}

std::vector<std::vector<VertexId>> Graph::reverse_adjacency() const {
  std::vector<std::vector<VertexId>> adj(nodes.size());
  for (auto v{0uz}; v < nodes.size(); ++v) {
    for (const auto &e : neighbors(v)) {
      adj[e.to].push_back(v);
    }
  }

  return adj;
}
} // namespace graph
