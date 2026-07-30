#include <fstream>
#include <print>
#include <vector>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <input_file> <output_file>", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::vector<size_t> frequencies(256);

    char character;
    while (is.get(character)) {
        uint8_t byte = static_cast<uint8_t>(character);
        frequencies[byte]++;
    }

    std::ofstream os(output_filename);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }
    for (size_t index = 0; index < frequencies.size(); index++) {
        std::println(os, "{:02x}\t{}", index, frequencies[index]);
    }

    return 0;
}
