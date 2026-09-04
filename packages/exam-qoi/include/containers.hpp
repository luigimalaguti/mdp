#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace containers {
    template <typename T>
    using rgba = std::array<T, 4>;

    template <typename T>
    class matrix {
        uint32_t rows_ = 0;
        uint32_t cols_ = 0;
        std::vector<rgba<T>> data_;

      public:
        matrix() {}

        matrix(uint32_t rows, uint32_t cols) : rows_(rows), cols_(cols), data_(rows * cols) {}

        const auto &operator()(size_t index) const {
            return data_[index];
        }

        auto &operator()(size_t index) {
            return data_[index];
        }

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

        size_t size() const {
            return data_.size();
        }

        bool empty() const {
            return data_.empty();
        }

        size_t raw_size() const {
            return size() * sizeof(rgba<T>);
        }

        const char *raw_data() const {
            return reinterpret_cast<const char *>(data_.data());
        }

        char *raw_data() {
            return reinterpret_cast<char *>(data_.data());
        }
    };
}  // namespace containers
