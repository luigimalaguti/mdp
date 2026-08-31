#pragma once

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

namespace huffman {
    template <typename S = uint8_t>
    class frequency_table {
        std::unordered_map<S, size_t> occurrences_;
        size_t total_ = 0;

      public:
        void operator()(const S &symbol) {
            occurrences_[symbol] += 1;
            total_ += 1;
        }

        auto begin() const {
            return occurrences_.begin();
        }

        auto end() const {
            return occurrences_.end();
        }

        size_t total() const {
            return total_;
        }
    };

    struct codeword {
        uint32_t length;
        uint32_t bits;
    };

    template <typename S = uint8_t>
    struct symbol_codeword {
        S symbol;
        uint32_t length;
        uint32_t bits;
    };

    template <typename S = uint8_t>
    class length_table {
        std::vector<std::pair<S, uint32_t>> entries_;

      public:
        void operator()(const S &symbol, uint32_t length) {
            entries_.push_back({symbol, length});
        }

        auto begin() const {
            return entries_.begin();
        }

        auto end() const {
            return entries_.end();
        }
    };

    template <typename S = uint8_t>
    class codeword_table {
        std::vector<symbol_codeword<S>> entries_;

      public:
        void operator()(const S &symbol, uint32_t length, uint32_t bits) {
            entries_.push_back({symbol, length, bits});
        }

        const auto &operator[](size_t index) const {
            return entries_[index];
        }

        auto &operator[](size_t index) {
            return entries_[index];
        }

        auto begin() const {
            return entries_.begin();
        }

        auto end() const {
            return entries_.end();
        }

        size_t size() const {
            return entries_.size();
        }
    };

    /*
     * STANDARD HUFFMAN
     */

    template <typename S>
    struct node {
        S symbol{};
        size_t frequency{0};
        node<S> *left = nullptr;
        node<S> *right = nullptr;

        node(S symbol, size_t frequency) : symbol(symbol), frequency(frequency) {}

        node(node<S> *left, node<S> *right) : frequency(left->frequency + right->frequency), left(left), right(right) {}

        bool is_leaf() const {
            return left == nullptr;
        }
    };

    template <typename S = uint8_t>
    class encoder {
        codeword_table<S> ordered_codes_;
        std::unordered_map<S, codeword> codes_;

        static bool by_frequency_descending(const node<S> *a, const node<S> *b) {
            return a->frequency > b->frequency;
        }

        static node<S> *generate_tree(const frequency_table<S> &frequencies) {
            std::vector<node<S> *> nodes;
            for (const auto &[symbol, frequency] : frequencies) {
                nodes.push_back(new node<S>(symbol, frequency));
            }
            if (nodes.empty()) {
                return nullptr;
            }
            std::sort(nodes.begin(), nodes.end(), by_frequency_descending);

            while (nodes.size() > 1) {
                node<S> *right = nodes.back();
                nodes.pop_back();
                node<S> *left = nodes.back();
                nodes.pop_back();
                node<S> *parent = new node<S>(left, right);
                auto it = std::lower_bound(nodes.begin(), nodes.end(), parent, by_frequency_descending);
                nodes.insert(it, parent);
            }
            node<S> *root = nodes.back();
            nodes.pop_back();

            return root;
        }

        static void delete_tree(node<S> *root) {
            if (root == nullptr) {
                return;
            }
            delete_tree(root->left);
            delete_tree(root->right);
            delete root;
        }

        void generate_codes(const node<S> *current, uint32_t length = 0, uint32_t bits = 0) {
            if (current->is_leaf()) {
                ordered_codes_(current->symbol, length, bits);
            } else {
                generate_codes(current->left, length + 1, (bits << 1) | 0x0);
                generate_codes(current->right, length + 1, (bits << 1) | 0x1);
            }
        }

      public:
        explicit encoder(const frequency_table<S> &frequencies) {
            node<S> *root = generate_tree(frequencies);
            if (root != nullptr) {
                generate_codes(root);
                delete_tree(root);
            }
            for (const auto &[symbol, length, bits] : ordered_codes_) {
                codes_[symbol] = {length, bits};
            }
        }

        const auto &operator[](const S &symbol) const {
            return codes_.at(symbol);
        }

        auto &operator[](const S &symbol) {
            return codes_[symbol];
        }

        size_t size() const {
            return codes_.size();
        }

        const codeword_table<S> &table() const {
            return ordered_codes_;
        }
    };

    template <typename S = uint8_t>
    class decoder {
        codeword_table<S> codes_;
        std::unordered_map<uint64_t, size_t> lookup_;

        static uint64_t hash_key(uint32_t length, uint32_t bits) {
            return (static_cast<uint64_t>(length) << 32) | bits;
        }

      public:
        explicit decoder(codeword_table<S> codes) : codes_(std::move(codes)) {
            for (size_t index = 0; index < codes_.size(); index++) {
                lookup_[hash_key(codes_[index].length, codes_[index].bits)] = index;
            }
        }

        const auto &operator[](size_t index) const {
            return codes_[index];
        }

        auto &operator[](size_t index) {
            return codes_[index];
        }

        int32_t find(const codeword &query) const {
            auto it = lookup_.find(hash_key(query.length, query.bits));
            if (it != lookup_.end()) {
                return static_cast<int32_t>(it->second);
            }
            return -1;
        }

        size_t size() const {
            return codes_.size();
        }
    };

    /*
     * CANONICAL HUFFMAN
     */

    template <typename S = uint8_t>
    class canonical_encoder {
        std::vector<std::pair<S, codeword>> ordered_codes_;
        std::unordered_map<S, codeword> codes_;

        static bool by_length_ascending(const std::pair<S, codeword> &a, const std::pair<S, codeword> &b) {
            return a.second.length < b.second.length;
        }

        void assign_canonical_codes() {
            std::sort(ordered_codes_.begin(), ordered_codes_.end(), by_length_ascending);

            uint32_t current_bits = 0;
            uint32_t current_length = 0;
            for (auto &[symbol, code] : ordered_codes_) {
                current_bits = current_bits << (code.length - current_length);
                current_length = code.length;
                code.bits = current_bits;
                current_bits += 1;
            }
        }

      public:
        explicit canonical_encoder(const frequency_table<S> &frequencies) {
            encoder<S> standard(frequencies);
            for (const auto &[symbol, length, bits] : standard.table()) {
                ordered_codes_.push_back({symbol, {length, 0}});
            }
            assign_canonical_codes();

            for (const auto &[symbol, code] : ordered_codes_) {
                codes_[symbol] = code;
            }
        }

        const auto &operator[](const S &symbol) const {
            return codes_.at(symbol);
        }

        auto &operator[](const S &symbol) {
            return codes_[symbol];
        }

        size_t size() const {
            return codes_.size();
        }

        length_table<S> table() const {
            length_table<S> lengths;
            for (const auto &[symbol, code] : ordered_codes_) {
                lengths(symbol, code.length);
            }
            return lengths;
        }
    };

    template <typename S = uint8_t>
    class canonical_decoder {
        decoder<S> standard_decoder_;

        static bool by_length_ascending(const std::pair<S, uint32_t> &a, const std::pair<S, uint32_t> &b) {
            return a.second < b.second;
        }

        static codeword_table<S> assign_canonical_codes(const length_table<S> &lengths) {
            std::vector<std::pair<S, uint32_t>> symbols(lengths.begin(), lengths.end());
            std::stable_sort(symbols.begin(), symbols.end(), by_length_ascending);

            codeword_table<S> codes;
            uint32_t current_bits = 0;
            uint32_t current_length = 0;
            for (const auto &[symbol, length] : symbols) {
                if (length > current_length) {
                    current_bits = current_bits << (length - current_length);
                    current_length = length;
                }
                codes(symbol, current_length, current_bits);
                current_bits += 1;
            }

            return codes;
        }

      public:
        explicit canonical_decoder(const length_table<S> &lengths)
            : standard_decoder_(assign_canonical_codes(lengths)) {}

        const auto &operator[](size_t index) const {
            return standard_decoder_[index];
        }

        auto &operator[](size_t index) {
            return standard_decoder_[index];
        }

        int32_t find(const codeword &query) const {
            return standard_decoder_.find(query);
        }

        size_t size() const {
            return standard_decoder_.size();
        }
    };
}  // namespace huffman
