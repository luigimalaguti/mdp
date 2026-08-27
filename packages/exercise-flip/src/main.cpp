#include <cstdint>
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

    matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows * cols) {}

    uint8_t &operator[](size_t row, size_t col) {
        return data_[row * cols_ + col];
    }

    const uint8_t &operator[](size_t row, size_t col) const {
        return data_[row * cols_ + col];
    }

    bool empty() const {
        return data_.empty();
    }

    size_t rows() const {
        return rows_;
    }

    size_t cols() const {
        return cols_;
    }

    size_t size() const {
        return data_.size();
    }

    size_t raw_size() const {
        return size() * sizeof(uint8_t);
    }

    const char *raw_data() const {
        return reinterpret_cast<const char *>(data_.data());
    }

    char *raw_data() {
        return reinterpret_cast<char *>(data_.data());
    }
};

matrix read_pam_image(std::istream &is) {
    std::string magic_number;
    std::getline(is, magic_number);
    if (magic_number != "P7") {
        std::println("Error: Invalid PAM magic number: {}", magic_number);
        return matrix();
    }

    int32_t width, height, depth, maxval;
    std::string tupltype;

    std::string line;
    std::string keyword;
    while (std::getline(is, line)) {
        if (line[0] == '#') {
            continue;
        }

        std::stringstream stream(line);
        stream >> keyword;

        if (keyword == "WIDTH") {
            stream >> width;
        } else if (keyword == "HEIGHT") {
            stream >> height;
        } else if (keyword == "DEPTH") {
            stream >> depth;
        } else if (keyword == "MAXVAL") {
            stream >> maxval;
        } else if (keyword == "TUPLTYPE") {
            stream >> tupltype;
        } else if (keyword == "ENDHDR") {
            break;
        }
    }

    if (width < 0 || height < 0 || depth != 1 || maxval != 255 || tupltype != "GRAYSCALE") {
        std::println("Error: Invalid PAM header values");
        return matrix();
    }

    matrix data = matrix(height, width);
    if (!is.read(data.raw_data(), static_cast<int32_t>(data.raw_size()))) {
        return matrix();
    }

    return data;
}

void flip_image(matrix &data) {
    for (size_t col = 0; col < data.cols(); col++) {
        for (size_t row = 0; row < data.rows() / 2; row++) {
            std::swap(data[row, col], data[data.rows() - 1 - row, col]);
        }
    }
}

void write_pam_image(std::ostream &os, matrix &data) {
    os << "P7\n";
    os << "WIDTH " << data.cols() << "\n";
    os << "HEIGHT " << data.rows() << "\n";
    os << "DEPTH 1\n";
    os << "MAXVAL 255\n";
    os << "TUPLTYPE GRAYSCALE\n";
    os << "ENDHDR\n";
    os.write(data.raw_data(), static_cast<int32_t>(data.raw_size()));
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <input file> <output file>", argv[0]);
        return 1;
    }

    const std::string input_filename = argv[1];
    const std::string output_filename = argv[2];

    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    matrix pam_data = read_pam_image(is);
    if (pam_data.empty()) {
        std::println("Error: Could not read PAM image from file {}", input_filename);
        return 1;
    }

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    flip_image(pam_data);
    write_pam_image(os, pam_data);

    return 0;
}
