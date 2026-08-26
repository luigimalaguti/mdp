#include <cstdint>
#include <fstream>
#include <print>
#include <string>
#include <vector>

enum state { IDLE, RUN, COPY };

int compression(const std::string &input_filename, const std::string &output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    state current_state = IDLE;
    uint8_t current_symbol = 0;
    uint8_t current_size = 0;
    std::vector<uint8_t> buffer(128);

    while (is.read(reinterpret_cast<char *>(&current_symbol), 1)) {
        switch (current_state) {
            case IDLE:
                current_size = 0;
                buffer[current_size] = current_symbol;
                current_size += 1;
                current_state = RUN;
                break;

            case RUN:
                if (current_symbol == buffer[current_size - 1]) {
                    buffer[current_size] = current_symbol;
                    current_size += 1;
                    if (current_size == 128) {
                        os.put(257 - current_size);
                        os.put(buffer[current_size - 1]);
                        current_state = IDLE;
                    }
                } else {
                    if (current_size > 1) {
                        os.put(257 - current_size);
                        os.put(buffer[current_size - 1]);
                        current_size = 0;
                    }
                    buffer[current_size] = current_symbol;
                    current_size += 1;
                    current_state = COPY;
                }
                break;

            case COPY:
                if (current_symbol != buffer[current_size - 1]) {
                    if (current_size == 128) {
                        os.put(current_size - 1);
                        os.write(reinterpret_cast<const char *>(buffer.data()), current_size);
                        current_size = 0;
                        current_state = RUN;
                    }
                    buffer[current_size] = current_symbol;
                    current_size += 1;
                } else {
                    if (current_size > 1) {
                        os.put(current_size - 2);
                        os.write(reinterpret_cast<const char *>(buffer.data()), current_size - 1);
                    }
                    buffer[0] = current_symbol;
                    buffer[1] = current_symbol;
                    current_size = 2;
                    current_state = RUN;
                }
                break;

            default:
                break;
        }
    }

    if (current_size > 0) {
        if (current_state == COPY || current_size == 1) {
            os.put(current_size - 1);
            os.write(reinterpret_cast<const char *>(buffer.data()), current_size);
        } else {
            os.put(257 - current_size);
            os.put(buffer[current_size - 1]);
        }
    }
    os.put(128);

    return 0;
}

int decompression(const std::string &input_filename, const std::string &output_filename) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    uint8_t current_code = 0;
    while (is.read(reinterpret_cast<char *>(&current_code), 1)) {
        if (current_code == 128) {
            break;
        } else if (current_code > 128) {
            uint8_t runs = 257 - current_code;
            uint8_t next_symbol = is.get();
            std::vector<uint8_t> buffer(runs, next_symbol);
            os.write(reinterpret_cast<const char *>(buffer.data()), runs);
        } else {
            uint8_t copies = current_code + 1;
            std::vector<uint8_t> buffer(copies);
            is.read(reinterpret_cast<char *>(buffer.data()), copies);
            os.write(reinterpret_cast<const char *>(buffer.data()), copies);
        }
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
