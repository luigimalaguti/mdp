#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <print>
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
            uint8_t byte;
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
    class frequency_table {
        std::unordered_map<uint8_t, size_t> frequencies_;
        size_t total_frequencies_ = 0;

      public:
        void operator()(uint8_t symbol) {
            frequencies_[symbol] += 1;
            total_frequencies_ += 1;
        }

        auto begin() const {
            return frequencies_.begin();
        }

        auto end() const {
            return frequencies_.end();
        }

        size_t total() const {
            return total_frequencies_;
        }
    };

    struct tree_node {
        int16_t symbol_ = -1;
        size_t frequency_ = 0;
        tree_node *left_ = nullptr;
        tree_node *right_ = nullptr;

        tree_node() {}

        tree_node(int16_t symbol, size_t frequency) : symbol_(symbol), frequency_(frequency) {}

        tree_node(tree_node *left, tree_node *right)
            : frequency_(left->frequency_ + right->frequency_), left_(left), right_(right) {}

        bool operator!() const {
            return left_ == nullptr && right_ == nullptr;
        }

        int operator<(const tree_node &rhs) const {
            if (frequency_ == rhs.frequency_) {
                return symbol_ < rhs.symbol_;
            }
            return frequency_ > rhs.frequency_;
        }
    };

    struct code_pair {
        uint8_t length_;
        uint32_t code_;
    };

    class codes_table {
        std::unordered_map<uint8_t, code_pair> codes_;

      public:
        void operator()(uint8_t symbol, uint8_t length, uint32_t code) {
            codes_[symbol] = {length, code};
        }

        const code_pair &operator[](uint8_t symbol) {
            return codes_[symbol];
        }

        auto begin() const {
            return codes_.begin();
        }

        auto end() const {
            return codes_.end();
        }

        size_t size() const {
            return codes_.size();
        }
    };

    frequency_table create_frequency_table(std::istream &is) {
        std::istreambuf_iterator<char> start(is);
        std::istreambuf_iterator<char> stop;
        frequency_table frequencies;
        frequencies = std::for_each(start, stop, frequencies);
        is.clear();
        is.seekg(0, std::ios::beg);
        return frequencies;
    }

    tree_node *create_binary_tree(frequency_table &frequencies) {
        std::vector<tree_node *> nodes;
        for (const auto &[symbol, frequency] : frequencies) {
            tree_node *node = new tree_node(symbol, frequency);
            nodes.push_back(node);
        }

        auto compare_nodes = [](tree_node *lhs, tree_node *rhs) {
            return *lhs < *rhs;
        };
        std::sort(nodes.begin(), nodes.end(), compare_nodes);

        while (nodes.size() > 1) {
            tree_node *right = nodes.back();
            nodes.pop_back();
            tree_node *left = nodes.back();
            nodes.pop_back();
            tree_node *parent = new tree_node(left, right);
            auto it = std::lower_bound(nodes.begin(), nodes.end(), parent, compare_nodes);
            nodes.insert(it, parent);
        }
        tree_node *root = nodes.back();
        nodes.pop_back();

        return root;
    }

    void delete_binary_tree(tree_node *node) {
        if (!*node) {
            delete node;
        } else {
            delete_binary_tree(node->left_);
            delete_binary_tree(node->right_);
            delete node;
        }
    }

    void create_codes_table(codes_table &table, tree_node *node, uint8_t length, uint32_t code) {
        if (!*node) {
            table(node->symbol_, length, code);
        } else {
            create_codes_table(table, node->left_, length + 1, (code << 1) | 0);
            create_codes_table(table, node->right_, length + 1, (code << 1) | 1);
        }
    }

    void write_encoding(std::istream &is, std::ostream &os, frequency_table &frequencies, codes_table &codes) {
        const std::string magic_number = "HUFFMAN1";
        os.write(magic_number.data(), 8);

        const uint8_t table_entries = codes.size() == 256 ? 0 : codes.size();
        os.write(reinterpret_cast<const char *>(&table_entries), 1);

        bit_stream::bit_writer writer(os);
        for (const auto &[symbol, pair] : codes) {
            const auto &[length, code] = pair;
            writer.write(symbol, 8);
            writer.write(length, 5);
            writer.write(code, length);
        }

        const uint32_t number_symbols = frequencies.total();
        writer.write(number_symbols, 32);

        uint8_t symbol;
        while (is.read(reinterpret_cast<char *>(&symbol), 1)) {
            const auto &[length, code] = codes[symbol];
            writer.write(code, length);
        }
    }

    int compression(const std::string &input_filename, const std::string &output_filename) {
        std::ifstream is(input_filename, std::ios::binary);
        if (!is) {
            std::println("Error: Could not open file {}", input_filename);
            return 1;
        }

        frequency_table frequencies = create_frequency_table(is);
        if (frequencies.total() == 0) {
            std::println("Error: Input file {} is empty", input_filename);
            return 1;
        }

        tree_node *root = create_binary_tree(frequencies);
        codes_table codes;
        create_codes_table(codes, root, 0, 0);

        std::ofstream os(output_filename, std::ios::binary);
        if (!os) {
            std::println("Error: Could not open file {}", output_filename);
            delete_binary_tree(root);
            return 1;
        }

        write_encoding(is, os, frequencies, codes);
        delete_binary_tree(root);

        return 0;
    }
};  // namespace huffman_encode

namespace huffman_decode {
    struct code_triplet {
        uint8_t symbol_;
        uint8_t length_;
        uint32_t code_;
    };

    class codes_table {
        std::vector<code_triplet> codes_;

      public:
        void operator()(uint8_t symbol, uint8_t length, uint32_t code) {
            codes_.push_back({symbol, length, code});
        }

        auto begin() const {
            return codes_.begin();
        }

        auto end() const {
            return codes_.end();
        }
    };

    struct tree_node {
        int16_t symbol_ = -1;
        size_t frequency_ = 0;
        tree_node *left_ = nullptr;
        tree_node *right_ = nullptr;

        tree_node() {}

        tree_node(int16_t symbol, size_t frequency) : symbol_(symbol), frequency_(frequency) {}

        tree_node(tree_node *left, tree_node *right)
            : frequency_(left->frequency_ + right->frequency_), left_(left), right_(right) {}

        bool operator!() const {
            return left_ == nullptr && right_ == nullptr;
        }

        int operator<(const tree_node &rhs) const {
            if (frequency_ == rhs.frequency_) {
                return symbol_ < rhs.symbol_;
            }
            return frequency_ > rhs.frequency_;
        }
    };

    bool read_magic_number(std::istream &is) {
        const std::string magic_number = "HUFFMAN1";
        std::string read_magic_number(8, '\0');
        is.read(read_magic_number.data(), 8);
        return read_magic_number == magic_number;
    }

    size_t read_table_entries(std::istream &is) {
        uint8_t table_entries;
        is.read(reinterpret_cast<char *>(&table_entries), 1);
        return table_entries == 0 ? 256 : table_entries;
    }

    codes_table read_codes_table(bit_stream::bit_reader &reader, size_t table_entries) {
        codes_table codes;
        for (size_t i = 0; i < table_entries; ++i) {
            uint64_t symbol, length, code;
            reader.read(symbol, 8);
            reader.read(length, 5);
            reader.read(code, length);
            codes(symbol, length, code);
        }
        return codes;
    }

    tree_node *create_binary_tree(codes_table &codes) {
        tree_node *root = new tree_node();

        for (const auto &[symbol, length, code] : codes) {
            tree_node *current = root;
            for (int index = 1; index <= length; index++) {
                uint8_t bit = (code >> (length - index)) & 0x1;
                if (bit == 0) {
                    if (current->left_ == nullptr) {
                        current->left_ = new tree_node();
                    }
                    current = current->left_;
                } else {
                    if (current->right_ == nullptr) {
                        current->right_ = new tree_node();
                    }
                    current = current->right_;
                }
            }
            current->symbol_ = symbol;
        }

        return root;
    }

    void delete_binary_tree(tree_node *node) {
        if (!*node) {
            delete node;
        } else {
            delete_binary_tree(node->left_);
            delete_binary_tree(node->right_);
            delete node;
        }
    }

    void read_encoding(bit_stream::bit_reader &reader, std::ostream &os, tree_node *root) {
        uint32_t number_symbols;
        reader.read(reinterpret_cast<uint64_t &>(number_symbols), 32);

        for (uint32_t index = 0; index < number_symbols; index++) {
            tree_node *current = root;
            while (current->symbol_ == -1) {
                uint64_t bit;
                reader.read(bit, 1);
                current = bit == 0 ? current->left_ : current->right_;
            }
            os.put(static_cast<char>(current->symbol_));
        }
    }

    int decompression(const std::string &input_filename, const std::string &output_filename) {
        std::ifstream is(input_filename, std::ios::binary);
        if (!is) {
            std::println("Error: Could not open file {}", output_filename);
            return 1;
        }

        if (!read_magic_number(is)) {
            std::println("Error: Invalid magic number in file {}", input_filename);
            return 1;
        }
        const size_t table_entries = read_table_entries(is);

        bit_stream::bit_reader reader(is);
        codes_table codes = read_codes_table(reader, table_entries);
        tree_node *root = create_binary_tree(codes);

        std::ofstream os(output_filename, std::ios::binary);
        if (!os) {
            std::println("Error: Could not open file {}", output_filename);
            delete_binary_tree(root);
            return 1;
        }

        read_encoding(reader, os, root);
        delete_binary_tree(root);

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
