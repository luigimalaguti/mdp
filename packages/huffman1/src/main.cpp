#include <cstdint>
#include <fstream>
#include <print>
#include <string>
#include <unordered_map>
#include <vector>

class bit_reader {
    std::istream &is_;
    uint8_t buffer_;
    size_t pending_;

    uint8_t read_bit() {
        if (pending_ == 0) {
            buffer_ = is_.get();
            pending_ = 8;
        }
        pending_ -= 1;
        return (buffer_ >> pending_) & 0x1;
    }

    uint8_t read_byte() {
        if (pending_ > 0) {
            uint8_t byte = buffer_ << (8 - pending_);
            buffer_ = is_.get();
            byte = byte | (buffer_ >> pending_);
            return byte;
        } else {
            buffer_ = is_.get();
            return buffer_;
        }
    }

public:
    bit_reader(std::istream &is) : is_(is), buffer_(0), pending_(0) {}

    std::istream &operator()(uint64_t &value, size_t bits) {
        return read(value, bits);
    }

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

class bit_writer {
    std::ostream &os_;
    uint8_t buffer_;
    size_t pending_;

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
    bit_writer(std::ostream &os) : os_(os), buffer_(0), pending_(0) {}

    ~bit_writer() {
        flush();
    }

    std::ostream &operator()(const uint64_t value, size_t bits) {
        return write(value, bits);
    }

    std::ostream &write(const uint64_t value, size_t bits) {
        while (bits > 0) {
            if (bits >= 8) {
                uint8_t current = (value >> (bits - 8)) & 0xff;
                write_byte(current);
                bits -= 8;
            } else {
                uint8_t current = (value >> (bits - 1)) & 0x1;
                write_bit(current);
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

struct node {
    uint8_t symbol_;
    size_t frequency_;
    node *left_;
    node *right_;

    node(uint8_t symbol, size_t frequency) : symbol_(symbol), frequency_(frequency), left_(nullptr), right_(nullptr) {}

    node(node *left, node *right)
        : symbol_(0), frequency_(left->frequency_ + right->frequency_), left_(left), right_(right) {}
};

node *build_tree(const std::unordered_map<uint8_t, size_t> &symbols_frequency) {
    std::vector<node *> tree;
    for (const auto &symbol : symbols_frequency) {
        const auto &key = symbol.first;
        const auto &value = symbol.second;
        node *leaf = new node(key, value);
        tree.push_back(leaf);
    }
    std::sort(tree.begin(), tree.end(), [](node *a, node *b) {
        if (a->frequency_ == b->frequency_) {
            return a->symbol_ > b->symbol_;
        }
        return a->frequency_ > b->frequency_;
    });

    while (tree.size() > 1) {
        node *left = tree.back();
        tree.pop_back();
        node *right = tree.back();
        tree.pop_back();

        node *parent = new node(left, right);
        auto it = tree.begin();
        for (size_t index = 0; index < tree.size(); index++) {
            if (tree[index]->frequency_ <= parent->frequency_) {
                break;
            }
            it += 1;
        }
        tree.insert(it, parent);
    }
    node *root = tree.back();
    tree.pop_back();
    return root;
}

void create_codes(std::unordered_map<uint8_t, std::pair<uint8_t, uint32_t>> &table, node *leaf, uint8_t length,
                  uint32_t code) {
    if (leaf->left_ == nullptr) {
        table[leaf->symbol_] = std::make_pair(length, code);
    } else {
        create_codes(table, leaf->left_, length + 1, (code << 1) | 0x0);
        create_codes(table, leaf->right_, length + 1, (code << 1) | 0x1);
    }
}

int compression(const std::string &input_filename, const std::string output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    is.seekg(0, std::ios::end);
    const auto length = is.tellg();
    is.seekg(0, std::ios::beg);
    if (length <= 0) {
        std::println("Error: Could not read file {}", input_filename);
        return 1;
    }

    const auto elements = static_cast<size_t>(length) / sizeof(uint8_t);
    if (elements == 0) {
        std::println("Error: Could not read file {}", input_filename);
        return 1;
    }

    std::vector<uint8_t> data(elements);
    is.read(reinterpret_cast<char *>(data.data()), static_cast<int64_t>(elements * sizeof(uint8_t)));

    std::unordered_map<uint8_t, size_t> symbols_frequency;
    for (const auto &symbol : data) {
        symbols_frequency[symbol] += 1;
    }

    node *root = build_tree(symbols_frequency);
    std::unordered_map<uint8_t, std::pair<uint8_t, uint32_t>> table;
    create_codes(table, root, 0, 0);

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    bit_writer writer(os);
    std::print(os, "HUFFMAN1");
    writer(table.size(), 8);
    for (const auto &triplet : table) {
        const auto &symbol = triplet.first;
        const auto &length = triplet.second.first;
        const auto &code = triplet.second.second;
        writer(symbol, 8);
        writer(length, 5);
        writer(code, length);
    }
    writer(data.size(), 32);
    for (const auto &symbol : data) {
        const auto &length = table[symbol].first;
        const auto &code = table[symbol].second;
        writer(code, length);
    }

    return 0;
}

int decompression(const std::string &input_filename, const std::string output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    bit_reader reader(is);
    uint64_t entries = 0;
    reader(entries, 64);
    entries = 0;
    reader(entries, 8);

    std::unordered_map<uint8_t, std::pair<uint8_t, uint32_t>> table;
    for (size_t index = 0; index < entries; index++) {
        uint64_t symbol = 0;
        reader(symbol, 8);
        uint64_t length = 0;
        reader(length, 5);
        uint64_t code = 0;
        reader(code, length);
        table[symbol] = std::make_pair(length, code);
    }

    uint64_t symbols = 0;
    reader(symbols, 32);

    std::vector<uint8_t> data;
    for (size_t index = 0; index < symbols; index++) {
        uint64_t code = 0;
        while (!table.contains(code)) {
            uint64_t bit = 0;
            reader(bit, 1);
            code = (code << 1) | bit;
        }
    }

    std::ofstream os(output_filename);
    if (!os) {
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
