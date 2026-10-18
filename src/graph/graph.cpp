#include <graph.hpp>
#include <queue>
#include <stack>

namespace graph {

std::span<const Edge> Graph::neighbors(const VertexId id) const {
  const auto begin = offsets[id];
  const auto end = offsets[id + 1];

  return std::span{edges.begin() + begin, end - begin};
}

void Graph::parse_nodes(auto json_nodes) {
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

void Graph::process_offsets(std::span<const uint64_t> degrees) {
  offsets.resize(nodes.size() + 1);
  for (auto i{0uz}; i < degrees.size(); ++i)
    offsets[i + 1] = offsets[i] + degrees[i];
}

void Graph::parse_raw_edges(auto json_edges) {
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

void Graph::parse_json(std::string_view str) {
  using namespace simdjson;
  ondemand::parser parser;
  auto json = simdjson::padded_string::load(str);
  ondemand::document doc = parser.iterate(json);

  parse_nodes(doc["nodes"].get_array());
  parse_raw_edges(doc["edges"].get_array());
}

constexpr Relation Graph::parse_relation(std::string_view str) {
  if (str == "friend")
    return Relation::Friend;

  if (str == "follows")
    return Relation::Follows;

  if (str == "coworker")
    return Relation::Coworker;

  std::terminate();
}

void Graph::bfs(VertexId a) const {
  std::queue<VertexId> q;
  std::vector<bool> visited(nodes.size(), false);
  visited[a] = true;
  q.push(a);

  std::size_t level{};
  while (!q.empty()) {
    const auto q_size = q.size();

    for (auto i{0uz}; i < q_size; ++i) {
      const auto front = q.front();
      q.pop();

      for (const auto &n : neighbors(front)) {
        if (visited[n.to])
          continue;
        visited[n.to] = true;
        q.push(n.to);
      }
    }
  }
}

void Graph::dfs(VertexId a) const {
  std::stack<std::pair<VertexId, std::size_t>> stack;
  std::vector<bool> visited(nodes.size(), false);
  stack.push({a, 0});
  visited[a] = true;

  std::size_t level{};
  while (!stack.empty()) {
    const auto s_size = stack.size();
    const auto [top, depth] = stack.top();
    stack.pop();

    for (const auto &n : neighbors(top)) {
      if (visited[n.to])
        continue;
      visited[n.to] = true;
      stack.push({n.to, depth + 1});
    }
  }
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
