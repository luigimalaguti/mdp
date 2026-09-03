#include "pam.hpp"

#include <cstdint>
#include <filesystem>
#include <print>
#include <string>
#include <vector>

struct placement {
    std::string filename;
    uint32_t x = 0;
    uint32_t y = 0;
};

std::vector<placement> parse_placements(int argc, char **argv) {
    std::vector<placement> placements;

    int index = 2;
    while (index < argc) {
        uint32_t x = 0, y = 0;
        if (std::string(argv[index]) == "-p") {
            x = std::stoul(argv[index + 1]);
            y = std::stoul(argv[index + 2]);
            index += 3;
        }

        std::string input_filename = argv[index];
        std::filesystem::path input_path(input_filename);
        if (input_path.has_extension()) {
            std::println("Error: Filename {} should not have an extension", input_filename);
            return std::vector<placement>();
        }
        input_filename = (input_path.parent_path() / input_path.stem()).string() + ".pam";

        placements.push_back({input_filename, x, y});
        index += 1;
    }

    return placements;
}

bool compose_file(pam::image<pam::rgba<uint8_t>> &destination, const std::string &filename, uint32_t x, uint32_t y) {
    pam::header info = pam::read_header(filename);
    if (info.depth == pam::pixel_type<pam::rgb<uint8_t>>::depth) {
        pam::image<pam::rgb<uint8_t>> source(filename);
        if (source.empty()) {
            return false;
        }
        destination.compose(source, x, y);
        return true;
    }
    if (info.depth == pam::pixel_type<pam::rgba<uint8_t>>::depth) {
        pam::image<pam::rgba<uint8_t>> source(filename);
        if (source.empty()) {
            return false;
        }
        destination.compose(source, x, y);
        return true;
    }
    return false;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::println(
            "Usage: {} "
            "<output_filename> "
            "[-p <x1> <y1>] <input_filename1> "
            "[ [-p <x2> <y2>] <input_filename2> ... ]",
            argv[0]);
        return 1;
    }

    std::string output_filename = argv[1];
    std::filesystem::path output_path(output_filename);
    if (output_path.has_extension()) {
        std::println("Error: Filename {} should not have an extension", output_filename);
        return 1;
    }
    output_filename = (output_path.parent_path() / output_path.stem()).string() + ".pam";
    const std::vector<placement> placements = parse_placements(argc, argv);
    if (placements.empty()) {
        return 1;
    }

    uint32_t width = 0, height = 0;
    for (const auto &placement : placements) {
        pam::header header = pam::read_header(placement.filename);
        if (header.width == 0 || header.height == 0) {
            std::println("Error: Could not open file {}", placement.filename);
            return 1;
        }
        width = std::max(width, placement.x + header.width);
        height = std::max(height, placement.y + header.height);
    }

    pam::image<pam::rgba<uint8_t>> canvas(width, height);
    for (const auto &placement : placements) {
        if (!compose_file(canvas, placement.filename, placement.x, placement.y)) {
            std::println("Error: Could not read file {}", placement.filename);
            return 1;
        }
    }

    if (!canvas.save(output_filename)) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    return 0;
}
