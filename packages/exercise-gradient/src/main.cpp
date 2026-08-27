#include <cstdint>
#include <fstream>
#include <print>
#include <string>
#include <vector>

struct pam {
    const std::string MAGIC_NUMBER = "P7";
    const size_t PIXELS = 256;
    const size_t WIDTH = PIXELS;
    const size_t HEIGHT = PIXELS;
    const size_t DEPTH = 1;
    const size_t SAMPLE_BIT = 8;
    const size_t MAXVAL = (0x1 << SAMPLE_BIT) - 0x1;
    const std::string TUPLTYPE = "GRAYSCALE";
};

void write_pam_header(std::ostream &os, pam &pam_config) {
    os << pam_config.MAGIC_NUMBER << "\n";
    os << "WIDTH " << pam_config.WIDTH << "\n";
    os << "HEIGHT " << pam_config.HEIGHT << "\n";
    os << "DEPTH " << pam_config.DEPTH << "\n";
    os << "MAXVAL " << pam_config.MAXVAL << "\n";
    os << "TUPLTYPE " << pam_config.TUPLTYPE << "\n";
    os << "ENDHDR\n";
}

class matrix {
    size_t rows_;
    size_t cols_;
    std::vector<uint8_t> data_;

  public:
    matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows * cols) {}

    uint8_t &operator[](const size_t row, const size_t col) {
        return data_[row * cols_ + col];
    }

    const uint8_t &operator[](const size_t row, const size_t col) const {
        return data_[row * cols_ + col];
    }

    size_t rows() const {
        return rows_;
    }

    size_t cols() const {
        return cols_;
    }

    size_t size() const {
        return rows_ * cols_;
    }

    size_t raw_size() const {
        return size() * sizeof(uint8_t);
    }

    const char *raw_data() const {
        return reinterpret_cast<const char *>(data_.data());
    }
};

matrix create_grayscale_gradient(const size_t rows, const size_t cols) {
    matrix data(rows, cols);
    for (size_t index = 0; index < rows * cols; index++) {
        size_t row = index / rows;
        size_t col = index % cols;
        data[row, col] = static_cast<uint8_t>(row);
    }
    return data;
}

void write_pam_data(std::ostream &os, matrix pam_data) {
    os.write(pam_data.raw_data(), static_cast<int32_t>(pam_data.raw_size()));
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::println("Usage: {} <output file>", argv[0]);
        return 1;
    }

    const std::string output_filename = argv[1];

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::println("Error: Could not open file {}", output_filename);
        return 1;
    }

    pam pam_config;
    write_pam_header(os, pam_config);
    matrix pam_data = create_grayscale_gradient(pam_config.WIDTH, pam_config.HEIGHT);
    write_pam_data(os, pam_data);

    return 0;
}
