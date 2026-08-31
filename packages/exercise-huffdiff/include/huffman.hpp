#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace huffman {

    template <typename S = uint8_t>
    class frequency_table {
        std::unordered_map<S, size_t> table_;
        size_t total_ = 0;

      public:
        void operator()(const S &symbol) {
            table_[symbol] += 1;
            total_ += 1;
        }

        auto begin() const {
            return table_.begin();
        }
        auto end() const {
            return table_.end();
        }
    };

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

    struct code {
        uint8_t length;
        uint32_t bits;
    };

    template <typename S = uint8_t>
    class canonical_encoder {
        // Ordine "canonico": raggruppato per lunghezza crescente. A parità di lunghezza
        // l'ordine NON è per simbolo, ma è quello che risulta da uno std::sort non stabile
        // (il confronto usato ignora il simbolo, guarda solo la lunghezza) applicato
        // all'ordine di visita dell'albero: per riprodurlo bit-esatto rispetto al
        // riferimento bisogna usare esattamente lo stesso algoritmo (std::sort/lower_bound
        // non stabili), non un ordinamento "pulito" per simbolo.
        std::vector<std::pair<S, code>> ordered_codes_;
        std::unordered_map<S, code> codes_;  // lookup veloce per simbolo

        std::vector<std::unique_ptr<node<S>>> storage_;

        static bool by_frequency_desc(const node<S> *a, const node<S> *b) {
            return a->frequency > b->frequency;
        }

        node<S> *generate_tree(const frequency_table<S> &freq_table) {
            std::vector<node<S> *> nodes;
            for (const auto &[symbol, frequency] : freq_table) {
                node<S> *n = new node<S>(symbol, frequency);
                storage_.emplace_back(n);
                nodes.push_back(n);
            }

            if (nodes.empty()) {
                return nullptr;
            }

            std::sort(nodes.begin(), nodes.end(), by_frequency_desc);

            while (nodes.size() > 1) {
                node<S> *n1 = nodes.back();
                nodes.pop_back();
                node<S> *n2 = nodes.back();
                nodes.pop_back();
                node<S> *parent = new node<S>(n1, n2);
                storage_.emplace_back(parent);
                auto it = std::lower_bound(nodes.begin(), nodes.end(), parent, by_frequency_desc);
                nodes.insert(it, parent);
            }

            return nodes.back();
        }

        void generate_codes(std::vector<std::pair<S, code>> &out, const node<S> *n, uint32_t length) {
            if (n->is_leaf()) {
                out.push_back({n->symbol, {static_cast<uint8_t>(length), 0}});
            } else {
                generate_codes(out, n->left, length + 1);
                generate_codes(out, n->right, length + 1);
            }
        }

      public:
        canonical_encoder(const frequency_table<S> &freq_table) {
            node<S> *root = generate_tree(freq_table);
            if (root) {
                generate_codes(ordered_codes_, root, 0);
            }
            // std::sort (non stabile) che confronta solo la lunghezza, come il riferimento.
            std::sort(ordered_codes_.begin(), ordered_codes_.end(), [](const auto &a, const auto &b) {
                return a.second.length < b.second.length;
            });

            uint32_t current_code = 0;
            uint8_t current_length = 0;
            for (auto &[symbol, c] : ordered_codes_) {
                current_code <<= (c.length - current_length);
                current_length = c.length;
                c.bits = current_code;
                current_code += 1;
            }

            for (const auto &[symbol, c] : ordered_codes_) {
                codes_[symbol] = c;
            }
        }

        const auto &operator[](const S &symbol) const {
            return codes_.at(symbol);
        }
        auto &operator[](const S &symbol) {
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

        std::vector<std::pair<S, uint8_t>> to_vector() const {
            std::vector<std::pair<S, uint8_t>> vec;
            for (const auto &[symbol, c] : ordered_codes_) {
                vec.push_back({symbol, c.length});
            }
            return vec;
        }
    };

    template <typename S = uint8_t>
    class length_table {
        // Vector invece di unordered_map: deve preservare l'ordine con cui le coppie
        // (simbolo, lunghezza) sono lette dal file, come richiesto dal formato.
        std::vector<std::pair<S, uint8_t>> table_;

      public:
        const auto &operator[](const S &symbol) const {
            for (const auto &[s, length] : table_) {
                if (s == symbol) {
                    return length;
                }
            }
            throw std::out_of_range("symbol not found");
        }
        auto &operator[](const S &symbol) {
            for (auto &[s, length] : table_) {
                if (s == symbol) {
                    return length;
                }
            }
            table_.push_back({symbol, uint8_t{0}});
            return table_.back().second;
        }
        auto begin() const {
            return table_.begin();
        }
        auto end() const {
            return table_.end();
        }
    };

    template <typename S = uint8_t>
    struct triplet {
        S symbol;
        uint8_t length;
        uint32_t bits;
    };

    template <typename S = uint8_t>
    class canonical_decoder {
        std::vector<triplet<S>> codes_;
        std::unordered_map<uint64_t, int32_t> lookup_;

        void generate_canonical_codes(const length_table<S> &lengths) {
            // Stable: a parità di lunghezza mantiene l'ordine con cui le coppie sono
            // apparse nel file (come richiesto dal formato), senza riordinare per simbolo.
            std::vector<std::pair<S, uint8_t>> symbols(lengths.begin(), lengths.end());
            std::stable_sort(symbols.begin(), symbols.end(), [](const auto &a, const auto &b) {
                return a.second < b.second;
            });

            uint32_t current_code = 0;
            uint8_t current_length = 0;
            for (const auto &[symbol, length] : symbols) {
                if (length > current_length) {
                    current_code <<= (length - current_length);
                    current_length = length;
                }
                int32_t idx = static_cast<int32_t>(codes_.size());
                codes_.push_back({symbol, current_length, current_code});

                uint64_t key = (static_cast<uint64_t>(current_length) << 32) | current_code;
                lookup_[key] = idx;
                current_code += 1;
            }
        }

      public:
        canonical_decoder(const length_table<S> &lengths) {
            generate_canonical_codes(lengths);
        }

        const auto &operator[](const size_t index) const {
            return codes_[index];
        }
        auto &operator[](const size_t index) {
            return codes_[index];
        }

        int32_t find(const code &c) const {
            uint64_t key = (static_cast<uint64_t>(c.length) << 32) | c.bits;
            auto it = lookup_.find(key);
            if (it != lookup_.end()) {
                return it->second;
            }
            return -1;
        }

        size_t size() const {
            return codes_.size();
        }
    };

}  // namespace huffman
