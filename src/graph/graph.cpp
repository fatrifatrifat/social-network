#include <graph.hpp>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <stack>
#include <vector>

namespace graph {

Graph Graph::from_edges(std::vector<Node> nodes,
                        std::span<const Edge> relations) {
  std::vector<Edge> directed;
  directed.reserve(relations.size() * 2);

  for (const auto &e : relations) {
    directed.push_back(e);
    if (e.relation != Relation::Follows)
      directed.push_back(
          Edge{.from = e.to, .to = e.from, .relation = e.relation});
  }

  Graph graph{.nodes = std::move(nodes), .edges = {}, .offsets = {}};

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

std::span<const Edge> Graph::neighbors(const VertexId id) const {
  const auto begin = offsets[id];
  const auto end = offsets[id + 1];

  return std::span{edges.begin() + begin, end - begin};
}

std::vector<std::vector<VertexId>> Graph::bfs(VertexId a) const {
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

  return result;
}

std::vector<std::vector<VertexId>> Graph::dfs(VertexId a) const {
  std::stack<std::pair<VertexId, std::size_t>> stack;
  std::vector<bool> visited(nodes.size(), false);
  stack.push({a, 0});
  visited[a] = true;

  std::vector<std::vector<VertexId>> result{{a}};

  while (!stack.empty()) {
    const auto [top, depth] = stack.top();
    stack.pop();

    for (const auto &n : neighbors(top)) {
      if (visited[n.to])
        continue;
      visited[n.to] = true;
      stack.push({n.to, depth + 1});

      if (result.size() <= depth + 2) {
        result.push_back({n.to});
      } else {
        result[depth + 2].push_back(n.to);
      }
    }
  }

  return result;
}

std::optional<std::vector<VertexId>> Graph::shortest_path(VertexId a,
                                                          VertexId b) const {
  std::queue<VertexId> q;
  std::vector<bool> visited(nodes.size(), false);
  std::vector<std::optional<VertexId>> parent(nodes.size());
  visited[a] = true;
  parent[a] = std::nullopt;
  q.push(a);

  std::vector<VertexId> lineage;
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
            lineage.push_back(curr);
            curr = parent[curr].value();
          }
          lineage.push_back(curr);

          return std::make_optional(lineage | std::views::reverse |
                                    std::ranges::to<std::vector>());
        }
      }
    }
  }

  return std::nullopt;
}

} // namespace graph
