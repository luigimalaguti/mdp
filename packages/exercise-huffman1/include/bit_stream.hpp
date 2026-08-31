#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <type_traits>

namespace bit_stream {
    inline int64_t sign_extend(uint64_t raw, size_t bits) {
        uint64_t sign_bit = uint64_t{1} << (bits - 1);
        return static_cast<int64_t>((raw ^ sign_bit) - sign_bit);
    }

    class bit_writer {
        std::ostream &os_;
        uint8_t buffer_ = 0;
        size_t pending_ = 0;

        void write_bit(uint8_t bit) {
            buffer_ = (buffer_ << 1) | (bit & 0x1);
            pending_ += 1;
            if (pending_ == 8) {
                os_.put(buffer_);
                buffer_ = 0;
                pending_ = 0;
            }
        }

      public:
        explicit bit_writer(std::ostream &os) : os_(os) {}

        ~bit_writer() {
            flush();
        }

        explicit operator bool() const {
            return bool(os_);
        }

        bit_writer &write(uint64_t value, size_t bits) {
            while (bits > 0) {
                uint8_t bit = (value >> (bits - 1)) & 0x1;
                write_bit(bit);
                bits -= 1;
            }
            return *this;
        }

        void flush() {
            if (pending_ > 0) {
                buffer_ = buffer_ << (8 - pending_);
                os_.put(buffer_);
                buffer_ = 0;
                pending_ = 0;
            }
        }
    };

    class bit_reader {
        std::istream &is_;
        uint8_t buffer_ = 0;
        size_t pending_ = 0;

        uint64_t read_bit() {
            if (pending_ == 0) {
                buffer_ = is_.get();
                pending_ = 8;
            }
            pending_ -= 1;
            return (buffer_ >> pending_) & 0x1;
        }

        template <typename T = uint64_t>
        T optional_signed_cast(uint64_t value, size_t bits) {
            if constexpr (std::is_signed_v<T>) {
                return static_cast<T>(sign_extend(value, bits));
            }
            return static_cast<T>(value);
        }

      public:
        explicit bit_reader(std::istream &is) : is_(is) {}

        explicit operator bool() const {
            return bool(is_);
        }

        template <typename T = uint64_t>
        T read(size_t bits) {
            uint64_t value = 0;
            size_t remaining = bits;
            while (remaining > 0) {
                value = (value << 1) | read_bit();
                remaining -= 1;
            }
            return optional_signed_cast<T>(value, bits);
        }

        template <typename T>
        bit_reader &read(T &value, size_t bits) {
            value = read<T>(bits);
            return *this;
        }
    };
}  // namespace bit_stream
