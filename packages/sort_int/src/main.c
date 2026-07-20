#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int compare_int32(const void *pointer_a, const void *pointer_b) {
    int32_t number_a = *(int32_t *)pointer_a;
    int32_t number_b = *(int32_t *)pointer_b;
    return (number_a > number_b) - (number_a < number_b);
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

    size_t size = 0;
    size_t capacity = 0;
    int32_t *data = NULL;

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        if (size >= capacity) {
            size_t temp_capacity = capacity == 0 ? 1 : capacity * 2;
            int32_t *temp_data = realloc(data, temp_capacity * sizeof(int32_t));
            data = temp_data;
            capacity = temp_capacity;
        }
        data[size] = number;
        size++;
    }
    fclose(input_file);

    qsort(data, size, sizeof(int32_t), compare_int32);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        free(data);
        printf("Error: Could not open file %s\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < size; index++) {
        fprintf(output_file, "%d\n", data[index]);
    }
    fclose(output_file);

    free(data);
    return EXIT_SUCCESS;
}
