#pragma once

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace pam_image {
    template <typename T = uint8_t>
    class matrix {
        uint32_t rows_ = 0;
        uint32_t cols_ = 0;
        std::vector<T> data_;

      public:
        matrix() {}

        matrix(uint32_t width, uint32_t height) : rows_(height), cols_(width), data_(width * height) {}

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

        uint32_t width() const {
            return cols_;
        }

        uint32_t height() const {
            return rows_;
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

    matrix<uint8_t> read_image(const std::string &filename) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return matrix<uint8_t>();
        }

        int64_t width, height, depth, max_value;
        std::string tuple_type;

        std::string line;
        std::string keyword;
        while (std::getline(is, line)) {
            if (line[0] == '#') {
                continue;
            }

            std::stringstream stream(line);
            stream >> keyword;

            if (keyword == "WIDTH") {
                stream >> width;
            } else if (keyword == "HEIGHT") {
                stream >> height;
            } else if (keyword == "DEPTH") {
                stream >> depth;
            } else if (keyword == "MAXVAL") {
                stream >> max_value;
            } else if (keyword == "TUPLTYPE") {
                stream >> tuple_type;
            } else if (keyword == "ENDHDR") {
                break;
            }
        }

        if (width < 0 || height < 0 || depth != 1 || max_value != 255 || tuple_type != "GRAYSCALE") {
            return matrix<uint8_t>();
        }

        matrix<uint8_t> data(width, height);
        is.read(data.row_data(), static_cast<int32_t>(data.raw_size()));
        return data;
    }

    bool write_image(const std::string &filename, const matrix<uint8_t> &data) {
        std::ofstream os(filename, std::ios::binary);
        if (!os) {
            return false;
        }

        os << "P7\n";
        os << "WIDTH " << data.width() << "\n";
        os << "HEIGHT " << data.height() << "\n";
        os << "DEPTH 1\n";
        os << "MAXVAL 255\n";
        os << "TUPLTYPE GRAYSCALE\n";
        os << "ENDHDR\n";
        os.write(data.row_data(), static_cast<int32_t>(data.raw_size()));

        return true;
    }

    template <typename R, typename I>
    matrix<R> compute_difference(const matrix<I> &data) {
        matrix<R> difference(data.width(), data.height());
        for (uint32_t row = 0; row < data.rows(); row++) {
            for (uint32_t col = 0; col < data.cols(); col++) {
                if (row == 0 && col == 0) {
                    difference[row, col] = data[row, col];
                } else if (col == 0) {
                    difference[row, col] = data[row, col] - data[row - 1, col];
                } else {
                    difference[row, col] = data[row, col] - data[row, col - 1];
                }
            }
        }
        return difference;
    }

    template <typename R, typename I>
    matrix<R> restore_difference(const matrix<I> &data) {
        matrix<R> original(data.width(), data.height());
        for (uint32_t row = 0; row < data.rows(); row++) {
            for (uint32_t col = 0; col < data.cols(); col++) {
                if (row == 0 && col == 0) {
                    original[row, col] = data[row, col];
                } else if (col == 0) {
                    original[row, col] = data[row, col] + original[row - 1, col] - 255;
                } else {
                    original[row, col] = data[row, col] + original[row, col - 1] - 255;
                }
            }
        }
        return original;
    }
};  // namespace pam_image
