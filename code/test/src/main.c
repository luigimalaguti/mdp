#include <stdio.h>

int main(int argc, char **argv) {
    printf("Running test C program.... \n");
    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            printf("Argv[%d] = %s\n", i, argv[i]);
        }
    }

    return 0;
}
