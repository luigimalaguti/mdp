#include "bit_stream.hpp"
#include "huffman.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <print>
#include <string>

int compression(const std::string &input_filename, const std::string &output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::istreambuf_iterator<char> start(is), end;
    auto frequencies = std::for_each(start, end, huffman::frequency_table<uint8_t>());
    huffman::canonical_encoder<uint8_t> encoder(frequencies);

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    const std::string magic_number = "HUFFMAN2";
    os.write(magic_number.data(), 8);
    uint8_t table_entries = encoder.size();
    os.write(reinterpret_cast<const char *>(&table_entries), 1);

    bit_stream::bit_writer writer(os);
    for (const auto &[symbol, length] : encoder.table()) {
        writer.write(symbol, 8).write(length, 5);
    }

    uint32_t number_symbols = frequencies.total();
    writer.write(number_symbols, 32);

    is.clear();
    is.seekg(0, std::ios::beg);
    for (size_t index = 0; index < number_symbols; index++) {
        uint8_t symbol = is.get();
        const auto &[length, code] = encoder[symbol];
        writer.write(code, length);
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
    if (magic_number != "HUFFMAN2") {
        std::println("Error: Invalid magic number '{}'", magic_number);
        return 1;
    }

    uint8_t table_entries = 0;
    is.read(reinterpret_cast<char *>(&table_entries), 1);

    bit_stream::bit_reader reader(is);
    huffman::length_table<uint8_t> table;
    for (size_t index = 0; index < table_entries; index++) {
        uint8_t symbol = reader.read(8);
        uint8_t length = reader.read(5);
        table(symbol, length);
    }

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    huffman::canonical_decoder<uint8_t> decoder(table);
    uint32_t number_symbols = reader.read(32);
    for (size_t number = 0; number < number_symbols; number++) {
        uint32_t bits = 0, length = 0;
        int32_t index = -1;
        while (index == -1) {
            bits = (bits << 1) | reader.read<uint8_t>(1);
            length += 1;
            index = decoder.find({length, bits});
        }
        uint8_t symbol = decoder[index].symbol;
        os.put(symbol);
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
