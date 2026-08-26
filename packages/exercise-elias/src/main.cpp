#include <algorithm>
#include <bit>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <vector>

class bit_writer {
    std::ostream &os_;
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
    bit_writer(std::ostream &os) : os_(os), buffer_(0), pending_(0) {}

    ~bit_writer() {
        flush();
    }

    std::ostream &operator()(const uint64_t value, size_t bits) {
        return write(value, bits);
    }

    std::ostream &write(const uint64_t value, size_t bits) {
        while (bits > 0) {
            if (bits >= 8) {
                uint8_t current = (value >> (bits - 8)) & 0xff;
                write_byte(current);
                bits -= 8;
            } else {
                uint8_t current = (value >> (bits - 1)) & 0x1;
                write_bit(current);
                bits -= 1;
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
};

class bit_reader {
    std::istream &is_;
    uint8_t buffer_;
    size_t pending_;

    uint8_t read_bit() {
        if (pending_ == 0) {
            buffer_ = is_.get();
            pending_ = 8;
        }
        pending_ -= 1;
        return (buffer_ >> pending_) & 0x1;
    }

    uint8_t read_byte() {
        if (pending_ > 0) {
            uint8_t byte = buffer_ << (8 - pending_);
            buffer_ = is_.get();
            byte = byte | (buffer_ >> pending_);
            return byte;
        } else {
            buffer_ = is_.get();
            return buffer_;
        }
    }

public:
    bit_reader(std::istream &is) : is_(is), buffer_(0), pending_(0) {}

    std::istream &operator()(uint64_t &value, size_t bits) {
        return read(value, bits);
    }

    std::istream &read(uint64_t &value, size_t bits) {
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
        return is_;
    }
};

uint64_t map(int64_t value) {
    return (value < 0) ? (-2 * value) : (1 + 2 * value);
}

int64_t unmap(uint64_t value) {
    return static_cast<int64_t>((value % 2 == 0) ? -(value / 2) : ((value - 1) / 2));
}

int compression(const std::string &input_filename, const std::string &output_filename) {
    std::ifstream is(input_filename);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::istream_iterator<int64_t> start(is);
    std::istream_iterator<int64_t> stop;
    std::vector vec(start, stop);

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    bit_writer writer(os);
    for (const auto &value : vec) {
        uint64_t mapped = map(value);
        size_t length = std::bit_width(mapped);
        size_t zeros = length - 1;
        writer(0, zeros);
        writer(mapped, length);
    }

    return 0;
}

int decompression(const std::string &input_filename, const std::string &output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::vector<int64_t> vec;

    bit_reader reader(is);
    uint64_t bit;
    size_t length = 0;
    while (reader(bit, 1)) {
        if (bit != 0) {
            uint64_t mapped;
            if (!reader(mapped, length)) {
                return 1;
            }
            mapped = (0x1ULL << length) | mapped;
            int64_t number = unmap(mapped);
            vec.push_back(number);
            length = 0;
        } else {
            length += 1;
        }
    }

    std::ofstream os(output_filename);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    std::ostream_iterator<int64_t> start(os, "\n");
    std::copy(vec.begin(), vec.end(), start);

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        std::println("Usage: {} [c|d] <filein> <fileout>", argv[0]);
        return 1;
    }

    const std::string elias_mode = argv[1];
    const std::string input_filename = argv[2];
    const std::string output_filename = argv[3];

    if (elias_mode == "c") {
        return compression(input_filename, output_filename);
    } else if (elias_mode == "d") {
        return decompression(input_filename, output_filename);
    } else {
        std::println("Error: Invalid mode '{}'. Use 'c' for compression or 'd' for decompression.", elias_mode);
        return 1;
    }
}
