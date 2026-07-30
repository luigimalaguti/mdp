#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <print>
#include <vector>

template <typename T>
std::istream &read_host_endian(std::ifstream &is, T &number, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char *>(&number), static_cast<unsigned int>(size));
}

template <typename T>
std::istream &read_little_endian(std::ifstream &is, T &number, size_t size = sizeof(T)) {
    T bytes = 0;
    for (size_t i = 0; i < size; i++) {
        char byte;
        if (!is.get(byte)) {
            return is;
        }
        bytes |= (static_cast<T>(static_cast<uint8_t>(byte)) << (i * 8));
    }
    number = static_cast<T>(bytes);
    return is;
}

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

    std::vector<int32_t> vec;

    int32_t number;
    while (read_little_endian(is, number)) {
        vec.push_back(number);
    }

    std::ofstream os(output_filename);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    std::ostream_iterator<int32_t> start(os, "\n");
    std::copy(vec.begin(), vec.end(), start);

    return 0;
}
