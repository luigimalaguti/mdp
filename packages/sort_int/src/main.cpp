#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace mdp {
    int compare_int32(const void *pointer_a, const void *pointer_b) {
        int32_t number_a = *(int32_t *)pointer_a;
        int32_t number_b = *(int32_t *)pointer_b;
        return (number_a > number_b) - (number_a < number_b);
    }

    class vector {
    private:
        size_t size_;
        size_t capacity_;
        int32_t *data_;

    public:
        vector() {
            size_ = 0;
            capacity_ = 0;
            data_ = nullptr;
        }

        vector(size_t capacity) {
            size_ = 0;
            capacity_ = capacity;
            data_ = new int32_t[capacity];
        }

        vector(const vector &other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = new int32_t[capacity_];
            for (size_t index = 0; index < size_; index++) {
                data_[index] = other.data_[index];
            }
        }

        vector(vector &&other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = other.data_;
            other.data_ = nullptr;
        }

        ~vector() {
            delete[] data_;
        }

        vector &operator=(vector rhs) {
            swap(*this, rhs);
            return *this;
        }

        void push_back(int32_t number) {
            if (size_ >= capacity_) {
                size_t temp_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
                int32_t *temp_data = new int32_t[temp_capacity];
                for (size_t index = 0; index < size_; index++) {
                    temp_data[index] = data_[index];
                }
                delete[] data_;
                data_ = temp_data;
                capacity_ = temp_capacity;
            }
            data_[size_] = number;
            size_++;
        }

        void sort() {
            qsort(data_, size_, sizeof(int32_t), compare_int32);
        }

        size_t size() const {
            return size_;
        }

        int32_t at(size_t index) const {
            assert(index < size_);
            return data_[index];
        }

        bool empty() const {
            return size_ == 0;
        }

        friend void swap(vector &vector_a, vector &vector_b) {
            using std::swap;
            swap(vector_a.size_, vector_b.size_);
            swap(vector_a.capacity_, vector_b.capacity_);
            swap(vector_a.data_, vector_b.data_);
        }
    };
}  // namespace mdp

mdp::vector read_from_file(const char *filename, size_t capacity) {
    mdp::vector vec(capacity);
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        int32_t number;
        while (fscanf(file, "%d", &number) == 1) {
            vec.push_back(number);
        }
        fclose(file);
    }
    return vec;
}

bool write_to_file(const char *filename, const mdp::vector &vec) {
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

    mdp::vector vec;
    vec = read_from_file(input_filename, 10);
    if (vec.empty() == true) {
        printf("Error: Could not open file %s\n", input_filename);
        return EXIT_FAILURE;
    }

    mdp::vector sorted = vec;
    vec.sort();
    swap(vec, sorted);

    bool result = write_to_file(output_filename, sorted);
    if (result == false) {
        printf("Error: Could not open file %s\n", output_filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
