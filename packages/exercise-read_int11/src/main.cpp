#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <print>
#include <vector>

class bit_reader {
    std::istream &is_;
    size_t bits_;
    uint8_t buffer_;
    size_t pending_;

    uint8_t read_bit() {
        if (pending_ == 0) {
            is_.get(reinterpret_cast<char &>(buffer_));
            pending_ = 8;
        }
        pending_ -= 1;
        return (buffer_ >> pending_) & 0x1;
    }

    uint8_t read_byte() {
        if (pending_ > 0) {
            uint8_t byte = buffer_ << (8 - pending_);
            is_.get(reinterpret_cast<char &>(buffer_));
            byte = byte | (buffer_ >> pending_);
            return byte;
        } else {
            is_.get(reinterpret_cast<char &>(buffer_));
            return buffer_;
        }
    }

public:
    bit_reader(std::istream &is, size_t bits = 8) : is_(is), bits_(bits), buffer_(0), pending_(0) {}

    std::istream &operator()(uint64_t &value) {
        return read(value);
    }

    std::istream &read(uint64_t &value) {
        value = 0;
        size_t bit = bits_;
        while (bit > 0) {
            if (bit >= 8) {
                value = (value << 8) | read_byte();
                bit -= 8;
            } else {
                value = (value << 1) | read_bit();
                bit -= 1;
            }
        }
        return is_;
    }

    void bits(size_t bits) {
        bits_ = bits;
    }
};

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <filein.bin> <fileout.txt>", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::vector<int16_t> vec;

    size_t bits = 11;
    bit_reader reader(is, bits);
    uint64_t number;
    while (reader(number)) {
        int16_t value = static_cast<int16_t>(number);
        if (number & (0x1 << (bits - 1))) {
            value = static_cast<int16_t>(value - (0x1 << bits));
        }
        vec.push_back(value);
    }

    std::ofstream os(output_filename);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    std::ostream_iterator<int16_t> start(os, "\n");
    std::copy(vec.begin(), vec.end(), start);

    return 0;
}
