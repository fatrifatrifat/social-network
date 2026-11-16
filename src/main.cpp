#include <exception>
#include <fstream>
#include <graph.hpp>
#include <loader.hpp>
#include <pqxx/pqxx>
#include <print>
#include <utility>

namespace {

void to_psql(const graph::Graph &graph) {
  try {
    pqxx::connection cx{"dbname=social-network-graph"};
    pqxx::work tx{cx};

    std::unordered_map<int, int> psql_id_mapping;
    for (const auto &n : graph.nodes) {
      const auto node_id =
          tx.exec("INSERT INTO nodes DEFAULT VALUES RETURNING id")
              .one_row()[0]
              .as<int>();
      psql_id_mapping[std::to_underlying(n.id)] = node_id;

      tx.exec(
          "INSERT INTO node_versions (node_id, name, age) VALUES ($1, $2, $3)",
          {node_id, n.name, n.age});
    }

    for (const auto &e : graph.edges) {
      const auto external_source_id =
          psql_id_mapping[std::to_underlying(graph.id_mapper.to_ext[e.from])];
      const auto external_target_id =
          psql_id_mapping[std::to_underlying(graph.id_mapper.to_ext[e.to])];
      tx.exec(
          "INSERT INTO edges (source_id, target_id, type) VALUES ($1, $2, $3)",
          {external_source_id, external_target_id,
           graph::Graph::to_string(e.relation)});
    }

    for (const auto id : tx.query<int>("SELECT * FROM nodes")) {
      std::println("[NODE]: {}", id);
    }

    for (const auto [id, node_id, name, age] :
         tx.query<int, int, std::string, int>("SELECT * FROM node_versions")) {
      std::println("[NODE_VERSION]: {}, {}, {}, {}", id, node_id, name, age);
    }

    for (const auto [id, source_id, target_id, type] :
         tx.query<int, int, int, std::string>("SELECT * FROM edges")) {
      std::println("[EDGE]: {}, {}, {}, {}", id, source_id, target_id, type);
    }

    tx.commit();
  } catch (const std::exception &e) {
    std::println("{}", e.what());
  }
}
}; // namespace

int main() {
  graph::Graph graph = loader::load_json("data/relations.json");

  to_psql(graph);

  [[maybe_unused]] const auto bfs = graph.bfs(graph::NodeId{0});
  std::println("[BFS]:");
  for (const auto &l : bfs) {
    for (const auto &v : l) {
      std::print("{} ", std::to_underlying(v));
    }
    std::println();
  }

  [[maybe_unused]] const auto dfs = graph.dfs(graph::NodeId{0});
  std::println("[DFS]:");
  for (const auto &l : dfs) {
    for (const auto &v : l) {
      std::print("{} ", std::to_underlying(v));
    }
    std::println();
  }

  std::println("[KOSARAJU]:");
  [[maybe_unused]] const auto kosaraju = graph.kosaraju();
  for (const auto &l : kosaraju) {
    for (const auto &v : l) {
      std::print("{} ", std::to_underlying(v));
    }
    std::println();
  }

  std::println("[SHORTEST_PATH]");
  [[maybe_unused]] const auto path =
      graph.shortest_path(graph::NodeId{0}, graph::NodeId{4});
  for (const auto &v : path.value()) {
    std::print("{} ", std::to_underlying(v));
  }

  // std::ofstream file("graph.dot");
  // file << graph;
}
