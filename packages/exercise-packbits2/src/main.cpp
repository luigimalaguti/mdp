#include <cstdint>
#include <fstream>
#include <print>
#include <string>
#include <vector>

enum state { COPY, CUMULATE, RUN };

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

    state current_state = COPY;
    uint8_t current_symbol = 0;
    uint8_t current_size = 0;
    std::vector<uint8_t> buffer(128);

    while (is.read(reinterpret_cast<char *>(&current_symbol), 1)) {
        int32_t first_symbol = buffer[0];
        int32_t last_symbol = current_size < 1 ? -1 : buffer[current_size - 1];
        int32_t second_last_symbol = current_size < 2 ? -1 : buffer[current_size - 2];

        switch (current_state) {
            /*
             * COPY state.
             *
             * Always add the current symbol to the buffer first, then evaluate buffer boundaries and repetition
             * conditions to determine the next state transition.
             */
            case COPY:
                buffer[current_size] = current_symbol;
                current_size += 1;
                /*
                 * Buffer full, 128 bytes, and the last two symbols match ('a', 'a').
                 * The last two bytes might be the start of a RUN in the next iteration or just part of a subsequent
                 * COPY. Flush the first 126 bytes as a COPY block, keep the 2 matching symbols at indices 0-1, and
                 * remain in COPY state.
                 * ┌─────┬─────┬─────╥─────┬─────┐
                 * │ ... │ ... │ ... ║  a  │  a  │
                 * ├-----┼-----┼-----╫-----┼-----┤
                 * │  0  │ ... │ 125 ║ 126 │ 127 │
                 * └─────┴─────┴─────╨─────┴─────┘
                 */
                if (current_symbol == last_symbol && current_size == 128) {
                    os.put(current_size - 3);
                    os.write(reinterpret_cast<const char *>(buffer.data()), current_size - 2);
                    buffer[0] = current_symbol;
                    buffer[1] = current_symbol;
                    current_size = 2;
                }
                /*
                 * Buffer full, 128 bytes, and the last two symbols differ ('b', 'a').
                 * The 128th byte, index 127, might start a RUN with the incoming 129th byte. Transition to CUMULATE
                 * state to inspect the next symbol before deciding how to flush.
                 * ┌─────┬─────┬─────┬─────╥─────┐
                 * │ ... │ ... │  b  │  a  ║  ?  │
                 * ├-----┼-----┼-----┼-----╫-----┤
                 * │  0  │ ... │ 126 │ 127 ║ 128 │
                 * └─────┴─────┴─────┴─────╨─────┘
                 */
                else if (current_symbol != last_symbol && current_size == 128) {
                    current_state = CUMULATE;
                }
                /*
                 * Detected 3 consecutive matching symbols ('a', 'a', 'a').
                 * A RUN sequence is starting. Flush any preceding unique bytes, indices 0 to current size - 4, as a
                 * COPY block, reset the buffer size to 3 for the run sequence, and transition to RUN state.
                 * ┌─────┬─────┬─────╥─────┬─────┬─────┐
                 * │ ... │ ... │  b  ║  a  │  a  │  a  │
                 * ├-----┼-----┼-----╫-----┼-----┼-----┤
                 * │  0  │ ... │ 124 ║ 125 │ 126 │ 127 │
                 * └─────┴─────┴─────┴─────┴─────╨─────┘
                 */
                else if (current_symbol == last_symbol && current_symbol == second_last_symbol) {
                    if (current_size > 3) {
                        os.put(current_size - 4);
                        os.write(reinterpret_cast<const char *>(buffer.data()), current_size - 3);
                    }
                    buffer[0] = current_symbol;
                    current_size = 3;
                    current_state = RUN;
                }
                break;

            /*
             * CUMULATE state.
             *
             * Entered when the COPY buffer reaches 128 bytes and the 128th byte, index 127, differed from byte 126. The
             * current symbol read is the 129th byte, which will decide if the 128th byte is the start of a RUN or just
             * another symbol in a new COPY block.
             */
            case CUMULATE:
                /*
                 * If current symbol matches the 128th symbol, buffer[127], we have a potential 2-byte run ('a', 'a').
                 * Flush the first 127 bytes as a COPY block, keep the 2 matching symbols in the buffer, and return to
                 * COPY state to see if a 3rd identical byte arrives.
                 * ┌─────┬─────┬─────┬─────╥─────┐
                 * │ ... │ ... │  b  │  a  ║  a  │
                 * ├─────┼─────┼─────┼─────╫─────┤
                 * │  0  │ ... │ 126 │ 127 ║ 128 │
                 * └─────┴─────┴─────┴─────╨─────┘
                 */
                if (current_symbol == last_symbol) {
                    os.put(current_size - 2);
                    os.write(reinterpret_cast<const char *>(buffer.data()), current_size - 1);
                    buffer[0] = current_symbol;
                    buffer[1] = current_symbol;
                    current_size = 2;
                }
                /*
                 * If current symbol differs from the 128th symbol, the entire 128-byte buffer is unique. Flush all 128
                 * bytes as a COPY block, place current symbol at index 0, and return to COPY state.
                 * ┌─────┬─────┬─────┬─────╥─────┐
                 * │ ... │ ... │  b  │  a  ║  c  │
                 * ├─────┼─────┼─────┼─────╫─────┤
                 * │  0  │ ... │ 126 │ 127 ║ 128 │
                 * └─────┴─────┴─────┴─────╨─────┘
                 */
                else {
                    os.put(current_size - 1);
                    os.write(reinterpret_cast<const char *>(buffer.data()), current_size);
                    buffer[0] = current_symbol;
                    current_size = 1;
                }
                current_state = COPY;
                break;

            /*
             * RUN state.
             *
             * Entered when at least 3 identical consecutive symbols have been detected. Continues accumulating matching
             * symbols up to a maximum limit of 128 bytes.
             */
            case RUN:
                /*
                 * Current symbol matches the active run symbol, first symbol. Store it and check if we have reached
                 * the maximum run size limit of 128 bytes.
                 * If the maximum run length of 128 is reached, flush the RUN sequence to file. Reset size to 0 and
                 * switch back to COPY state.
                 */
                if (current_symbol == first_symbol) {
                    buffer[current_size] = current_symbol;
                    current_size += 1;
                    if (current_size == 128) {
                        os.put(257 - current_size);
                        os.put(first_symbol);
                        current_size = 0;
                        current_state = COPY;
                    }
                }
                /*
                 * Current symbol differs from the active run symbol. Flush the accumulated RUN sequence, start a new
                 * COPY block with current symbol, and return to COPY state.
                 */
                else {
                    os.put(257 - current_size);
                    os.put(first_symbol);
                    buffer[0] = current_symbol;
                    current_size = 1;
                    current_state = COPY;
                }
                break;

            default:
                break;
        }
    }

    if (current_size > 0) {
        if (current_state == COPY || current_state == CUMULATE || current_size == 1) {
            os.put(current_size - 1);
            os.write(reinterpret_cast<const char *>(buffer.data()), current_size);
        } else {
            os.put(257 - current_size);
            os.put(buffer[0]);
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
