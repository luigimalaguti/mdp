#include <algorithm>
#include <fstream>
#include <iterator>
#include <numeric>
#include <print>
#include <vector>

std::vector<int32_t> read_from_file(const char *filename) {
    std::vector<int32_t> vec;
    std::ifstream is(filename);
    if (!is) {
        return vec;
    }
    std::istream_iterator<int32_t> start(is);
    std::istream_iterator<int32_t> stop;
    std::copy(start, stop, std::back_inserter(vec));
    return vec;
}

bool write_to_file(const char *filename, const std::vector<int32_t> &vec) {
    std::ofstream os(filename);
    if (!os) {
        return false;
    }
    for (const auto &value : vec) {
        os << value << "\n";
    }
    return true;
}

struct entry {
    int32_t value_;
    double distance_;
};

int main(int argc, char **argv) {
    if (argc != 3) {
        std::println("Usage: {} <input_file> <output_file>", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    std::vector<int32_t> vec;
    vec = read_from_file(input_filename);
    if (vec.empty() == true) {
        std::println("Error: Could not open file {}", input_filename);
        return EXIT_FAILURE;
    }

    double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
    double average = sum / (double)vec.size();

    std::vector<entry> entries;
    std::transform(vec.begin(), vec.end(), std::back_inserter(entries),
                   [&average](int32_t number) { return entry{number, std::abs(number - average)}; });

    std::ranges::sort(entries, {}, &entry::distance_);

    for (const auto &value : entries) {
        std::println("{:.2f} - {}", value.distance_, value.value_);
    }

    std::sort(vec.begin(), vec.end());

    bool result = write_to_file(output_filename, vec);
    if (result == false) {
        std::println("Error: Could not open file {}", output_filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
