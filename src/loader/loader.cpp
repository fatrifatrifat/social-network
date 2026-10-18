#include <loader.hpp>
#include <simdjson.h>

namespace loader {

using namespace graph;

namespace {

constexpr Relation parse_relation(std::string_view str) {
  if (str == "friend")
    return Relation::Friend;

  if (str == "follows")
    return Relation::Follows;

  if (str == "coworker")
    return Relation::Coworker;

  std::terminate();
}

std::vector<Node> parse_nodes(simdjson::ondemand::array json_nodes) {
  std::vector<Node> nodes;

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

  return nodes;
}

std::vector<Edge> parse_edges(simdjson::ondemand::array json_edges) {
  std::vector<Edge> edges;

  for (auto value : json_edges) {
    auto obj = value.get_object();

    const VertexId source = static_cast<VertexId>(obj["source"].get_uint64());
    const VertexId target = static_cast<VertexId>(obj["target"].get_uint64());
    const Relation relation = parse_relation(obj["type"].get_string());

    edges.push_back(Edge{
        .from = source,
        .to = target,
        .relation = relation,
    });
  }

  return edges;
}

} // namespace

Graph load_json(const std::filesystem::path &path) {
  simdjson::ondemand::parser parser;
  auto json = simdjson::padded_string::load(path.string());
  simdjson::ondemand::document doc = parser.iterate(json);

  auto nodes = parse_nodes(doc["nodes"].get_array());
  const auto edges = parse_edges(doc["edges"].get_array());

  return Graph::from_edges(std::move(nodes), edges);
}

} // namespace loader
