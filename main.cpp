#include <iostream>
#include <unordered_set>
#include "filesystem/cli/CLI.hpp"
#include <filesystem>

class FStreamDecorator : public std::fstream {

    public:
      FStreamDecorator& operator<<(std::ostream& os, const FStreamDecorator& decorator) {

      }
};

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Error no filename present, please restart the program"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    auto fileName = std::string(argv[1]);

    if (!std::filesystem::exists(fileName)) {
        auto fstream = FStreamWrapper(fileName); // vytvori soubor pokud neexistuje
    }
    std::cout << "Creating CLI" << std::endl;

    auto cli = CLI(fileName);


    cli.run();

}

