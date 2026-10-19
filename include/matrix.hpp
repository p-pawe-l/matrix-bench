#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <latch>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include <initializer_list>
#include <utility>

template <typename T>
struct Matrix {
private:
	template <typename A>
	using arr_t = std::initializer_list<A>;

public:
	std::vector<T> data_;
	int rows_;
	int cols_;

	Matrix(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}

	Matrix(arr_t<arr_t<T>> data): Matrix(data.size(), data.begin()->size())
	{
		auto row_it = data.begin();
		for (int i = 0; i < rows_; ++i) {
			auto it = row_it->begin();
			for (int j = 0; j < cols_; ++j) {
				data_[cols_ * i + j] = *it;
				++it;
			}
			++row_it;
		}
	}
};


// Prints the matrix with box-drawing brackets and right-aligned columns.
// Matrices larger than max_rows x max_cols are elided with ⋮ / ⋯ / ⋱.
template <typename T>
auto print_matrix(const Matrix<T>& m, std::ostream& os = std::cout,
                  int precision = 3, int max_rows = 10, int max_cols = 10) -> void {
	os << m.rows_ << " × " << m.cols_ << "\n";
	if (m.rows_ == 0 || m.cols_ == 0) {
		os << "[ ]\n";
		return;
	}

	// Indices to display; -1 marks the elided gap.
	auto pick = [](int n, int max) {
		std::vector<int> idx;
		if (n <= max) {
			for (int i = 0; i < n; ++i) idx.push_back(i);
			return idx;
		}
		int head = (max + 1) / 2;
		int tail = max / 2;
		for (int i = 0; i < head; ++i) idx.push_back(i);
		idx.push_back(-1);
		for (int i = n - tail; i < n; ++i) idx.push_back(i);
		return idx;
	};
	auto rows = pick(m.rows_, max_rows);
	auto cols = pick(m.cols_, max_cols);

	auto fmt = [&](const T& v) {
		std::ostringstream ss;
		if constexpr (std::is_floating_point_v<T>) {
			ss << std::fixed << std::setprecision(precision);
		}
		ss << v;
		return ss.str();
	};

	std::vector<std::vector<std::string>> cells(rows.size(), std::vector<std::string>(cols.size()));
	std::vector<std::size_t> width(cols.size(), 1);
	for (std::size_t r = 0; r < rows.size(); ++r) {
		for (std::size_t c = 0; c < cols.size(); ++c) {
			if (rows[r] < 0 || cols[c] < 0) continue;
			cells[r][c] = fmt(m.data_[m.cols_ * rows[r] + cols[c]]);
			width[c] = std::max(width[c], cells[r][c].size());
		}
	}

	for (std::size_t r = 0; r < rows.size(); ++r) {
		bool first = r == 0;
		bool last = r == rows.size() - 1;
		const char* left  = rows.size() == 1 ? "[" : first ? "┌" : last ? "└" : "│";
		const char* right = rows.size() == 1 ? "]" : first ? "┐" : last ? "┘" : "│";

		os << left;
		for (std::size_t c = 0; c < cols.size(); ++c) {
			os << ' ';
			if (rows[r] < 0 || cols[c] < 0) {
				// Glyphs are multi-byte but one column wide, so pad by hand.
				const char* glyph = rows[r] < 0 && cols[c] < 0 ? "⋱" : rows[r] < 0 ? "⋮" : "⋯";
				os << std::string(width[c] - 1, ' ') << glyph;
			} else {
				os << std::string(width[c] - cells[r][c].size(), ' ') << cells[r][c];
			}
		}
		os << ' ' << right << '\n';
	}
}

template <typename T> struct mul_cfg;

template <typename T>
auto thread_task(int row_id, const mul_cfg<T>& cfg) -> void {
	auto calc_row = cfg.out->cols_ * row_id;
	
	for (int j = 0; j < cfg.m2.rows_; ++j) {
		auto scalar = cfg.m1.data_[cfg.m1.cols_ * row_id + j];
		
		for (int k = 0; k < cfg.m2.cols_; ++k) {
			auto m2_row_data = cfg.m2.data_[cfg.m2.cols_ * j + k];
			cfg.out->data_[calc_row + k] += scalar * m2_row_data;
		}
	}
}

template <typename T>
struct mul_cfg {
	Matrix<T> m1;
	Matrix<T> m2;
	Matrix<T> *out;

	// Not needed for non multithreaded mul operations.
	// Threads are created here and block on start_ until mul_multithread releases them.
	// mutable so mul_multithread can drive them through a const cfg
	mutable std::vector<std::thread> threads_;
	mutable std::latch start_{1};
	mutable bool started_ = false;
	bool cancelled_ = false;

	mul_cfg(Matrix<T> m1, Matrix<T> m2, Matrix<T>* out)
		: m1(std::move(m1)), m2(std::move(m2)), out(out) {
		threads_.reserve(this->m1.rows_);
		for (int i = 0; i < this->m1.rows_; ++i) {
			threads_.emplace_back([this, i] {
				start_.wait();
				if (!cancelled_) thread_task<T>(i, *this);
			});
		}
	}

	// Threads hold `this`, so the cfg must stay in place.
	mul_cfg(const mul_cfg&) = delete;
	mul_cfg& operator=(const mul_cfg&) = delete;

	~mul_cfg() {
		if (!started_) {
			// mul_multithread never ran: wake threads and let them exit without work
			cancelled_ = true;
			start_.count_down();
		}
		for (auto& t : threads_) {
			if (t.joinable()) t.join();
		}
	}
};

template <typename T>
auto mul_row_prior(const mul_cfg<T>& cfg) -> void {	
	/*
	* in this function we are filling the whole row of out matrix
	* and if whole row is calculated we are going to the next column 
	* and in the same way we are calculating the second column
	*/

	for (int i = 0; i < cfg.m1.rows_; ++i) {
		
		for (int j = 0; j < cfg.m2.cols_; ++j) {
			auto calc_index = cfg.out->cols_ * i + j;
			
			for (int z = 0; z < cfg.m1.cols_; ++z) {
				auto m1_row_data = cfg.m1.data_[cfg.m1.cols_ * i + z];
				auto m2_column_data = cfg.m2.data_[cfg.m2.cols_ * z + j];
				cfg.out->data_[calc_index] += m1_row_data * m2_column_data;
			}
		}
	}

}

template <typename T>
auto mul_col_prior(const mul_cfg<T>& cfg) -> void {
	/*
	* in this function we are calculating the whole column of the next matrix
	* and after calculating this column we are going to the next row and in the 
	* same way we are calculating the next column of out matrix
	*/

	for (int i = 0; i < cfg.m2.cols_; ++i) {
		
		for (int j = 0; j < cfg.m1.rows_; ++j) {
			auto calc_index = cfg.out->cols_ * j + i;
			
			for (int z = 0; z < cfg.m1.cols_; ++z) {
				auto m1_row_data = cfg.m1.data_[cfg.m1.cols_ * j + z];
				auto m2_column_data = cfg.m2.data_[cfg.m2.cols_ * z + i];
				cfg.out->data_[calc_index] += m1_row_data * m2_column_data;
			}
		}

	}

}

template <typename T>
auto mul(const mul_cfg<T>& cfg) -> void {
	/*
	* in this function we are calculating the whole column of the next matrix
	* and after calculating this column we are going to the next row and in the 
	* same way we are calculating the next column of out matrix
	*/

	for (int i = 0; i < cfg.m1.rows_; ++i) {
		
		for (int j = 0; j < cfg.m2.rows_; ++j) {
			auto calc_row = cfg.out->cols_ * i;
			auto scalar = cfg.m1.data_[cfg.m1.cols_ * i + j];
			
			for (int k = 0; k < cfg.m2.cols_; ++k) {
				auto m2_row_data = cfg.m2.data_[cfg.m2.cols_ * j + k];
				cfg.out->data_[calc_row + k] += scalar * m2_row_data;
			}
		}
	}

}

template <typename T>
auto mul_multithread(const mul_cfg<T>& cfg) -> void {
    // Threads are one-shot: they were created in the cfg constructor
    if (cfg.started_)
        throw std::logic_error("mul_multithread can only run once per mul_cfg");
    cfg.started_ = true;
    cfg.start_.count_down();

    for (auto& t : cfg.threads_) {
        t.join();
    }
}