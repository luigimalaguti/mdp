#pragma once

#include "containers.hpp"

#include <cstdint>
#include <fstream>
#include <string>

namespace qoi {
    template <typename T>
    using matrix = containers::matrix<T>;

    template <typename T>
    using rgba = containers::rgba<T>;

    struct qoi_header {
        std::string magic_number;
        uint32_t width{0};
        uint32_t height{0};
        uint8_t channel{0};
        uint8_t color_space{0};
    };

    template <typename T>
    struct qoi_status {
        size_t pixel_index{0};
        rgba<T> previous_pixel{0, 0, 0, 255};
        std::vector<rgba<T>> pixels_seen{64};
    };

    uint8_t index_position(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return (r * 3 + g * 5 + b * 7 + a * 11) % 64;
    }

    qoi_header read_header(std::istream &is) {
        qoi_header header;
        header.magic_number = std::string(4, '\0');
        is.read(header.magic_number.data(), 4);

        uint32_t raw_width = 0, raw_height = 0;
        for (size_t index = 0; index < 4; index++) {
            is.read(reinterpret_cast<char *>(&raw_width), 1);
            header.width = (header.width << 8) | raw_width;
        }
        for (size_t index = 0; index < 4; index++) {
            is.read(reinterpret_cast<char *>(&raw_height), 1);
            header.height = (header.height << 8) | raw_height;
        }

        is.read(reinterpret_cast<char *>(&header.channel), 1);
        is.read(reinterpret_cast<char *>(&header.color_space), 1);

        return header;
    }

    template <typename T>
    void read_rgb_operation(std::istream &is, qoi_status<T> &status, matrix<T> &data) {
        rgba<T> pixel = {
            static_cast<T>(is.get()),
            static_cast<T>(is.get()),
            static_cast<T>(is.get()),
            status.previous_pixel[3],
        };

        data(status.pixel_index) = {
            pixel[0],
            pixel[1],
            pixel[2],
            pixel[3],
        };

        uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
        status.pixels_seen[index] = pixel;
        status.previous_pixel = pixel;
        status.pixel_index += 1;
    }

    template <typename T>
    void read_rgba_operation(std::istream &is, qoi_status<T> &status, matrix<T> &data) {
        rgba<T> pixel = {
            static_cast<T>(is.get()),
            static_cast<T>(is.get()),
            static_cast<T>(is.get()),
            static_cast<T>(is.get()),
        };

        data(status.pixel_index) = {
            pixel[0],
            pixel[1],
            pixel[2],
            pixel[3],
        };

        uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
        status.pixels_seen[index] = pixel;
        status.previous_pixel = pixel;
        status.pixel_index += 1;
    }

    template <typename T>
    void read_index_operation(uint8_t symbol, qoi_status<T> &status, matrix<T> &data) {
        uint8_t index = symbol & 0b00111111;

        data(status.pixel_index) = {
            status.pixels_seen[index][0],
            status.pixels_seen[index][1],
            status.pixels_seen[index][2],
            status.pixels_seen[index][3],
        };

        status.previous_pixel = status.pixels_seen[index];
        status.pixel_index += 1;
    }

    template <typename T>
    void read_diff_operation(uint8_t symbol, qoi_status<T> &status, matrix<T> &data) {
        int8_t diff_r = static_cast<int8_t>(((symbol & 0b00110000) >> 4) - 2);
        int8_t diff_g = static_cast<int8_t>(((symbol & 0b00001100) >> 2) - 2);
        int8_t diff_b = static_cast<int8_t>(((symbol & 0b00000011) >> 0) - 2);

        rgba<T> pixel = {
            status.previous_pixel[0],
            status.previous_pixel[1],
            status.previous_pixel[2],
            status.previous_pixel[3],
        };
        pixel[0] = static_cast<uint8_t>(pixel[0] + diff_r);
        pixel[1] = static_cast<uint8_t>(pixel[1] + diff_g);
        pixel[2] = static_cast<uint8_t>(pixel[2] + diff_b);
        data(status.pixel_index) = {
            pixel[0],
            pixel[1],
            pixel[2],
            pixel[3],
        };

        uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
        status.pixels_seen[index] = pixel;
        status.previous_pixel = pixel;
        status.pixel_index += 1;
    }

    template <typename T>
    void read_luma_operation(std::istream &is, uint8_t symbol, qoi_status<T> &status, matrix<T> &data) {
        int8_t diff_g = static_cast<int8_t>(((symbol & 0b00111111) >> 0) - 32);
        symbol = is.get();
        int8_t diff_r_g = static_cast<int8_t>(((symbol & 0b11110000) >> 4) - 8);
        int8_t diff_b_g = static_cast<int8_t>(((symbol & 0b00001111) >> 0) - 8);

        rgba<T> pixel = {
            status.previous_pixel[0],
            status.previous_pixel[1],
            status.previous_pixel[2],
            status.previous_pixel[3],
        };
        pixel[0] = static_cast<uint8_t>(pixel[0] + diff_r_g + diff_g);
        pixel[1] = static_cast<uint8_t>(pixel[1] + diff_g);
        pixel[2] = static_cast<uint8_t>(pixel[2] + diff_b_g + diff_g);
        data(status.pixel_index) = {
            pixel[0],
            pixel[1],
            pixel[2],
            pixel[3],
        };

        uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
        status.pixels_seen[index] = pixel;
        status.previous_pixel = pixel;
        status.pixel_index += 1;
    }

    template <typename T>
    void read_run_operation(uint8_t symbol, qoi_status<T> &status, matrix<T> &data) {
        uint8_t length = (symbol & 0b00111111) + 1;

        for (uint32_t index = 0; index < length; index++) {
            data(status.pixel_index + index) = {
                status.previous_pixel[0],
                status.previous_pixel[1],
                status.previous_pixel[2],
                status.previous_pixel[3],
            };
        }

        status.pixel_index += length;
    }

    template <typename T>
    matrix<T> read_image(const std::string &filename) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return matrix<T>();
        }

        qoi_header header = read_header(is);
        matrix<T> data(header.height, header.width);
        qoi_status<T> status;

        uint8_t symbol = 0;
        while (is.read(reinterpret_cast<char *>(&symbol), 1) && status.pixel_index < header.width * header.height) {
            uint8_t tag = (symbol >> 6) & 0b11;
            if (symbol == 0xfe) {
                read_rgb_operation(is, status, data);
            } else if (symbol == 0xff) {
                read_rgba_operation(is, status, data);
            } else if (tag == 0x0) {
                read_index_operation(symbol, status, data);
            } else if (tag == 0x1) {
                read_diff_operation(symbol, status, data);
            } else if (tag == 0x2) {
                read_luma_operation(is, symbol, status, data);
            } else if (tag == 0x3) {
                read_run_operation(symbol, status, data);
            }
        }

        return data;
    }

    template <typename T>
    class image {
        matrix<T> data_;

      public:
        image(uint32_t width, uint32_t height) : data_(height, width) {}

        explicit image(const std::string &filename) : data_(read_image<T>(filename)) {}

        auto &data() {
            return data_;
        }

        bool empty() const {
            return data_.empty();
        }
    };
}  // namespace qoi
