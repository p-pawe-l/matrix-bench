#include <chrono>

#include "../include/benchmark.hpp"

#define TIME_MES_NOW std::chrono::high_resolution_clock::now()

auto mes_time(const std::function<void()>& f) -> unsigned long {
    auto start = TIME_MES_NOW;
    f();
    auto end = TIME_MES_NOW;
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

template <typename T>
auto bench_matrix_mul(const mul_cfg<T>& cfg) -> bench_result {
    return {
        .row_prior_mul = mes_time([&] { mul_row_prior(cfg); }),
        .col_prior_mul = mes_time([&] { mul_col_prior(cfg); }),
        .multithread_mul = mes_time([&] { mul_multithread(cfg); }),
    };
}