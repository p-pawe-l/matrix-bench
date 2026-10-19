#pragma once

#include <functional>

#include "matrix.hpp"

struct bench_result {
    unsigned long row_prior_mul;
    unsigned long col_prior_mul;
    unsigned long multithread_mul;
};

auto mes_time(const std::function<void()>& f) -> unsigned long;

template <typename T>
auto bench_matrix_mul(const mul_cfg<T>& cfg) -> bench_result;
