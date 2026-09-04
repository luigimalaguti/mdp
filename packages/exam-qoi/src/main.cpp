#include "pam.hpp"
#include "qoi.hpp"

#include <cstdint>
#include <print>
#include <string>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <input file> <output file>", argv[0]);
        return 1;
    }

    const std::string input_filename = argv[1];
    const std::string output_filename = argv[2];

    qoi::image<uint8_t> qoi_image(input_filename);
    if (qoi_image.empty()) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    pam::image<uint8_t> pam_image(qoi_image.data());
    if (!pam_image.save(output_filename)) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    return 0;
}
