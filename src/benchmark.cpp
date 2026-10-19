#include <chrono>

#include "../include/benchmark.hpp"

#define TIME_MES_NOW std::chrono::high_resolution_clock::now()

auto mes_time(const std::function<void()>& f) -> unsigned long {
    auto start = TIME_MES_NOW;
    f();
    auto end = TIME_MES_NOW;
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}
