#include "bit_stream.hpp"
#include "huffman.hpp"
#include "pam.hpp"

#include <filesystem>
#include <fstream>
#include <print>
#include <string>
#include <utility>

int compression(const std::string &input_filename, const std::string &output_filename) {
    pam::image<uint8_t> image(input_filename);
    if (image.empty()) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    pam::image<int16_t> difference = image.difference<int16_t>();
    std::filesystem::path path(input_filename);
    std::string difference_filename = (path.parent_path() / path.stem()).string() + "_difference.pam";
    difference.viewable().save(difference_filename);

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    const std::string magic_number = "HUFFDIFF";
    os.write(magic_number.data(), 8);
    uint32_t width = difference.width();
    uint32_t height = difference.height();
    os.write(reinterpret_cast<const char *>(&width), 4);
    os.write(reinterpret_cast<const char *>(&height), 4);

    auto frequencies = std::for_each(difference.begin(), difference.end(), huffman::frequency_table<int16_t>());
    huffman::canonical_encoder<int16_t> encoder(frequencies);

    bit_stream::bit_writer writer(os);
    writer.write(encoder.size(), 9);
    for (const auto &[symbol, length] : encoder.table()) {
        writer.write(symbol, 9).write(length, 5);
    }

    for (uint32_t row = 0; row < difference.rows(); row++) {
        for (uint32_t col = 0; col < difference.cols(); col++) {
            int16_t value = difference[row, col];
            const auto &[length, bits] = encoder[value];
            writer.write(bits, length);
        }
    }

    return 0;
}

int decompression(const std::string &input_filename, const std::string &output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::string magic_number(8, '\0');
    is.read(magic_number.data(), 8);
    if (magic_number != "HUFFDIFF") {
        std::println("Error: Invalid magic number in file {}", input_filename);
        return 1;
    }

    uint32_t width, height;
    is.read(reinterpret_cast<char *>(&width), 4);
    is.read(reinterpret_cast<char *>(&height), 4);

    bit_stream::bit_reader reader(is);
    uint16_t number_symbols = reader.read(9);

    huffman::length_table<int16_t> lengths;
    for (uint16_t index = 0; index < number_symbols; index++) {
        int16_t symbol = reader.read<int16_t>(9);
        uint8_t length = reader.read(5);
        lengths(symbol, length);
    }
    huffman::canonical_decoder<int16_t> decoder(lengths);

    containers::matrix<int16_t> raw_difference(height, width);
    for (uint32_t row = 0; row < raw_difference.rows(); row++) {
        for (uint32_t col = 0; col < raw_difference.cols(); col++) {
            uint32_t bits = 0, length = 0;
            int32_t index = -1;
            while (index == -1) {
                bits = (bits << 1) | reader.read<uint8_t>(1);
                length += 1;
                index = decoder.find({length, bits});
            }
            raw_difference[row, col] = decoder[index].symbol;
        }
    }

    pam::image<int16_t> difference(std::move(raw_difference));
    pam::image data = difference.restore<uint8_t>();
    if (!data.save(output_filename)) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        std::println("Usage: {} [c|d] <input file> <output file>", argv[0]);
        return 1;
    }

    const std::string operation = argv[1];
    const std::string input_filename = argv[2];
    const std::string output_filename = argv[3];

    if (operation == "c") {
        return compression(input_filename, output_filename);
    } else if (operation == "d") {
        return decompression(input_filename, output_filename);
    } else {
        std::println("Error: Invalid operation '{}'. Use 'c' for compression or 'd' for decompression", operation);
        return 1;
    }
}
