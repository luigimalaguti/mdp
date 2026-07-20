#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    (void)input_filename;
    (void)output_filename;

    return EXIT_SUCCESS;
}
