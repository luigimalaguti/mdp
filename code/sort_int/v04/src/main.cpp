#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int compare_int32(const void *pointer_a, const void *pointer_b) {
    int32_t number_a = *(int32_t *)pointer_a;
    int32_t number_b = *(int32_t *)pointer_b;
    return (number_a > number_b) - (number_a < number_b);
}

struct vector {
        size_t size_;
        size_t capacity_;
        int32_t *data_;

        // Default constructor
        vector() {
            size_ = 0;
            capacity_ = 0;
            data_ = nullptr;
        }

        // Copy constructor
        vector(const vector &other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = new int32_t[other.capacity_];
            for (size_t index = 0; index < other.size_; index++) {
                data_[index] = other.data_[index];
            }
        }

        // Move constructor
        vector(vector &&other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = other.data_;
            other.data_ = nullptr;
        }

        ~vector() {
            delete[] data_;
        }

        // Assignment operator
        vector &operator=(const vector &rhs) {
            if (this != &rhs) {
                size_ = rhs.size_;
                capacity_ = rhs.capacity_;
                delete[] data_;
                data_ = new int32_t[rhs.capacity_];
                for (size_t index = 0; index < rhs.size_; index++) {
                    data_[index] = rhs.data_[index];
                }
            }
            return *this;
        }

        // Move operator
        vector &operator=(vector &&rhs) {
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            delete[] data_;
            data_ = rhs.data_;
            rhs.data_ = nullptr;
            return *this;
        }

        int push_back(int32_t number) {
            if (size_ >= capacity_) {
                size_t temp_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
                int32_t *temp_vector = new int32_t[temp_capacity];
                for (size_t index = 0; index < size_; index++) {
                    temp_vector[index] = data_[index];
                }
                delete[] data_;
                data_ = temp_vector;
                capacity_ = temp_capacity;
            }
            data_[size_] = number;
            size_++;
            return 1;
        }

        void shrink_to_fit() {
            if (size_ < capacity_) {
                int32_t *temp_vector = new int32_t[size_];
                for (size_t index = 0; index < size_; index++) {
                    temp_vector[index] = data_[index];
                }
                delete[] data_;
                data_ = temp_vector;
                capacity_ = size_;
            }
        }

        size_t size() const {
            return size_;
        }

        int32_t *data() {
            return data_;
        }

        int32_t at(size_t index) const {
            assert(index < size_);
            return data_[index];
        }

        void sort() {
            qsort(data_, size_, sizeof(*data_), compare_int32);
        }

        bool empty() const {
            return size_ == 0;
        }
};

vector read_from_file(const char *filename) {
    vector vec;
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        int32_t number;
        while (fscanf(file, "%d", &number) == 1) {
            int result = vec.push_back(number);
            if (result == false) {
                break;
            }
        }
        vec.shrink_to_fit();
        fclose(file);
    }
    return vec;
}

bool write_to_file(const char *filename, const vector &vec) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    for (size_t index = 0; index < vec.size(); index++) {
        fprintf(file, "%d\n", vec.at(index));
    }
    fclose(file);
    return true;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    vector vec = read_from_file(input_filename);
    if (vec.empty() == true) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    vec.sort();

    bool result = write_to_file(output_filename, vec);
    if (result == false) {
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
