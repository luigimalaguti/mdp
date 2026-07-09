#include <print>

int main(int argc, char **argv) {
    std::println("Running test C++ program...");
    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            std::println("Argv[{}] = {}", i, argv[i]);
        }
    }

    return 0;
}
