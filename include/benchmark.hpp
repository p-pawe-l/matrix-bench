#pragma once

#include <functional>
#include <stdexcept>

#include "matrix.hpp"

struct bench_result {
    unsigned long row_prior_mul;
    unsigned long col_prior_mul;
    unsigned long mul;
    unsigned long multithread_mul;
};

auto mes_time(const std::function<void()>& f) -> unsigned long;

template <typename T>
auto bench_matrix_mul(const mul_cfg<T>& cfg) -> bench_result {
    if (cfg.m1.cols_ != cfg.m2.rows_)
        throw std::invalid_argument("m1.cols_ and m2.rows_ need to be in the same dim!");
    if (cfg.out->rows_ != cfg.m1.rows_ && cfg.out->cols_ != cfg.m2.cols_)
        throw std::invalid_argument("out matrix need to be (m1.rows_, m2.cols_) dimension");

    return {
        .row_prior_mul = mes_time([&] { mul_row_prior<T>(cfg); }),
        .col_prior_mul = mes_time([&] { mul_col_prior<T>(cfg); }),
        .multithread_mul = mes_time([&] { mul_multithread<T>(cfg); }),
        .mul = mes_time([&] { mul<T>(cfg); }),
    };
}
