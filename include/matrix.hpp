#pragma once

#include <thread>
#include <vector>
#include <initializer_list>

template <typename T>
struct Matrix {
private:
	template <typename A>
	using arr_t = std::initializer_list<A>;

public:
	std::vector<T> data_;
	int rows_;
	int cols_;

	Matrix(int rows, int cols);
	Matrix(arr_t<arr_t<T>> data);
};


template <typename T>
struct mul_cfg {
	Matrix<T> m1;
	Matrix<T> m2;
	Matrix<T> *out;

	// Not needed for non multithreaded mul operations
	std::vector<std::thread> threads_;
};

template <typename T>
auto mul_row_prior(const mul_cfg<T>& cfg) -> void;

template <typename T>
auto mul_col_prior(const mul_cfg<T>& cfg) -> void;

template <typename T>
auto mul_multithread(const mul_cfg<T>& cfg) -> void;
