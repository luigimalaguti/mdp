#pragma once

#include "containers.hpp"

#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

namespace pam {
    template <typename T>
    using matrix = containers::matrix<T>;

    template <typename T>
    using rgb = std::array<T, 3>;

    template <typename T>
    struct pixel_type;

    template <>
    struct pixel_type<uint8_t> {
        static constexpr int64_t depth = 1;
        static constexpr const char *typltype = "GRAYSCALE";
    };

    template <>
    struct pixel_type<rgb<uint8_t>> {
        static constexpr int64_t depth = 3;
        static constexpr const char *typltype = "RGB";
    };

    template <typename T = uint8_t>
    matrix<T> read_image(const std::string &filename) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return matrix<T>();
        }

        int64_t width = -1, height = -1, depth = -1, max_value = -1;
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

        if (width < 0 || height < 0 || depth != pixel_type<T>::depth || max_value != 255 ||
            tuple_type != pixel_type<T>::tupltype) {
            return matrix<T>();
        }

        matrix<T> data(height, width);
        is.read(data.data(), static_cast<int64_t>(data.raw_size()));
        return data;
    }

    template <typename T = uint8_t>
    bool write_image(const std::string &filename, const matrix<T> &data) {
        std::ofstream os(filename, std::ios::binary);
        if (!os) {
            return false;
        }

        os << "P7\n";
        os << "WIDTH " << data.cols() << "\n";
        os << "HEIGHT " << data.rows() << "\n";
        os << "DEPTH " << pixel_type<T>::depth << "\n";
        os << "MAXVAL 255\n";
        os << "TUPLTYPE " << pixel_type<T>::typltype << "\n";
        os << "ENDHDR\n";
        os.write(data.raw_data(), static_cast<int64_t>(data.raw_size()));

        return true;
    }

    template <typename T = uint8_t>
    class image {
        matrix<T> data_;

      public:
        image(uint32_t width, uint32_t height) : data_(height, width) {}

        explicit image(const std::string &filename) : data_(read_image<T>(filename)) {}

        explicit image(matrix<T> data) : data_(std::move(data)) {}

        const auto &operator[](uint32_t row, uint32_t col) const {
            return data_[row, col];
        }

        auto &operator[](uint32_t row, uint32_t col) {
            return data_[row, col];
        }

        auto begin() const {
            return data_.begin();
        }

        auto end() const {
            return data_.end();
        }

        uint32_t rows() const {
            return data_.rows();
        }

        uint32_t cols() const {
            return data_.cols();
        }

        size_t size() const {
            return data_.size();
        }

        bool empty() const {
            return data_.empty();
        }

        auto &data() {
            return data_;
        }

        size_t raw_size() const {
            return data_.raw_size();
        }

        const char *raw_data() const {
            return reinterpret_cast<const char *>(data_.raw_data());
        }

        char *raw_data() {
            return reinterpret_cast<char *>(data_.raw_data());
        }

        bool save(const std::string &filename) const {
            return write_image(filename, data_);
        }
    };
}  // namespace pam
