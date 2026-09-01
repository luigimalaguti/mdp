#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <print>
#include <string>
#include <vector>

namespace bencode {
    class element {
        char type_;

        std::string string_;
        int64_t integer_;
        std::vector<element> list_;
        std::map<std::string, element> dict_;

      public:
        element() : type_(0) {}

        element(std::istream &is) {
            std::string buffer;
            char next_symbol = is.peek();
            switch (next_symbol) {
                case 'i':
                    type_ = is.get();
                    std::getline(is, buffer, 'e');
                    integer_ = std::stoll(buffer);
                    break;
                case 'l':
                    type_ = is.get();
                    while (is.peek() != 'e') {
                        list_.emplace_back(is);
                    }
                    is.get();
                    break;
                case 'd':
                    type_ = is.get();
                    while (is.peek() != 'e') {
                        std::string key = element(is).string_;
                        dict_[key] = element(is);
                    }
                    is.get();
                    break;
                default:
                    type_ = 's';
                    std::getline(is, buffer, ':');
                    int64_t length = std::stoll(buffer);
                    string_ = std::string(length, '\0');
                    is.read(string_.data(), length);
                    break;
            }
        }

        void print(std::ostream &os, int32_t tabs = 0, bool pieces = false) const {
            std::string indent(tabs, '\t');
            switch (type_) {
                case 'i':
                    std::print(os, "{}", integer_);
                    break;
                case 's':
                    if (pieces) {
                        indent += "\t";
                        for (size_t chunk = 0; chunk < string_.size(); chunk += 20) {
                            std::print(os, "\n{}", indent);
                            for (size_t index = 0; index < 20; index++) {
                                std::print(os, "{:02x}", string_[chunk + index]);
                            }
                        }
                    } else {
                        std::print(os, "\"");
                        for (const auto &symbol : string_) {
                            auto character = 32 <= symbol && symbol <= 126 ? symbol : '.';
                            std::print(os, "{}", character);
                        }
                        std::print(os, "\"");
                    }
                    break;
                case 'l':
                    std::print(os, "[\n");
                    for (const auto &value : list_) {
                        std::print(os, "{}", indent + "\t");
                        value.print(os, tabs + 1);
                        std::print(os, "\n");
                    }
                    std::print(os, "{}]", indent);
                    break;
                case 'd':
                    std::print(os, "{{\n");
                    for (const auto &[key, value] : dict_) {
                        std::print(os, "{}\"{}\" => ", indent + "\t", key);
                        value.print(os, tabs + 1, key == "pieces");
                        std::print(os, "\n");
                    }
                    std::print(os, "{}}}", indent);
                    break;
                default:
                    break;
            }
        }
    };
}  // namespace bencode

int main(int argc, char **argv) {
    if (argc != 2) {
        std::println("Usage: {} <file .torrent>", argv[0]);
        return 1;
    }

    const std::string input_filename = argv[1];

    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    bencode::element root(is);
    root.print(std::cout);

    return 0;
}
