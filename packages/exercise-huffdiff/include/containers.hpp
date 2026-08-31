#pragma once

#include <cstdint>
#include <vector>

namespace containers {
    template <typename T = uint8_t>
    class matrix {
        uint32_t rows_ = 0;
        uint32_t cols_ = 0;
        std::vector<T> data_;

      public:
        matrix() {}

        matrix(uint32_t rows, uint32_t cols) : rows_(rows), cols_(cols), data_(rows * cols) {}

        const auto &operator[](uint32_t row, uint32_t col) const {
            return data_[row * cols_ + col];
        }

        auto &operator[](uint32_t row, uint32_t col) {
            return data_[row * cols_ + col];
        }

        auto begin() const {
            return data_.begin();
        }

        auto end() const {
            return data_.end();
        }

        uint32_t rows() const {
            return rows_;
        }

        uint32_t cols() const {
            return cols_;
        }

        bool empty() const {
            return data_.empty();
        }

        size_t raw_size() const {
            return data_.size() * sizeof(T);
        }

        const char *row_data() const {
            return reinterpret_cast<const char *>(data_.data());
        }

        char *row_data() {
            return reinterpret_cast<char *>(data_.data());
        }
    };

    template <typename T>
    matrix<T> flip(const matrix<T> &data) {
        matrix<T> result(data.rows(), data.cols());
        for (uint32_t row = 0; row < data.rows(); row++) {
            for (uint32_t col = 0; col < data.cols(); col++) {
                result[row, col] = data[data.rows() - 1 - row, col];
            }
        }
        return result;
    }

    template <typename T>
    matrix<T> mirror(const matrix<T> &data) {
        matrix<T> result(data.rows(), data.cols());
        for (uint32_t row = 0; row < data.rows(); row++) {
            for (uint32_t col = 0; col < data.cols(); col++) {
                result[row, col] = data[row, data.cols() - 1 - col];
            }
        }
        return result;
    }

    template <typename R, typename I>
    matrix<R> difference(const matrix<I> &data) {
        matrix<R> result(data.rows(), data.cols());
        for (uint32_t row = 0; row < data.rows(); row++) {
            for (uint32_t col = 0; col < data.cols(); col++) {
                if (row == 0 && col == 0) {
                    result[row, col] = data[row, col];
                } else if (col == 0) {
                    result[row, col] = data[row, col] - data[row - 1, col];
                } else {
                    result[row, col] = data[row, col] - data[row, col - 1];
                }
            }
        }
        return result;
    }

    template <typename R, typename I>
    matrix<R> restore(const matrix<I> &data) {
        matrix<R> result(data.rows(), data.cols());
        for (uint32_t row = 0; row < data.rows(); row++) {
            for (uint32_t col = 0; col < data.cols(); col++) {
                if (row == 0 && col == 0) {
                    result[row, col] = data[row, col];
                } else if (col == 0) {
                    result[row, col] = data[row, col] + result[row - 1, col];
                } else {
                    result[row, col] = data[row, col] + result[row, col - 1];
                }
            }
        }
        return result;
    }
}  // namespace containers
