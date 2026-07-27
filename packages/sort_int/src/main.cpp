#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace mdp {
    template <typename T>
    int compare(const void *pointer_a, const void *pointer_b) {
        T value_a = *(T *)pointer_a;
        T value_b = *(T *)pointer_b;
        return (value_a > value_b) - (value_a < value_b);
    }

    template <typename T>
    class vector {
    private:
        size_t size_;
        size_t capacity_;
        T *data_;

    public:
        vector(size_t capacity = 0) : size_(0), capacity_(capacity), data_(nullptr) {
            if (capacity_ > 0) {
                data_ = new T[capacity_];
            }
        }

        vector(const vector &other) : size_(other.size_), capacity_(other.capacity_), data_(new T[capacity_]) {
            for (size_t index = 0; index < size_; index++) {
                data_[index] = other.data_[index];
            }
        }

        vector(vector &&other) : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
            other.data_ = nullptr;
        }

        ~vector() {
            delete[] data_;
        }

        vector &operator=(vector rhs) {
            swap(*this, rhs);
            return *this;
        }

        const T &operator[](size_t index) const {
            return data_[index];
        }

        T &operator[](size_t index) {
            return data_[index];
        }

        void push_back(const T &number) {
            if (size_ >= capacity_) {
                size_t temp_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
                T *temp_data = new T[temp_capacity];
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
            qsort(data_, size_, sizeof(T), compare<T>);
        }

        size_t size() const {
            return size_;
        }

        const T &at(size_t index) const {
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

mdp::vector<int32_t> read_from_file(const char *filename, size_t capacity) {
    mdp::vector<int32_t> vec(capacity);
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

bool write_to_file(const char *filename, const mdp::vector<int32_t> &vec) {
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

    mdp::vector<int32_t> vec;
    vec = read_from_file(input_filename, 10);
    if (vec.empty() == true) {
        printf("Error: Could not open file %s\n", input_filename);
        return EXIT_FAILURE;
    }

    mdp::vector<int32_t> sorted = vec;
    vec.sort();
    swap(vec, sorted);

    bool result = write_to_file(output_filename, sorted);
    if (result == false) {
        printf("Error: Could not open file %s\n", output_filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
