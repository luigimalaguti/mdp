#include <algorithm>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <unordered_map>
#include <vector>

namespace bit_stream {
    class bit_writer {
        std::ostream &os_;
        uint8_t buffer_ = 0;
        size_t pending_ = 0;

        void write_bit(uint8_t bit) {
            buffer_ = (buffer_ << 1) | bit;
            pending_ += 1;
            if (pending_ == 8) {
                os_.put(buffer_);
                pending_ = 0;
            }
        }

        void write_byte(uint8_t byte) {
            if (pending_ > 0) {
                buffer_ = (buffer_ << (8 - pending_)) | (byte >> pending_);
                os_.put(buffer_);
                buffer_ = byte & ((0x1 << pending_) - 0x1);
            } else {
                os_.put(byte);
            }
        }

      public:
        bit_writer(std::ostream &os) : os_(os) {}

        ~bit_writer() {
            flush();
        }

        std::ostream &write(const uint64_t &value, size_t bits) {
            while (bits > 0) {
                if (bits >= 8) {
                    uint8_t byte = (value >> (bits - 8)) & 0xff;
                    write_byte(byte);
                    bits -= 8;
                } else {
                    uint8_t bit = (value >> (bits - 1)) & 0x1;
                    write_bit(bit);
                    bits -= 1;
                }
            }
            return os_;
        }

        void flush() {
            if (pending_ > 0) {
                buffer_ = buffer_ << (8 - pending_);
                os_.put(buffer_);
                pending_ = 0;
            }
        }
    };

    class bit_reader {
        std::istream &is_;
        uint8_t buffer_ = 0;
        size_t pending_ = 0;

        uint8_t read_bit() {
            if (pending_ == 0) {
                buffer_ = is_.get();
                pending_ = 8;
            }
            pending_ -= 1;
            return (buffer_ >> pending_) & 0x1;
        }

        uint8_t read_byte() {
            uint8_t byte = 0;
            if (pending_ > 0) {
                byte = buffer_ << (8 - pending_);
                buffer_ = is_.get();
                byte = byte | (buffer_ >> pending_);
            } else {
                byte = is_.get();
            }
            return byte;
        }

      public:
        bit_reader(std::istream &is) : is_(is) {}

        std::istream &read(uint64_t &value, size_t bits) {
            value = 0;
            while (bits > 0) {
                if (bits >= 8) {
                    value = (value << 8) | read_byte();
                    bits -= 8;
                } else {
                    value = (value << 1) | read_bit();
                    bits -= 1;
                }
            }
            return is_;
        }
    };
};  // namespace bit_stream

namespace huffman_encode {
    struct encode_node {
        uint8_t symbol_ = 0;
        size_t frequency_ = 0;
        encode_node *left_ = nullptr;
        encode_node *right_ = nullptr;

        encode_node(uint8_t symbol, size_t frequency) : symbol_(symbol), frequency_(frequency) {}

        encode_node(encode_node *left, encode_node *right) : left_(left), right_(right) {
            frequency_ = left_->frequency_ + right_->frequency_;
        }

        bool is_leaf() const {
            return left_ == nullptr && right_ == nullptr;
        }
    };

    struct compare_nodes {
        bool operator()(const encode_node *lhs, const encode_node *rhs) {
            return lhs->frequency_ > rhs->frequency_;
        }
    };

    void generate_lengths(std::vector<std::pair<uint8_t, uint8_t>> &table, encode_node *node, uint8_t length = 0) {
        if (node->is_leaf()) {
            table.push_back({node->symbol_, length});
        } else {
            generate_lengths(table, node->left_, length + 1);
            generate_lengths(table, node->right_, length + 1);
        }
    }

    void generate_codes(std::vector<std::pair<uint8_t, uint8_t>> &lengths,
                        std::unordered_map<uint8_t, std::pair<uint8_t, uint32_t>> &codes) {
        uint32_t current_code = 0;
        auto [_, current_len] = lengths[0];
        for (auto &[symbol, length] : lengths) {
            if (length > current_len) {
                current_code <<= (length - current_len);
                current_len = length;
            }
            codes[symbol] = {length, current_code};
            current_code++;
        }
    }

    void delete_encode_tree(encode_node *node) {
        if (node->is_leaf()) {
            delete node;
        } else {
            delete_encode_tree(node->left_);
            delete_encode_tree(node->right_);
            delete node;
        }
    }

    int compression(const std::string &input_filename, const std::string &output_filename) {
        std::ifstream is(input_filename, std::ios::binary);
        if (!is) {
            std::println("Error: Could not open file {}", input_filename);
            return 1;
        }

        uint32_t number_symbols = 0;
        std::unordered_map<uint8_t, size_t> frequencies_table;
        std::istreambuf_iterator<char> start(is), end;
        std::for_each(start, end, [&frequencies_table, &number_symbols](const char &symbol) {
            frequencies_table[symbol] += 1;
            number_symbols += 1;
        });
        is.clear();
        is.seekg(0, std::ios::beg);

        std::vector<encode_node *> nodes;
        for (const auto &[symbol, frequency] : frequencies_table) {
            encode_node *node = new encode_node(symbol, frequency);
            nodes.push_back(node);
        }
        std::sort(nodes.begin(), nodes.end(), compare_nodes());

        while (nodes.size() > 1) {
            encode_node *left = nodes.back();
            nodes.pop_back();
            encode_node *right = nodes.back();
            nodes.pop_back();
            encode_node *parent = new encode_node(left, right);
            auto it = std::lower_bound(nodes.begin(), nodes.end(), parent, compare_nodes());
            nodes.insert(it, parent);
        }
        encode_node *root = nodes.back();
        nodes.pop_back();

        std::vector<std::pair<uint8_t, uint8_t>> lengths_table;
        generate_lengths(lengths_table, root);
        std::sort(lengths_table.begin(), lengths_table.end(), [](const auto &lhs, const auto &rhs) {
            if (lhs.second == rhs.second) {
                return lhs.first < rhs.first;
            }
            return lhs.second < rhs.second;
        });

        std::unordered_map<uint8_t, std::pair<uint8_t, uint32_t>> codes_table;
        generate_codes(lengths_table, codes_table);

        std::ofstream os(output_filename, std::ios::binary);
        if (!os) {
            std::println("Error: Could not open file {}", output_filename);
            return 1;
        }

        const std::string magic_number = "HUFFMAN2";
        os.write(magic_number.data(), 8);
        const uint8_t table_entries = codes_table.size() == 256 ? 0 : static_cast<uint8_t>(codes_table.size());
        os.write(reinterpret_cast<const char *>(&table_entries), 1);

        bit_stream::bit_writer writer(os);
        for (const auto &[symbol, length] : lengths_table) {
            writer.write(symbol, 8);
            writer.write(length, 5);
        }

        writer.write(number_symbols, 32);
        for (size_t index = 0; index < number_symbols; index++) {
            uint8_t symbol = is.get();
            const auto &[length, code] = codes_table[symbol];
            writer.write(code, length);
        }

        delete_encode_tree(root);
        return 0;
    }
};  // namespace huffman_encode

namespace huffman_decode {
    struct triplet {
        uint8_t symbol_ = 0;
        uint8_t length_ = 0;
        uint32_t code_ = 0;

        bool operator<(const triplet &rhs) const {
            return length_ < rhs.length_;
        }
    };

    void generate_codes(std::vector<std::pair<uint8_t, uint8_t>> &lengths, std::vector<triplet> &codes) {
        uint32_t current_code = 0;
        auto [_, current_len] = lengths[0];
        for (auto &[symbol, length] : lengths) {
            if (length > current_len) {
                current_code <<= (length - current_len);
                current_len = length;
            }
            codes.push_back({symbol, length, current_code});
            current_code++;
        }
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

        uint8_t raw_table_entries;
        is.read(reinterpret_cast<char *>(&raw_table_entries), 1);
        size_t table_entries = raw_table_entries == 0 ? 256 : raw_table_entries;

        bit_stream::bit_reader reader(is);
        std::vector<std::pair<uint8_t, uint8_t>> lengths_table;
        for (size_t index = 0; index < table_entries; index++) {
            uint64_t raw_symbol = 0, raw_length = 0;
            reader.read(raw_symbol, 8);
            reader.read(raw_length, 5);
            lengths_table.push_back({static_cast<uint8_t>(raw_symbol), static_cast<uint8_t>(raw_length)});
        }
        std::sort(lengths_table.begin(), lengths_table.end(), [](const auto &lhs, const auto &rhs) {
            if (lhs.second == rhs.second) {
                return lhs.first < rhs.first;
            }
            return lhs.second < rhs.second;
        });

        std::vector<triplet> codes_table;
        generate_codes(lengths_table, codes_table);

        uint64_t raw_number_symbols;
        reader.read(raw_number_symbols, 32);
        uint32_t number_symbols = static_cast<uint32_t>(raw_number_symbols);

        std::ofstream os(output_filename, std::ios::binary);
        if (!os) {
            std::println("Error: Could not open file {}", output_filename);
            return 1;
        }

        for (size_t index = 0; index < number_symbols; index++) {
            uint32_t decoded_code = 0;
            size_t current_length = 0, entry_index = 0;
            do {
                size_t entry_length = codes_table[entry_index].length_;
                if (current_length < entry_length) {
                    uint64_t raw_bit;
                    size_t bits_number = entry_length - current_length;
                    reader.read(raw_bit, bits_number);
                    decoded_code = (decoded_code << bits_number) | static_cast<uint32_t>(raw_bit);
                    current_length = entry_length;
                }
                if (decoded_code == codes_table[entry_index].code_) {
                    break;
                }
                entry_index += 1;
            } while (entry_index < table_entries);
            if (entry_index >= table_entries) {
                std::println("Error: Invalid code in file {}", input_filename);
                return 1;
            }
            os.put(codes_table[entry_index].symbol_);
        }

        return 0;
    }
};  // namespace huffman_decode

int main(int argc, char **argv) {
    if (argc != 4) {
        std::println("Usage: {} [c|d] <input file> <output file>", argv[0]);
        return 1;
    }

    const std::string operation = argv[1];
    const std::string input_filename = argv[2];
    const std::string output_filename = argv[3];

    if (operation == "c") {
        return huffman_encode::compression(input_filename, output_filename);
    } else if (operation == "d") {
        return huffman_decode::decompression(input_filename, output_filename);
    } else {
        std::println("Error: Invalid operation '{}'. Use 'c' for compression or 'd' for decompression", operation);
        return 1;
    }
}
