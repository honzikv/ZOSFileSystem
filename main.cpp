#include <iostream>
#include <unordered_set>
#include "filesystem/cli/CLI.hpp"
#include <filesystem>


int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Error no filename present, please restart the program"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    auto fileName = std::string(argv[1]);
    CLI(fileName).run();

}

