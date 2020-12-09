#include <iostream>
#include <unordered_set>
#include "filesystem/cli/CLI.hpp"
#include <filesystem>


int main(int argc, char* argv[]) {
//    if (argc == 1) {
//        std::cout << "Error no filename present, please restart the program"
//                  << std::endl;
//        exit(EXIT_FAILURE);
//    }
//
//    auto fileName = std::string(argv[1]);
//    auto cli = CLI(fileName);
//
//
//    cli.run();

    auto fileStream = FileStream("hello.bin");
    fileStream.formatSpace(4096);
    auto bool1 = true;
    fileStream.moveTo(0);
    fileStream.write(bool1);

    fileStream.moveTo(250);
    fileStream.write(bool1);

    fileStream.moveTo(4);
    auto uint = uint64_t(234);
    fileStream.write(uint);

    auto readbool = false;
    auto readbool2 = false;
    auto readuint = uint64_t {};
    fileStream.moveTo(0);
    fileStream.read(readbool);
    fileStream.moveTo(250);
    fileStream.read(readbool2);
    fileStream.moveTo(4);
    fileStream.read(readuint);

    std::cout << readbool << " " << readbool2 << " " << readuint << " " << std::endl;

}

