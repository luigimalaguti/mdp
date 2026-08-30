#include "bit_stream.hpp"
#include "huffman.hpp"
#include "pam_image.hpp"

#include <fstream>
#include <print>
#include <string>

int compression(const std::string &input_filename, const std::string &output_filename) {
    pam_image::matrix data = pam_image::read_image(input_filename);
    if (data.empty()) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    pam_image::matrix difference = compute_difference<int16_t, uint8_t>(data);
    auto frequency_table = std::for_each(difference.begin(), difference.end(), huffman::frequency_table<int16_t>());
    huffman::canonical_encoder<int16_t> encoder(frequency_table);

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

    bit_stream::bit_writer writer(os);
    uint64_t number_symbols = encoder.size();
    writer.write(number_symbols, 9);

    auto codes = encoder.to_vector();
    for (const auto &[symbol, length] : codes) {
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
    using namespace pam_image;
    using namespace huffman;

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
    uint64_t number_symbols;
    reader.read(number_symbols, 9);

    length_table<int16_t> lengths;
    for (uint16_t index = 0; index < number_symbols; index++) {
        uint64_t symbol;
        uint64_t length;
        reader.read(symbol, 9).read(length, 5);
        lengths[symbol] = length;
    }

    matrix<int16_t> difference(width, height);
    canonical_decoder<int16_t> decoder(lengths);
    for (uint32_t row = 0; row < difference.rows(); row++) {
        for (uint32_t col = 0; col < difference.cols(); col++) {
            bool decoded = false;
            uint32_t current_bits = 0;
            uint8_t current_length = 0;
            while (!decoded) {
                uint64_t bit = reader.read(1);
                current_bits = (current_bits << 1) | bit;
                current_length++;

                int32_t index = decoder.find({current_length, current_bits});
                if (index != -1) {
                    difference[row, col] = decoder[index].symbol;
                    decoded = true;
                }
            }
        }
    }

    matrix data = restore_difference<uint8_t, int16_t>(difference);
    if (!write_image(output_filename, data)) {
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
