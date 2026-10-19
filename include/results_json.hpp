#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "benchmark.hpp"

struct bench_entry {
    int n;
    bench_result result;
};

// Writes benchmark results as:
// { "unit": "ms", "results": [ { "n": 128, "row_prior_mul": 4, ... }, ... ] }
inline auto save_results_json(const std::vector<bench_entry>& entries, const std::string& path) -> void {
    auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty())
        std::filesystem::create_directories(parent);

    std::ofstream file(path);
    if (!file)
        throw std::runtime_error("could not open " + path + " for writing");

    file << "{\n  \"unit\": \"ms\",\n  \"results\": [\n";
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const auto& [n, r] = entries[i];
        file << "    {"
             << "\"n\": " << n
             << ", \"row_prior_mul\": " << r.row_prior_mul
             << ", \"col_prior_mul\": " << r.col_prior_mul
             << ", \"multithread_mul\": " << r.multithread_mul
             << ", \"mul\": " << r.mul
             << "}" << (i + 1 < entries.size() ? "," : "") << "\n";
    }
    file << "  ]\n}\n";
}
