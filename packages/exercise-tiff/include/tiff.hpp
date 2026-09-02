#pragma once

#include "containers.hpp"

#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tiff {
    template <typename T>
    using matrix = containers::matrix<T>;

    struct ifde {
        uint16_t tag_ = 0;
        uint16_t type_ = 0;
        uint32_t count_ = 0;
        uint32_t value_offset_ = 0;

        ifde(std::istream &is, uint32_t offset) {
            is.seekg(offset, std::ios::beg);
            is.read(reinterpret_cast<char *>(&tag_), sizeof(tag_));
            is.read(reinterpret_cast<char *>(&type_), sizeof(type_));
            is.read(reinterpret_cast<char *>(&count_), sizeof(count_));
            is.read(reinterpret_cast<char *>(&value_offset_), sizeof(value_offset_));
        }
    };

    struct ifd {
        uint16_t number_entries_ = 0;
        std::unordered_map<uint16_t, ifde> entries_;
        uint32_t next_ifd_offset_ = 0;

        ifd(std::istream &is, uint32_t offset) {
            is.seekg(offset, std::ios::beg);
            is.read(reinterpret_cast<char *>(&number_entries_), sizeof(number_entries_));
            for (uint16_t index = 0; index < number_entries_; ++index) {
                ifde entry(is, offset + 2 + index * 12);
                entries_.emplace(entry.tag_, entry);
            }
            is.read(reinterpret_cast<char *>(&next_ifd_offset_), sizeof(next_ifd_offset_));
        }

        const auto &operator[](uint16_t tag) const {
            return entries_.at(tag).value_offset_;
        }
    };

    struct ifh {
        uint16_t byte_order_ = 0;
        uint16_t identifier_ = 0;
        uint32_t offset_ = 0;
        std::vector<ifd> directories_;

        ifh(std::istream &is) {
            is.seekg(0, std::ios::beg);
            is.read(reinterpret_cast<char *>(&byte_order_), sizeof(byte_order_));
            is.read(reinterpret_cast<char *>(&identifier_), sizeof(identifier_));
            is.read(reinterpret_cast<char *>(&offset_), sizeof(offset_));
            do {
                directories_.emplace_back(is, offset_);
                offset_ = directories_.back().next_ifd_offset_;
            } while (offset_ != 0);
        }
    };

    enum class ifde_tag : uint16_t {
        ImageWidth = 256,
        ImageLength = 257,
        BitsPerSample = 258,
        Compression = 259,
        PhotometricInterpretation = 262,
        StripOffsets = 273,
        RowsPerStrip = 278,
        StripByteCounts = 279,
    };

    template <typename T = uint8_t>
    matrix<T> read_image(const std::string &filename) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return matrix<T>();
        }

        ifh header(is);
        if (header.byte_order_ != 0x4949 || header.identifier_ != 42 || header.directories_.size() > 1) {
            return matrix<T>();
        }
        ifd &directory = header.directories_.front();
        uint32_t width = directory[static_cast<uint16_t>(ifde_tag::ImageWidth)];
        uint32_t height = directory[static_cast<uint16_t>(ifde_tag::ImageLength)];
        if (directory[static_cast<uint16_t>(ifde_tag::BitsPerSample)] != 8 ||
            directory[static_cast<uint16_t>(ifde_tag::Compression)] != 1 ||
            directory[static_cast<uint16_t>(ifde_tag::PhotometricInterpretation)] != 1 ||
            directory[static_cast<uint16_t>(ifde_tag::RowsPerStrip)] != height ||
            directory[static_cast<uint16_t>(ifde_tag::StripByteCounts)] != width * height) {
            return matrix<T>();
        }

        uint32_t strip_offset = directory[static_cast<uint16_t>(ifde_tag::StripOffsets)];
        is.seekg(strip_offset, std::ios::beg);
        matrix<T> data(height, width);
        is.read(data.raw_data(), width * height);
        return data;
    }

    template <typename T = uint8_t>
    class image {
        matrix<T> data_;

      public:
        image(uint32_t width, uint32_t height) : data_(height, width) {}

        explicit image(const std::string &filename) : data_(read_image<T>(filename)) {}

        explicit image(const matrix<T> &data) : data_(std::move(data)) {}

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
    };
}  // namespace tiff
