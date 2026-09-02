#include "pam.hpp"
#include "tiff.hpp"

#include <print>
#include <string>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <input file .TIFF> <output file .PAM>", argv[1]);
        return 1;
    }

    const std::string input_filename = argv[1];
    const std::string output_filename = argv[2];

    tiff::image tiff_image(input_filename);
    if (tiff_image.empty()) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    pam::image pam_image(tiff_image.data());
    if (!pam_image.save(output_filename)) {
        std::println("Error: Could not save file {}", output_filename);
        return 1;
    }

    return 0;
}
