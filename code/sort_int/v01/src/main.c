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
    if (this != NULL) {
        return vector_constructor(this);
    }
    return NULL;
}

void vector_delete(vector_t *this) {
    free(vector_destructor(this));
}

int vector_push_back(vector_t *this, int32_t number) {
    if (this->size_ >= this->capacity_) {
        size_t temp_capacity = (this->capacity_ == 0) ? 1 : this->capacity_ * 2;
        int32_t *temp_vector = realloc(this->data_, temp_capacity * sizeof(*this->data_));
        if (temp_vector == NULL) {
            return 0;
        }
        this->data_ = temp_vector;
        this->capacity_ = temp_capacity;
    }
    this->data_[this->size_] = number;
    this->size_++;
    return 1;
}

void vector_shrink_to_fit(vector_t *this) {
    if (this->size_ < this->capacity_) {
        int32_t *temp_vector = realloc(this->data_, this->size_ * sizeof(*this->data_));
        if (temp_vector != NULL) {
            this->data_ = temp_vector;
            this->capacity_ = this->size_;
        }
    }
}

size_t vector_size(const vector_t *this) {
    return this->size_;
}

int32_t *vector_data(vector_t *this) {
    return this->data_;
}

int32_t vector_at(const vector_t *this, size_t index) {
    assert(index < this->size_);
    return this->data_[index];
}

void vector_sort(vector_t *this) {
    qsort(this->data_, this->size_, sizeof(*this->data_), compare_int32);
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
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    vector_t *vector = vector_new();
    if (vector == NULL) {
        fclose(input_file);
        printf("Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        int result = vector_push_back(vector, number);
        if (!result) {
            printf("Error: Memory allocation failed\n");
            break;
        }
    }
    fclose(input_file);

    vector_shrink_to_fit(vector);

    vector_sort(vector);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        vector_destructor(vector);
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < vector_size(vector); index++) {
        fprintf(output_file, "%d\n", vector_at(vector, index));
    }
    fclose(output_file);

    vector_delete(vector);
    return EXIT_SUCCESS;
}
