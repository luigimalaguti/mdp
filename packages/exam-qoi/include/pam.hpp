#pragma once

#include "containers.hpp"

#include <fstream>
#include <string>
#include <utility>

namespace pam {
    template <typename T>
    using matrix = containers::matrix<T>;

    template <typename T>
    using rgba = containers::rgba<T>;

    template <typename T>
    bool write_image(const std::string &filename, const matrix<T> &data) {
        std::ofstream os(filename, std::ios::binary);
        if (!os) {
            return false;
        }

        os << "P7\n";
        os << "WIDTH " << data.cols() << "\n";
        os << "HEIGHT " << data.rows() << "\n";
        os << "DEPTH 4\n";
        os << "MAXVAL 255\n";
        os << "TUPLTYPE RGB_ALPHA\n";
        os << "ENDHDR" << "\n";
        os.write(data.raw_data(), static_cast<int64_t>(data.raw_size()));

        return true;
    }

    template <typename T>
    class image {
        matrix<T> data_;

      public:
        image(matrix<T> data) : data_(std::move(data)) {}

        bool save(const std::string &filename) const {
            return write_image(filename, data_);
        }
    };
}  // namespace pam
