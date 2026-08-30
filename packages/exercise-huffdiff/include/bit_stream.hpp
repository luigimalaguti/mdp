#pragma once

#include <cstdint>
#include <istream>
#include <ostream>

namespace bit_stream {
    class bit_writer {
        std::ostream &os_;
        uint8_t buffer_ = 0;
        size_t pending_ = 0;

        void write_bit(uint8_t bit) {
            buffer_ = (buffer_ << 1) | bit;
            pending_ += 1;
            if (pending_ == 8) {
                os_.put(buffer_);
                buffer_ = 0;
                pending_ = 0;
            }
        }

        void write_byte(uint8_t byte) {
            if (pending_ > 0) {
                buffer_ = (buffer_ << (8 - pending_)) | (byte >> pending_);
                os_.put(buffer_);
                buffer_ = byte & ((0x1 << pending_) - 0x1);
            } else {
                os_.put(byte);
            }
        }

      public:
        bit_writer(std::ostream &os) : os_(os) {}

        ~bit_writer() {
            flush();
        }

        explicit operator bool() const {
            return bool(os_);
        }

        bit_writer &write(const uint64_t &value, size_t bits) {
            while (bits > 0) {
                if (bits >= 8) {
                    uint8_t byte = (value >> (bits - 8)) & 0xff;
                    write_byte(byte);
                    bits -= 8;
                } else {
                    uint8_t bit = (value >> (bits - 1)) & 0x1;
                    write_bit(bit);
                    bits -= 1;
                }
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

        uint8_t read_bit() {
            if (pending_ == 0) {
                buffer_ = is_.get();
                pending_ = 8;
            }
            pending_ -= 1;
            return (buffer_ >> pending_) & 0x1;
        }

        uint8_t read_byte() {
            uint8_t byte = 0;
            if (pending_ > 0) {
                byte = buffer_ << (8 - pending_);
                buffer_ = is_.get();
                byte = byte | (buffer_ >> pending_);
            } else {
                byte = is_.get();
            }
            return byte;
        }

      public:
        bit_reader(std::istream &is) : is_(is) {}

        explicit operator bool() const {
            return bool(is_);
        }

        bit_reader &read(uint64_t &value, size_t bits) {
            value = 0;
            while (bits > 0) {
                if (bits >= 8) {
                    value = (value << 8) | read_byte();
                    bits -= 8;
                } else {
                    value = (value << 1) | read_bit();
                    bits -= 1;
                }
            }
            return *this;
        }

        auto read(size_t bits) {
            uint64_t value;
            read(value, bits);
            return value;
        }
    };
};  // namespace bit_stream
