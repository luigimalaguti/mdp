#include "greeter.hpp"
#include "mathutil.h"

#include <print>

int main(int argc, char **argv) {
    std::println("Running test mixed C/C++ program...");

    Greeter greeter("Luigi");
    std::println("{}", greeter.greet());

    std::println("add(2, 3) = {}", add(2, 3));
    std::println("mul(4, 5) = {}", mul(4, 5));

    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            std::println("Argv[{}] = {}", i, argv[i]);
        }
    }

    return 0;
}
