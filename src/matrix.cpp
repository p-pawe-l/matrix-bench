#include "../include/matrix.hpp"

template <typename T>
Matrix<T>::Matrix(int rows, int cols) {
	rows_ = rows;
	cols_ = cols;
	data_.resize(rows * cols);
}

template <typename T>
Matrix<T>::Matrix(arr_t<arr_t<T>> data): Matrix(data.size(), data.begin()->size())
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
