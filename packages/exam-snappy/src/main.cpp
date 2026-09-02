#include <cstdint>
#include <fstream>
#include <print>
#include <string>
#include <vector>

uint64_t read_preamble(std::istream &is) {
    uint8_t read_more = 1;
    uint8_t count = 0;
    uint64_t preamble = 0;
    do {
        uint8_t byte = is.get();
        read_more = byte & 0b10000000;
        preamble |= static_cast<uint64_t>(byte & 0b01111111) << (7 * count);
        count += 1;
    } while (read_more != 0);
    return preamble;
}

bool read_literal(std::istream &is, uint8_t element, std::vector<uint8_t> &decoded) {
    uint32_t length = element >> 2;
    if (length >= 60) {
        uint32_t extra_bytes = length - 59;
        length = 0;
        for (size_t index = 0; index < extra_bytes; index++) {
            length |= static_cast<uint32_t>(is.get()) << (8 * index);
        }
    }
    length += 1;
    for (size_t index = 0; index < length; index++) {
        decoded.push_back(is.get());
    }
    return true;
}

bool read_copy(std::istream &is, uint8_t element, std::vector<uint8_t> &decoded) {
    uint8_t tag = element & 0b11;
    uint8_t length = 0;
    uint32_t offset = 0;

    if (tag == 1) {
        length = ((element >> 2) & 0b111) + 4;
        offset = ((element & 0b11100000) << 3) | is.get();
    } else if (tag == 2) {
        length = (element >> 2) + 1;
        for (size_t index = 0; index < 2; index++) {
            offset |= (static_cast<uint32_t>(is.get()) << (8 * index));
        }
    } else if (tag == 3) {
        length = (element >> 2) + 1;
        for (size_t index = 0; index < 4; index++) {
            offset |= (static_cast<uint32_t>(is.get()) << (8 * index));
        }
    } else {
        return false;
    }

    size_t start_position = decoded.size() - offset;
    for (size_t index = 0; index < length; index++) {
        decoded.push_back(decoded[start_position + index]);
    }

    return true;
}

int decompression(const std::string &input_filename, const std::string &output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    read_preamble(is);

    std::vector<uint8_t> decoded;
    uint8_t symbol = 0;
    while (is.read(reinterpret_cast<char *>(&symbol), 1)) {
        bool result = true;
        uint8_t tag = symbol & 0b11;
        if (tag == 0) {
            result = read_literal(is, symbol, decoded);
        } else {
            result = read_copy(is, symbol, decoded);
        }
        if (!result) {
            return -1;
        }
    }

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }
    os.write(reinterpret_cast<const char *>(decoded.data()), static_cast<int64_t>(decoded.size()));

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <input file> <output file>", argv[0]);
        return 1;
    }

    const std::string input_filename = argv[1];
    const std::string output_filename = argv[2];

    return decompression(input_filename, output_filename);
}
