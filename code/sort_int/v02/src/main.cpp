#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int compare_int32(const void *pointer_a, const void *pointer_b) {
    int32_t number_a = *(int32_t *)pointer_a;
    int32_t number_b = *(int32_t *)pointer_b;
    return (number_a > number_b) - (number_a < number_b);
}

struct vector {
        size_t size_;
        size_t capacity_;
        int32_t *data_;

        vector() {
            size_ = 0;
            capacity_ = 0;
            data_ = NULL;
        }

        ~vector() {
            free(data_);
        }

        int push_back(int32_t number) {
            if (size_ >= capacity_) {
                size_t temp_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
                int32_t *temp_vector = (int32_t *)realloc(data_, temp_capacity * sizeof(*data_));
                if (temp_vector == NULL) {
                    return 0;
                }
                data_ = temp_vector;
                capacity_ = temp_capacity;
            }
            data_[size_] = number;
            size_++;
            return 1;
        }

        void shrink_to_fit() {
            if (size_ < capacity_) {
                int32_t *temp_vector = (int32_t *)realloc(data_, size_ * sizeof(*data_));
                if (temp_vector != NULL) {
                    data_ = temp_vector;
                    capacity_ = size_;
                }
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
};

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    vector vec;

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        int result = vec.push_back(number);
        if (!result) {
            printf("Error: Memory allocation failed\n");
            break;
        }
    }
    fclose(input_file);

    vec.shrink_to_fit();

    vec.sort();

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < vec.size(); index++) {
        fprintf(output_file, "%d\n", vec.at(index));
    }
    fclose(output_file);

    return EXIT_SUCCESS;
}
