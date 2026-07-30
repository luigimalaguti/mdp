#include <cstdint>
#include <fstream>
#include <iterator>
#include <print>
#include <vector>

template <typename T>
void write_host_endian(std::ofstream &os, const T &number, size_t size = sizeof(T)) {
    os.write(reinterpret_cast<const char *>(&number), static_cast<unsigned int>(size));
}

template <typename T>
void write_little_endian(std::ofstream &os, const T &number, size_t size = sizeof(T)) {
    for (size_t i = 0; i < size; i++) {
        uint8_t byte = static_cast<uint8_t>((number >> (i * 8)) & 0xff);
        os.put(byte);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {}  <filein.txt> <fileout.bin>", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    std::ifstream is(input_filename);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::istream_iterator<int32_t> start(is);
    std::istream_iterator<int32_t> stop;
    std::vector<int32_t> vec(start, stop);

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    for (const auto &value : vec) {
        write_little_endian(os, value);
    }

    return 0;
}
