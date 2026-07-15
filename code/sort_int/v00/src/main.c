#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * ==================================================
 * COMPARE FUNCTION
 * ==================================================
 */
int compare_int32(const void *pointer_a, const void *pointer_b) {
    int32_t number_a = *(int32_t *)pointer_a;
    int32_t number_b = *(int32_t *)pointer_b;
    return (number_a > number_b) - (number_a < number_b);
}

int main(int argc, char **argv) {
    /*
     * ==================================================
     * CHECK ARGUMENTS
     * ==================================================
     */
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    /*
     * ==================================================
     * OPENING INPUT FILE
     * ==================================================
     */
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    int32_t *vector = NULL;
    size_t size = 0;
    size_t capacity = 0;

    /*
     * ==================================================
     * READING INPUT FILE
     * ==================================================
     */
    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        // Resize the vector if necessary
        if (size >= capacity) {
            size_t temp_capacity = (capacity == 0) ? 1 : capacity * 2;
            int32_t *temp_vector = realloc(vector, temp_capacity * sizeof(*vector));
            if (temp_vector == NULL) {
                printf("Error: Memory allocation failed\n");
                break;
            }
            vector = temp_vector;
            capacity = temp_capacity;
        }
        // Store the number in the vector
        vector[size] = number;
        size++;
    }
    fclose(input_file);

    /*
     * ==================================================
     * SHRINKING VECTOR TO FIT SIZE
     * ==================================================
     */
    if (size < capacity) {
        int32_t *temp_vector = realloc(vector, size * sizeof(*vector));
        if (temp_vector != NULL) {
            vector = temp_vector;
            capacity = size;
        }
    }

    /*
     * ==================================================
     * SORTING VECTOR
     * ==================================================
     */
    qsort(vector, size, sizeof(*vector), compare_int32);

    /*
     * ==================================================
     * OPENING OUTPUT FILE
     * ==================================================
     */
    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        free(vector);
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    /*
     * ==================================================
     * WRITING OUTPUT FILE
     * ==================================================
     */
    for (size_t index = 0; index < size; index++) {
        fprintf(output_file, "%d\n", vector[index]);
    }
    fclose(output_file);

    free(vector);
    return EXIT_SUCCESS;
}
