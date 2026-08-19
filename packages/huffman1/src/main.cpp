#include <fstream>
#include <print>
#include <string>

int compression(const std::string &input_filename, const std::string output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    return 0;
}

int decompression(const std::string &input_filename, const std::string output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::ofstream os(output_filename);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (argc != 4) {
        std::println("Usage: {} [c|d] <input file> <output file>", argv[0]);
        return 1;
    }

    const std::string huffman_mode = argv[1];
    const std::string input_filename = argv[2];
    const std::string output_filename = argv[3];

    if (huffman_mode == "c") {
        return compression(input_filename, output_filename);
    } else if (huffman_mode == "d") {
        return decompression(input_filename, output_filename);
    } else {
        std::println("Error: Invalid mode '{}'. Use 'c' for compression or 'd' for decompression", huffman_mode);
        return 1;
    }
}
