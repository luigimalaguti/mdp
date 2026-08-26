#include <cstdint>
#include <fstream>
#include <iterator>
#include <print>
#include <vector>

class bit_writer {
    std::ostream &os_;
    size_t bits_;
    uint8_t buffer_;
    size_t pending_;

    void write_bit(uint8_t bit) {
        buffer_ = (buffer_ << 1) | bit;
        pending_ += 1;
        if (pending_ == 8) {
            os_.put(buffer_);
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
    bit_writer(std::ostream &os, size_t bits = 8) : os_(os), bits_(bits), buffer_(0), pending_(0) {}

    ~bit_writer() {
        flush();
    }

    std::ostream &operator()(const uint64_t value) {
        return write(value);
    }

    std::ostream &write(const uint64_t value) {
        size_t bit = bits_;
        while (bit > 0) {
            if (bit >= 8) {
                uint8_t current = (value >> (bit - 8)) & 0xff;
                write_byte(current);
                bit -= 8;
            } else {
                uint8_t current = (value >> (bit - 1)) & 0x1;
                write_bit(current);
                bit -= 1;
            }
        }
        return os_;
    }

    void flush() {
        if (pending_ > 0) {
            buffer_ = buffer_ << (8 - pending_);
            os_.put(buffer_);
            pending_ = 0;
        }
    }

    void bits(size_t bits) {
        bits_ = bits;
    }
};

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <filein.txt> <fileout.bin>", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    std::ifstream is(input_filename);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::istream_iterator<int16_t> start(is);
    std::istream_iterator<int16_t> stop;
    std::vector<int16_t> vec(start, stop);

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    bit_writer writer(os, 11);
    for (const auto &value : vec) {
        writer(value);
    }

    return 0;
}
