#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int compare_int32(const void *pointer_a, const void *pointer_b) {
    int32_t number_a = *(int32_t *)pointer_a;
    int32_t number_b = *(int32_t *)pointer_b;
    return (number_a > number_b) - (number_a < number_b);
}

typedef struct vector {
    size_t size_;
    size_t capacity_;
    int32_t *data_;
} vector_t;

vector_t *vector_constructor(vector_t *this) {
    this->size_ = 0;
    this->capacity_ = 0;
    this->data_ = NULL;
    return this;
}

vector_t *vector_destructor(vector_t *this) {
    free(this->data_);
    return this;
}

vector_t *vector_new(void) {
    vector_t *this = malloc(sizeof(vector_t));
    return vector_constructor(this);
}

void vector_delete(vector_t *this) {
    free(vector_destructor(this));
}

void vector_push_back(vector_t *this, int32_t number) {
    if (this->size_ >= this->capacity_) {
        size_t temp_capacity = this->capacity_ == 0 ? 1 : this->capacity_ * 2;
        int32_t *temp_data = realloc(this->data_, temp_capacity * sizeof(int32_t));
        this->data_ = temp_data;
        this->capacity_ = temp_capacity;
    }
    this->data_[this->size_] = number;
    this->size_++;
}

void vector_sort(vector_t *this) {
    qsort(this->data_, this->size_, sizeof(int32_t), compare_int32);
}

size_t vector_size(const vector_t *this) {
    return this->size_;
}

int32_t vector_at(const vector_t *this, size_t index) {
    assert(index < this->size_);
    return this->data_[index];
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open file %s\n", input_filename);
        return EXIT_FAILURE;
    }

    vector_t *vec = vector_new();

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        vector_push_back(vec, number);
    }
    fclose(input_file);

    vector_sort(vec);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        vector_delete(vec);
        printf("Error: Could not open file %s\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < vector_size(vec); index++) {
        fprintf(output_file, "%d\n", vector_at(vec, index));
    }
    fclose(output_file);

    vector_delete(vec);
    return EXIT_SUCCESS;
}
