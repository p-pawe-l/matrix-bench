#include <iostream>
#include <random>

#include "include/benchmark.hpp"
#include "include/matrix.hpp"

template <typename T>
auto random_matrix(int rows, int cols, std::mt19937& gen) -> Matrix<T> {
	std::uniform_real_distribution<double> dist(-1.0, 1.0);
	Matrix<T> m(rows, cols);
	for (auto& v : m.data_) {
		v = static_cast<T>(dist(gen));
	}
	return m;
}

int main() {
	using T = double;
	std::mt19937 gen(42);

	{
		Matrix<T> result(2, 2);
		mul_cfg<T> cfg{
			.m1 = Matrix<T>{{1, 2}, {3, 4}},
			.m2 = Matrix<T>{{5, 6}, {7, 8}},
			.out = &result,
		};
		mul_row_prior(cfg);

		std::cout << "A = ";
		print_matrix(cfg.m1);
		std::cout << "B = ";
		print_matrix(cfg.m2);
		std::cout << "A * B = ";
		print_matrix(result);
		std::cout << "\n";
	}

	for (int n : {128, 256, 512, 1024, 2024}) {
		Matrix<T> out(n, n);
		mul_cfg<T> cfg{
			.m1 = random_matrix<T>(n, n, gen),
			.m2 = random_matrix<T>(n, n, gen),
			.out = &out,
		};

		auto res = bench_matrix_mul<double>(cfg);

		std::cout << n << "x" << n << ":\n"
		          << "  row prior: " << res.row_prior_mul << " ms\n"
		          << "  col prior: " << res.col_prior_mul << " ms\n";
	}

	return 0;
}
