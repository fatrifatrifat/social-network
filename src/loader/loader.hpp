#pragma once
#include <filesystem>
#include <graph.hpp>
#include <pqxx/pqxx>

namespace loader {

[[nodiscard]] graph::Graph load_json(const std::filesystem::path &path);

constexpr std::size_t batch_size = 500;
void to_psql(const graph::Graph &graph, std::string_view dbname);

} // namespace loader
