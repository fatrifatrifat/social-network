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

namespace {
std::vector<std::int64_t> insert_identity_rows(pqxx::work &tx,
                                               std::string_view table,
                                               std::size_t count) {
  const auto result =
      tx.exec(std::format("INSERT INTO {} SELECT FROM "
                          "generate_series(1, $1::integer) RETURNING id",
                          table),
              pqxx::params{static_cast<int>(count)});

  if (result.size() != count) {
    throw std::runtime_error(std::format("{}: expected {} ids, got {}", table,
                                         count, result.size()));
  }

  std::vector<std::int64_t> ids;
  ids.reserve(count);
  for (const auto &row : result) {
    ids.push_back(row[0].as<std::int64_t>());
  }
  return ids;
}
} // namespace

void to_psql(const graph::Graph &graph, std::string_view dbname) {
  pqxx::connection cx{std::format("dbname={}", dbname)};
  pqxx::work tx{cx};

  std::vector<std::int64_t> version_ids(graph.nodes.size());

  for (auto begin{0uz}; begin < graph.nodes.size(); begin += batch_size) {
    const auto count = std::min(batch_size, graph.nodes.size() - begin);
    const auto node_ids = insert_identity_rows(tx, "nodes", count);

    std::unordered_map<std::int64_t, std::size_t> node_id_to_idx;
    node_id_to_idx.reserve(count);

    std::string sql = "INSERT INTO node_versions (node_id, name, age) VALUES";
    for (auto i{0uz}; i < count; ++i) {
      const auto &node = graph.nodes[begin + i];
      node_id_to_idx.emplace(node_ids[i], begin + i);
      sql += std::format(" ({}, {}, {}),", node_ids[i], tx.quote(node.name),
                         node.age);
    }
    sql.back() = ' ';
    sql += "RETURNING id, node_id";

    for (const auto [version_id, node_id] :
         tx.query<std::int64_t, std::int64_t>(sql)) {
      version_ids[node_id_to_idx.at(node_id)] = version_id;
    }
  }

  const auto edges =
      graph.edges | std::views::filter([](const graph::Edge &e) {
        return e.relation == graph::Relation::Follows || e.from < e.to;
      }) |
      std::ranges::to<std::vector>();

  for (auto begin{0uz}; begin < edges.size(); begin += batch_size) {
    const auto count = std::min(batch_size, edges.size() - begin);
    const auto edge_ids = insert_identity_rows(tx, "edges", count);

    std::string sql = "INSERT INTO edge_versions (edge_id, source_id, "
                      "target_id, type) VALUES";

    for (auto i{0uz}; i < count; ++i) {
      const auto &edge = edges[begin + i];
      sql += std::format(" ({}, {}, {}, {}),", edge_ids[i],
                         version_ids[edge.from], version_ids[edge.to],
                         tx.quote(graph::Graph::to_string(edge.relation)));
    }
    sql.pop_back();

    tx.exec(sql);
  }

  tx.commit();
}

} // namespace loader
