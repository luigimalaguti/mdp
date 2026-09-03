#pragma once

#include "containers.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace pam {
    template <typename T>
    using matrix = containers::matrix<T>;

    template <typename T>
    using rgb = std::array<T, 3>;

    template <typename T>
    using rgba = std::array<T, 4>;

    template <typename T>
    struct pixel_type;

    template <>
    struct pixel_type<uint8_t> {
        static constexpr int64_t depth = 1;
        static constexpr const char *tuple_type = "GRAYSCALE";
    };

    template <>
    struct pixel_type<rgb<uint8_t>> {
        static constexpr int64_t depth = 3;
        static constexpr const char *tuple_type = "RGB";
    };

    template <>
    struct pixel_type<rgba<uint8_t>> {
        static constexpr int64_t depth = 4;
        static constexpr const char *tuple_type = "RGB_ALPHA";
    };

    struct header {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;
        uint32_t max_value = 0;
        std::string tuple_type;
    };

    header read_header(std::istream &is) {
        header result;
        std::string line, keyword;
        while (std::getline(is, line)) {
            if (line[0] == '#') {
                continue;
            }

            std::stringstream stream(line);
            stream >> keyword;

            if (keyword == "WIDTH") {
                stream >> result.width;
            } else if (keyword == "HEIGHT") {
                stream >> result.height;
            } else if (keyword == "DEPTH") {
                stream >> result.depth;
            } else if (keyword == "MAXVAL") {
                stream >> result.max_value;
            } else if (keyword == "TUPLTYPE") {
                stream >> result.tuple_type;
            } else if (keyword == "ENDHDR") {
                break;
            }
        }

        return result;
    }

    header read_header(const std::string &filename) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return {};
        }
        return read_header(is);
    }

    template <typename T>
    matrix<T> read_image(const std::string &filename) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return matrix<T>();
        }

        header info = read_header(is);

        if (info.width == 0 || info.height == 0 || info.depth != pixel_type<T>::depth || info.max_value != 255 ||
            info.tuple_type != pixel_type<T>::tuple_type) {
            return matrix<T>();
        }

        matrix<T> data(static_cast<uint32_t>(info.height), static_cast<uint32_t>(info.width));
        is.read(data.raw_data(), static_cast<int64_t>(data.raw_size()));
        return data;
    }

    template <typename T>
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
        os << "TUPLTYPE " << pixel_type<T>::tuple_type << "\n";
        os << "ENDHDR\n";
        os.write(data.raw_data(), static_cast<int64_t>(data.raw_size()));

        return true;
    }

    template <typename S>
    rgba<uint8_t> to_rgba(const S &pixel) {
        if constexpr (std::is_same_v<S, rgba<uint8_t>>) {
            return pixel;
        } else {
            static_assert(std::is_same_v<S, rgb<uint8_t>>, "to_rgba pixel_type not supported");
            return {pixel[0], pixel[1], pixel[2], 255};
        }
    }

    rgba<uint8_t> blend(const rgba<uint8_t> &a, const rgba<uint8_t> &b) {
        double alpha_a = a[3] / 255.0;
        double alpha_b = b[3] / 255.0;
        double alpha_0 = alpha_a + alpha_b * (1.0 - alpha_a);

        if (alpha_0 <= 0.0) {
            return {0, 0, 0, 0};
        }

        rgba<uint8_t> result{};
        for (size_t channel = 0; channel < 3; channel++) {
            double c_a = a[channel];
            double c_b = b[channel];
            double c_0 = (c_a * alpha_a + c_b * alpha_b * (1.0 - alpha_a)) / alpha_0;
            result[channel] = static_cast<uint8_t>(std::round(c_0));
        }
        result[3] = static_cast<uint8_t>(std::round(alpha_0 * 255.0));
        return result;
    }

    template <typename S>
    void compose(const matrix<S> &source, matrix<rgba<uint8_t>> &destination, uint32_t x, uint32_t y) {
        for (uint32_t row = 0; row < source.rows() && row + y < destination.rows(); row++) {
            for (uint32_t col = 0; col < source.cols() && col + x < destination.cols(); col++) {
                rgba<uint8_t> foreground = to_rgba(source[row, col]);
                destination[row + y, col + x] = blend(foreground, destination[row + y, col + x]);
            }
        }
    }

    template <typename T>
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

        uint32_t width() const {
            return data_.cols();
        }

        uint32_t height() const {
            return data_.rows();
        }

        bool empty() const {
            return data_.empty();
        }

        const matrix<T> &data() const {
            return data_;
        }

        bool save(const std::string &filename) const {
            return write_image(filename, data_);
        }

        template <typename S>
        image<T> &compose(const image<S> &source, uint32_t x, uint32_t y) {
            static_assert(std::is_same_v<T, rgba<uint8_t>>, "compose destination must be rgba<uint8_t>");
            pam::compose(source.data(), data_, x, y);
            return *this;
        }
    };
}  // namespace pam
