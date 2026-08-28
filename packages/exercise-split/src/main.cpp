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

    size_t raw_size() {
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
        std::println("Error: Invalid PAM file format");
        return matrix();
    }

    int32_t width, height, depth, max_val;
    std::string tuple_type;

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
            stream >> max_val;
        } else if (keyword == "TUPLTYPE") {
            stream >> tuple_type;
        } else if (keyword == "ENDHDR") {
            break;
        }
    }

    if (width < 0 || height < 0 || depth != 3 || max_val != 255 || tuple_type != "RGB") {
        std::println("Error: Invalid PAM file format");
        return matrix();
    }

    matrix pam_data(height, width);
    if (!is.read(pam_data.raw_data(), static_cast<int32_t>(pam_data.raw_size()))) {
        std::println("Error: Could not read PAM image data");
        return matrix();
    }

    return pam_data;
}

bool write_split_planes(const std::string &filename, matrix &data) {
    const std::vector<std::string> plane_filenames = {filename + "_R.pam", filename + "_G.pam", filename + "_B.pam"};

    for (size_t index = 0; index < 3; index++) {
        std::ofstream os(plane_filenames[index], std::ios::binary);
        if (!os) {
            std::println("Error: Could not open file {}", plane_filenames[index]);
            return false;
        }

        os << "P7\n";
        os << "WIDTH " << data.cols() << "\n";
        os << "HEIGHT " << data.rows() << "\n";
        os << "DEPTH 1\n";
        os << "MAXVAL 255\n";
        os << "TUPLTYPE GRAYSCALE\n";
        os << "ENDHDR\n";

        for (size_t row = 0; row < data.rows(); ++row) {
            for (size_t col = 0; col < data.cols(); ++col) {
                uint8_t value = data[row, col, index];
                os.write(reinterpret_cast<const char *>(&value), sizeof(uint8_t));
            }
        }
    }

    return true;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::println("Usage: {} <filename.pam>", argv[0]);
        return 1;
    }

    const std::string input_filename = argv[1];
    const std::filesystem::path path(input_filename);
    const std::string base = (path.parent_path() / path.stem()).string();
    const std::string extension = path.extension().string();
    if (extension != ".pam") {
        std::println("Error: Input file must have .pam extension");
        return 1;
    }

    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::println("Error: Could not open file {}", input_filename);
        return 1;
    }

    matrix pam_data = read_pam_image(is);
    if (!write_split_planes(base, pam_data)) {
        std::println("Error: Could not write split planes");
        return 1;
    }

    return 0;
}
