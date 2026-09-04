#pragma once

#include "containers.hpp"

#include <cstdint>
#include <fstream>
#include <numeric>
#include <string>

namespace qoi {
    template <typename T>
    using matrix = containers::matrix<T>;

    template <typename T>
    using rgba = containers::rgba<T>;

    uint8_t index_position(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return (r * 3 + g * 5 + b * 7 + a * 11) % 64;
    }

    void read_header(std::istream &is, uint32_t &width, uint32_t &height) {
        std::string magic_number(4, '\0');
        is.read(magic_number.data(), 4);
        uint32_t raw_width = 0, raw_height = 0;
        for (size_t index = 0; index < 4; index++) {
            is.read(reinterpret_cast<char *>(&raw_width), 1);
            width = (width << 8) | raw_width;
        }
        for (size_t index = 0; index < 4; index++) {
            is.read(reinterpret_cast<char *>(&raw_height), 1);
            height = (height << 8) | raw_height;
        }
        uint8_t channel = 0, color_space;
        is.read(reinterpret_cast<char *>(&channel), 1);
        is.read(reinterpret_cast<char *>(&color_space), 1);
    }

    template <typename T>
    matrix<T> read_image(const std::string &filename) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return matrix<T>();
        }
        uint32_t width = 0, height = 0;
        read_header(is, width, height);

        uint32_t position = 0;
        matrix<T> data(height, width);

        rgba<T> previous_pixel = {0, 0, 0, 255};
        std::vector<rgba<T>> pixels_seen(64);
        std::vector<uint8_t> previous_symbols(8);

        uint8_t symbol = 0;
        while (is.read(reinterpret_cast<char *>(&symbol), 1)) {
            if (std::accumulate(previous_symbols.begin(), previous_symbols.end(), 0) == 1) {
                break;
            }

            uint8_t tag = (symbol >> 6) & 0b11;
            if (tag == 0x0) {
                uint8_t index = symbol & 0b00111111;

                data(position) = {
                    pixels_seen[index][0],
                    pixels_seen[index][1],
                    pixels_seen[index][2],
                    pixels_seen[index][3],
                };

                previous_pixel = pixels_seen[index];
                position += 1;
            } else if (tag == 0x1) {
                int8_t diff_r = static_cast<int8_t>(((symbol & 0b00110000) >> 4) - 2);
                int8_t diff_g = static_cast<int8_t>(((symbol & 0b00001100) >> 2) - 2);
                int8_t diff_b = static_cast<int8_t>(((symbol & 0b00000011) >> 0) - 2);

                rgba<T> pixel = {
                    previous_pixel[0],
                    previous_pixel[1],
                    previous_pixel[2],
                    previous_pixel[3],
                };
                pixel[0] = static_cast<uint8_t>(pixel[0] + diff_r);
                pixel[1] = static_cast<uint8_t>(pixel[1] + diff_g);
                pixel[2] = static_cast<uint8_t>(pixel[2] + diff_b);
                data(position) = {
                    pixel[0],
                    pixel[1],
                    pixel[2],
                    pixel[3],
                };

                uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
                pixels_seen[index] = pixel;
                previous_pixel = pixel;
                position += 1;
            } else if (tag == 0x2) {
                int8_t diff_g = static_cast<int8_t>(((symbol & 0b00111111) >> 0) - 32);
                symbol = is.get();
                int8_t diff_r_g = static_cast<int8_t>(((symbol & 0b11110000) >> 4) - 8);
                int8_t diff_b_g = static_cast<int8_t>(((symbol & 0b00001111) >> 0) - 8);

                rgba<T> pixel = {
                    previous_pixel[0],
                    previous_pixel[1],
                    previous_pixel[2],
                    previous_pixel[3],
                };
                pixel[0] = static_cast<uint8_t>(pixel[0] + diff_r_g + diff_g);
                pixel[1] = static_cast<uint8_t>(pixel[1] + diff_g);
                pixel[2] = static_cast<uint8_t>(pixel[2] + diff_b_g + diff_g);
                data(position) = {
                    pixel[0],
                    pixel[1],
                    pixel[2],
                    pixel[3],
                };

                uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
                pixels_seen[index] = pixel;
                previous_pixel = pixel;
                position += 1;
            } else if (tag == 0x3) {
                uint8_t length = (symbol & 0b00111111) + 1;

                for (uint32_t index = 0; index < length; index++) {
                    data(position + index) = {
                        previous_pixel[0],
                        previous_pixel[1],
                        previous_pixel[2],
                        previous_pixel[3],
                    };
                }

                position += length;
            } else if (symbol == 0xfe) {
                rgba<T> pixel = {
                    static_cast<T>(is.get()),
                    static_cast<T>(is.get()),
                    static_cast<T>(is.get()),
                    previous_pixel[3],
                };

                data(position) = {
                    pixel[0],
                    pixel[1],
                    pixel[2],
                    pixel[3],
                };

                uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
                pixels_seen[index] = pixel;
                previous_pixel = pixel;
                position += 1;
            } else if (symbol == 0xff) {
                rgba<T> pixel = {
                    static_cast<T>(is.get()),
                    static_cast<T>(is.get()),
                    static_cast<T>(is.get()),
                    static_cast<T>(is.get()),
                };

                data(position) = {
                    pixel[0],
                    pixel[1],
                    pixel[2],
                    pixel[3],
                };

                uint8_t index = index_position(pixel[0], pixel[1], pixel[2], pixel[3]);
                pixels_seen[index] = pixel;
                previous_pixel = pixel;
                position += 1;
            }
            previous_symbols[position % 8] = symbol;
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
