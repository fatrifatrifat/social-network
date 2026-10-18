#pragma once
#include <filesystem>
#include <graph.hpp>

namespace loader {

[[nodiscard]] graph::Graph load_json(const std::filesystem::path &path);

} // namespace loader
