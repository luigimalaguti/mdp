// #pragma once

// #include <algorithm>
// #include <cstdint>
// #include <unordered_map>
// #include <vector>

// namespace huffman {
//     template <typename S = uint8_t>
//     class frequency_table {
//         std::unordered_map<S, size_t> table_;
//         size_t total_ = 0;

//       public:
//         void operator()(const S &symbol) {
//             table_[symbol] += 1;
//             total_ += 1;
//         }

//         auto begin() const {
//             return table_.begin();
//         }

//         auto end() const {
//             return table_.end();
//         }
//     };

//     template <typename S>
//     struct node {
//         S symbol;
//         size_t frequency;
//         node<S> *left = nullptr;
//         node<S> *right = nullptr;

//         node(S symbol, size_t frequency) : symbol(symbol), frequency(frequency) {}

//         node(node<S> *left, node<S> *right) : left(left), right(right) {
//             symbol = S();
//             frequency = left->frequency + right->frequency;
//         }

//         bool is_leaf() const {
//             return left == nullptr && right == nullptr;
//         }
//     };

//     template <typename S>
//     struct compare_nodes {
//         bool operator()(const node<S> *a, const node<S> *b) const {
//             return a->frequency > b->frequency;
//         }
//     };

//     struct code {
//         uint8_t length;
//         uint32_t bits;
//     };

//     template <typename S = uint8_t>
//     class canonical_encoder {
//         std::unordered_map<S, code> codes_;

//         node<S> *generate_tree(const frequency_table<S> &frequency_table) {
//             std::vector<node<S> *> nodes;
//             for (const auto &[symbol, frequency] : frequency_table) {
//                 nodes.push_back(new node<S>(symbol, frequency));
//             }
//             std::sort(nodes.begin(), nodes.end(), compare_nodes<S>());

//             while (nodes.size() > 1) {
//                 node<S> *right = nodes.back();
//                 nodes.pop_back();
//                 node<S> *left = nodes.back();
//                 nodes.pop_back();
//                 node<S> *parent = new node<S>(left, right);
//                 auto it = std::lower_bound(nodes.begin(), nodes.end(), parent, compare_nodes<S>());
//                 nodes.insert(it, parent);
//             }
//             node<S> *root = nodes.back();
//             nodes.pop_back();

//             return root;
//         }

//         void generate_codes(std::unordered_map<S, code> &table, node<S> *node, uint8_t length = 0, uint32_t bits = 0)
//         {
//             if (node->is_leaf()) {
//                 table[node->symbol] = {length, bits};
//             } else {
//                 generate_codes(table, node->left, length + 1, (bits << 1) | 0x0);
//                 generate_codes(table, node->right, length + 1, (bits << 1) | 0x1);
//             }
//         }

//         void generate_canonical_codes(std::unordered_map<S, code> &table) {
//             std::vector<std::pair<S, code>> symbols;
//             for (const auto &[symbol, code] : table) {
//                 symbols.push_back({symbol, code});
//             }
//             std::sort(symbols.begin(), symbols.end(), [](const auto &a, const auto &b) {
//                 return a.second.length < b.second.length || (a.second.length == b.second.length && a.first <
//                 b.first);
//             });

//             uint32_t current_code = 0;
//             uint8_t current_length = 0;
//             for (const auto &[symbol, code] : symbols) {
//                 if (code.length > current_length) {
//                     current_code = current_code << (code.length - current_length);
//                     current_length = code.length;
//                 }
//                 table[symbol] = {current_length, current_code};
//                 current_code += 1;
//             }
//         }

//         void delete_tree(node<S> *node) {
//             if (node->is_leaf()) {
//                 delete node;
//             } else {
//                 delete_tree(node->left);
//                 delete_tree(node->right);
//                 delete node;
//             }
//         }

//       public:
//         canonical_encoder(const frequency_table<S> &frequency_table) {
//             node<S> *root = generate_tree(frequency_table);
//             generate_codes(codes_, root);
//             generate_canonical_codes(codes_);
//         }

//         const auto &operator[](const S &symbol) const {
//             return codes_[symbol];
//         }

//         auto &operator[](const S &symbol) {
//             return codes_[symbol];
//         }

//         auto begin() const {
//             return codes_.begin();
//         }

//         auto end() const {
//             return codes_.end();
//         }

//         template <typename I>
//         static canonical_encoder from_iterator(I begin, I end) {
//             frequency_table<S> frequency = std::for_each(begin, end, frequency_table<S>());
//             return canonical_encoder(frequency);
//         }

//         size_t size() const {
//             return codes_.size();
//         }

//         std::vector<std::pair<S, uint8_t>> to_vector() const {
//             std::vector<std::pair<S, uint8_t>> vec;
//             for (const auto &[symbol, code] : codes_) {
//                 vec.push_back({symbol, code.length});
//             }
//             std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
//                 return a.second < b.second;
//             });
//             return vec;
//         }
//     };

//     template <typename S = uint8_t>
//     class length_table {
//         std::unordered_map<S, uint8_t> table_;

//       public:
//         const auto &operator[](const S &symbol) const {
//             return table_[symbol];
//         }

//         auto &operator[](const S &symbol) {
//             return table_[symbol];
//         }

//         auto begin() const {
//             return table_.begin();
//         }

//         auto end() const {
//             return table_.end();
//         }
//     };

//     template <typename S = uint8_t>
//     struct triplet {
//         S symbol;
//         uint8_t length;
//         uint32_t bits;
//     };

//     template <typename S = uint8_t>
//     class canonical_decoder {
//         std::vector<triplet<S>> codes_;

//         void generate_canonical_codes(const length_table<S> &length_table) {
//             std::vector<std::pair<S, uint8_t>> symbols;
//             for (const auto &[symbol, length] : length_table) {
//                 symbols.push_back({symbol, length});
//             }
//             std::sort(symbols.begin(), symbols.end(), [](const auto &a, const auto &b) {
//                 return a.second < b.second || (a.second == b.second && a.first < b.first);
//             });

//             uint32_t current_code = 0;
//             uint8_t current_length = 0;
//             for (const auto &[symbol, length] : symbols) {
//                 if (length > current_length) {
//                     current_code = current_code << (length - current_length);
//                     current_length = length;
//                 }
//                 codes_.push_back({symbol, current_length, current_code});
//                 current_code += 1;
//             }
//         }

//       public:
//         canonical_decoder(const length_table<S> &length_table) {
//             generate_canonical_codes(length_table);
//         }

//         template <typename I>
//         static canonical_decoder from_iterator(I begin, I end) {
//             length_table<S> length = std::for_each(begin, end, length_table<S>());
//             return canonical_decoder(length);
//         }

//         const auto &operator[](const size_t index) const {
//             return codes_[index];
//         }

//         auto &operator[](const size_t index) {
//             return codes_[index];
//         }

//         int32_t find(const code &code) {
//             for (size_t index = 0; index < codes_.size(); index++) {
//                 const auto &entry = codes_[index];
//                 if (entry.length == code.length && entry.bits == code.bits) {
//                     return static_cast<int32_t>(index);
//                 }
//             }
//             return -1;
//         }

//         size_t size() const {
//             return codes_.size();
//         }
//     };
// }  // namespace huffman

#pragma once

#include <algorithm>
#include <cstdint>
#include <queue>
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
        S min_symbol{};
        size_t frequency{0};
        node<S> *left = nullptr;
        node<S> *right = nullptr;

        node(S symbol, size_t frequency) : symbol(symbol), min_symbol(symbol), frequency(frequency) {}

        node(node<S> *left, node<S> *right)
            : symbol(S()),
              min_symbol(std::min(left->min_symbol, right->min_symbol)),
              frequency(left->frequency + right->frequency),
              left(left),
              right(right) {}

        bool is_leaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    template <typename S>
    struct compare_nodes {
        bool operator()(const node<S> *a, const node<S> *b) const {
            if (a->frequency != b->frequency) {
                return a->frequency > b->frequency;  // Min-heap: frequenza minore ha priorità
            }
            return a->min_symbol > b->min_symbol;  // Pareggio: simbolo minimo minore ha priorità
        }
    };

    struct code {
        uint8_t length;
        uint32_t bits;
    };

    template <typename S = uint8_t>
    class canonical_encoder {
        std::unordered_map<S, code> codes_;

        void free_tree(node<S> *n) {
            if (!n) {
                return;
            }
            free_tree(n->left);
            free_tree(n->right);
            delete n;
        }

        node<S> *generate_tree(const frequency_table<S> &freq_table) {
            std::priority_queue<node<S> *, std::vector<node<S> *>, compare_nodes<S>> pq;

            for (const auto &[symbol, frequency] : freq_table) {
                pq.push(new node<S>(symbol, frequency));
            }

            if (pq.empty()) {
                return nullptr;
            }

            if (pq.size() == 1) {
                return pq.top();
            }

            while (pq.size() > 1) {
                node<S> *left = pq.top();
                pq.pop();
                node<S> *right = pq.top();
                pq.pop();
                node<S> *parent = new node<S>(left, right);
                pq.push(parent);
            }

            return pq.top();
        }

        void generate_codes(std::unordered_map<S, code> &table, node<S> *n, uint8_t length = 0, uint32_t bits = 0) {
            if (!n) {
                return;
            }
            if (n->is_leaf()) {
                table[n->symbol] = {length == 0 ? static_cast<uint8_t>(1) : length, bits};
            } else {
                generate_codes(table, n->left, length + 1, (bits << 1) | 0x0);
                generate_codes(table, n->right, length + 1, (bits << 1) | 0x1);
            }
        }

        void generate_canonical_codes(std::unordered_map<S, code> &table) {
            std::vector<std::pair<S, code>> symbols(table.begin(), table.end());
            std::sort(symbols.begin(), symbols.end(), [](const auto &a, const auto &b) {
                return a.second.length < b.second.length || (a.second.length == b.second.length && a.first < b.first);
            });

            uint32_t current_code = 0;
            uint8_t current_length = 0;
            for (const auto &[symbol, c] : symbols) {
                if (c.length > current_length) {
                    current_code <<= (c.length - current_length);
                    current_length = c.length;
                }
                table[symbol] = {current_length, current_code};
                current_code += 1;
            }
        }

      public:
        canonical_encoder(const frequency_table<S> &freq_table) {
            node<S> *root = generate_tree(freq_table);
            if (root) {
                generate_codes(codes_, root);
                // Se l'albero consisteva di un solo nodo radice/foglia
                if (root->is_leaf()) {
                    delete root;
                } else {
                    free_tree(root);
                }
            }
            generate_canonical_codes(codes_);
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
            for (const auto &[symbol, c] : codes_) {
                vec.push_back({symbol, c.length});
            }
            std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
                return a.second < b.second || (a.second == b.second && a.first < b.first);
            });
            return vec;
        }
    };

    template <typename S = uint8_t>
    class length_table {
        std::unordered_map<S, uint8_t> table_;

      public:
        const auto &operator[](const S &symbol) const {
            return table_.at(symbol);
        }
        auto &operator[](const S &symbol) {
            return table_[symbol];
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
            std::vector<std::pair<S, uint8_t>> symbols(lengths.begin(), lengths.end());
            std::sort(symbols.begin(), symbols.end(), [](const auto &a, const auto &b) {
                return a.second < b.second || (a.second == b.second && a.first < b.first);
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
