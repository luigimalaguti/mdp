#include <cstdint>
#include <fstream>
#include <print>
#include <sstream>
#include <string>
#include <vector>

using rgb = std::array<uint8_t, 3>;

enum pixel_format {
    PIXEL_FORMAT_GRAYSCALE,
    PIXEL_FORMAT_RGB,
};

class matrix {
    pixel_format format_ = PIXEL_FORMAT_GRAYSCALE;
    size_t rows_ = 0;
    size_t cols_ = 0;
    std::vector<uint8_t> data_;

  public:
    matrix() {}

    matrix(pixel_format format, size_t rows, size_t cols) : format_(format), rows_(rows), cols_(cols) {
        uint8_t factor = format == PIXEL_FORMAT_RGB ? 3 : 1;
        data_.resize(rows * cols * factor);
    }

    template <typename T>
    const T &at(size_t row, size_t col) const {
        uint8_t factor = format_ == PIXEL_FORMAT_RGB ? 3 : 1;
        size_t index = (row * cols_ + col) * factor;
        return *reinterpret_cast<const T *>(&data_[index]);
    }

    template <typename T>
    T &at(size_t row, size_t col) {
        return const_cast<T &>(static_cast<const matrix *>(this)->at<T>(row, col));
    }

    pixel_format format() const {
        return format_;
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

    bool empty() const {
        return data_.empty();
    }

    size_t raw_size() const {
        return data_.size() * sizeof(uint8_t);
    }

    char *raw_data() {
        return reinterpret_cast<char *>(data_.data());
    }

    const char *raw_data() const {
        return reinterpret_cast<const char *>(data_.data());
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

    if (width <= 0 || height <= 0 || maxval != 255) {
        std::println("Error: Invalid PAM header values");
        return matrix();
    }
    if (tupltype != "RGB" && tupltype != "GRAYSCALE") {
        std::println("Error: Unsupported TUPLTYPE: {}", tupltype);
        return matrix();
    }
    if (depth != 1 && depth != 3) {
        std::println("Error: Unsupported DEPTH: {}", depth);
        return matrix();
    }

    pixel_format format = (tupltype == "RGB") ? PIXEL_FORMAT_RGB : PIXEL_FORMAT_GRAYSCALE;
    matrix data = matrix(format, height, width);
    is.read(data.raw_data(), static_cast<int32_t>(data.raw_size()));

    return data;
}

template <typename T>
void flip_image(matrix &data) {
    for (size_t col = 0; col < data.cols(); col++) {
        for (size_t row = 0; row < data.rows() / 2; row++) {
            std::swap(data.at<T>(row, col), data.at<T>(data.rows() - 1 - row, col));
        }
    }
}

template <typename T>
void mirror_image(matrix &data) {
    for (size_t row = 0; row < data.rows(); row++) {
        for (size_t col = 0; col < data.cols() / 2; col++) {
            std::swap(data.at<T>(row, col), data.at<T>(row, data.cols() - 1 - col));
        }
    }
}

void write_pam_image(std::ostream &os, matrix &data) {
    os << "P7\n";
    os << "WIDTH " << data.cols() << "\n";
    os << "HEIGHT " << data.rows() << "\n";
    uint8_t depth = data.format() == PIXEL_FORMAT_RGB ? 3 : 1;
    os << "DEPTH " << static_cast<int32_t>(depth) << "\n";
    os << "MAXVAL 255\n";
    std::string tupltype = data.format() == PIXEL_FORMAT_RGB ? "RGB" : "GRAYSCALE";
    os << "TUPLTYPE " << tupltype << "\n";
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

    mirror_image<rgb>(pam_data);
    write_pam_image(os, pam_data);

    return 0;
}
