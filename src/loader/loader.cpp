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

    const NodeId id{static_cast<std::int64_t>(obj["id"].get_int64())};
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

std::vector<ExternalEdge> parse_edges(simdjson::ondemand::array json_edges) {
  std::vector<ExternalEdge> edges;

  for (auto value : json_edges) {
    auto obj = value.get_object();

    const NodeId source{static_cast<std::int64_t>(obj["source"].get_int64())};
    const NodeId target{static_cast<std::int64_t>(obj["target"].get_int64())};
    const Relation relation = parse_relation(obj["type"].get_string());

    edges.push_back(ExternalEdge{
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
