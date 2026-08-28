#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>
#include <string>
#include <vector>

class matrix {
    size_t rows_ = 0;
    size_t cols_ = 0;
    std::vector<uint8_t> data_;

  public:
    matrix() {}

    matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows * cols * 3) {}

    const uint8_t &operator[](size_t row, size_t col, size_t channel) const {
        return data_[(row * cols_ + col) * 3 + channel];
    }

    uint8_t &operator[](size_t row, size_t col, size_t channel) {
        return data_[(row * cols_ + col) * 3 + channel];
    }

    size_t rows() const {
        return rows_;
    }

    size_t cols() const {
        return cols_;
    }

    size_t size() {
        return rows_ * cols_;
    }

    bool empty() const {
        return data_.empty();
    }

    size_t raw_size() const {
        return data_.size() * sizeof(uint8_t);
    }

    const char *raw_data() const {
        return reinterpret_cast<const char *>(data_.data());
    }

    char *raw_data() {
        return reinterpret_cast<char *>(data_.data());
    }
};

matrix read_split_planes(const std::string &input_filename) {
    std::filesystem::path path(input_filename);
    const std::string base = (path.parent_path() / path.stem()).string();
    const std::string extension = ".pam";

    std::ifstream is_r(base + "_R" + extension, std::ios::binary);
    if (!is_r) {
        std::println("Error: could not open file {}", base + "_R" + extension);
        return matrix();
    }
    std::ifstream is_g(base + "_G" + extension, std::ios::binary);
    if (!is_g) {
        std::println("Error: could not open file {}", base + "_G" + extension);
        return matrix();
    }
    std::ifstream is_b(base + "_B" + extension, std::ios::binary);
    if (!is_b) {
        std::println("Error: could not open file {}", base + "_B" + extension);
        return matrix();
    }

    int32_t width = 0;
    int32_t height = 0;

    std::vector<std::ifstream *> streams = {&is_r, &is_g, &is_b};
    for (auto *stream : streams) {
        std::string magic_number;
        std::getline(*stream, magic_number);
        if (magic_number != "P7") {
            std::println("Error: invalid magic number");
            return matrix();
        }

        int32_t plane_width, plane_height, plane_depth, plane_mav_val;
        std::string plane_tuple_type;

        std::string line;
        std::string keyword;
        while (std::getline(*stream, line)) {
            if (line[0] == '#') {
                continue;
            }

            std::stringstream line_stream(line);
            line_stream >> keyword;

            if (keyword == "WIDTH") {
                line_stream >> plane_width;
            } else if (keyword == "HEIGHT") {
                line_stream >> plane_height;
            } else if (keyword == "DEPTH") {
                line_stream >> plane_depth;
            } else if (keyword == "MAXVAL") {
                line_stream >> plane_mav_val;
            } else if (keyword == "TUPLTYPE") {
                line_stream >> plane_tuple_type;
            } else if (keyword == "ENDHDR") {
                break;
            }
        }

        if (plane_width < 0 || plane_height < 0) {
            std::println("Error: invalid width or height");
            return matrix();
        }
        if (width != 0 && height != 0 && (plane_width != width || plane_height != height)) {
            std::println("Error: invalid width or height");
            return matrix();
        }
        if (plane_depth != 1 || plane_mav_val != 255 || plane_tuple_type != "GRAYSCALE") {
            std::println("Error: invalid PAM header");
            return matrix();
        }

        width = plane_width;
        height = plane_height;
    }

    matrix pam_data(height, width);
    for (size_t channel = 0; channel < 3; channel++) {
        std::ifstream *stream = streams[channel];
        size_t index = 0;
        uint8_t value = 0;
        while (stream->read(reinterpret_cast<char *>(&value), sizeof(uint8_t))) {
            size_t row = index / width;
            size_t col = index % width;
            pam_data[row, col, channel] = value;
            index += 1;
        }
    }

    return pam_data;
}

bool write_pam_image(const std::string &input_filename, matrix &data) {
    const std::string output_filename = input_filename + "_reconstructed.pam";
    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: could not open file {}", output_filename);
        return false;
    }

    os << "P7\n";
    os << "WIDTH " << data.cols() << "\n";
    os << "HEIGHT " << data.rows() << "\n";
    os << "DEPTH 3\n";
    os << "MAXVAL 255\n";
    os << "TUPLTYPE RGB\n";
    os << "ENDHDR\n";

    os.write(data.raw_data(), static_cast<int32_t>(data.raw_size()));

    return true;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::println("Usage: {} <filename>", argv[0]);
        return 1;
    }

    const std::string input_filename = argv[1];
    matrix pam_data = read_split_planes(input_filename);
    if (pam_data.empty()) {
        std::println("Error: failed to read PAM data");
        return 1;
    }

    if (!write_pam_image(input_filename, pam_data)) {
        std::println("Error: failed to write PAM image");
        return 1;
    }

    return 0;
}
