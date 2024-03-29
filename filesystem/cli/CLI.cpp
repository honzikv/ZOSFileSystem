#include <iostream>
#include <iterator>
#include "CLI.hpp"


CLI::CLI(std::string filePath) : filePath(std::move(filePath)) {
}

void CLI::run() {
    std::cout << "Virtual File System Application" << std::endl;
    std::cout << "Type \"format\" to format disk" << std::endl;
    std::cout << "Type \"help\" to list all commands" << std::endl << std::endl;

    auto fileStream = FileStream(filePath);
    fileSystem = std::make_unique<FileSystem>(fileStream);

    auto input = std::string();
    while (running) {
        std::cout << "#:";
        std::getline(std::cin, input);
        try {
            // pokud je vstup prazdny nebo pouze mezery pokracujeme (jinak by program spadl)
            if (input.empty() || input.find_first_not_of(' ') == std::string::npos) {
                continue;
            } else {
                auto tokens = splitByWhitespace(input);

                // prikaz na lowercase - uzivatel tedy muze pouzivat napr. i LS nebo lS ...
                std::transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
                input = tokens[0];

                if (input == "help") {
                    printHelp(); // vytisknuti napovedy
                } else if (input == "exit" || input == "q" || input == "quit" || input == "c90") {
                    running = false; // ukonci program
                } else if (input == "load") {
                    executeScript(tokens); // provede skript ze souboru
                } else {
                    execute(tokens); // jinak provede primo prikaz
                }
            }
        }
        catch (FSException& ex) {
            // jakoukoliv exception co zadna cast programu neosetri vypiseme do konzole
            std::cout << ex.what() << std::endl;
        }

    }

    exit(EXIT_SUCCESS); // pri q | quit | exit
}


void CLI::executeScript(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        std::cout << INCORRECT_NUM_PARAMS + "\"load\"" << std::endl;
        return;
    }

    auto fileName = tokens[1];
    auto fileStream = std::ifstream(fileName);

    auto commands = std::vector<std::string>(); // vsechny prikazy

    auto line = std::string(); // radka
    while (std::getline(fileStream, line)) { // iterace pres radky
        commands.push_back(line);
    }

    for (auto& command : commands) { // provadeni prikazu
        auto commandTokens = splitByWhitespace(command);
        if (commandTokens.empty()) {
            continue;
        }
        execute(commandTokens);
    }
}


void CLI::execute(const std::vector<std::string>& tokens) {
    try {
        fileSystem->execute(tokens); // predani prikazu "file systemu"
    }
    catch (FSException& exception) {
        std::cout << exception.what() << std::endl;
    }
}

void CLI::printHelp() {
    std::cout << "help - print help" << std::endl;
    std::cout
            << "format [filename] [size][GB|MB|kB] - create new disk file with size in kilobytes / megabytes / gigabytes"
            << std::endl;
    std::cout << "mount [filename] - mount specified file as a disk" << std::endl;
    std::cout << "cp [file] [path] - copy file to the specified path" << std::endl;
    std::cout << "mv [file] [path] - move file to the specified path or rename file if path contains only filename"
              << std::endl;
    std::cout << "rm [file] - remove file from current directory" << std::endl;
    std::cout << "mkdir [dirname] - create new directory dirname in current path" << std::endl;
    std::cout << "rmdir [dirname] - remove specified directory" << std::endl;
    std::cout << "ls [|dirname] - lists either current directory or dirname (if dirname is a directory)"
              << std::endl;
    std::cout << "cat [filename] - reads characters of specified filename" << std::endl;
    std::cout << "cd [target] - move to target directory" << std::endl;
    std::cout << "pwd - prints current path" << std::endl;
    std::cout << "info [file|dir] - prints info about file or directory" << std::endl;
    std::cout << "incp [file] [target] - copy file in external drive of the disk to the specified target"
              << std::endl;
    std::cout << "outcp [file] [target] - copy file in disk to the target path on external drive" << std::endl;
    std::cout << "load [script] - load script with commands and execute them sequentially" << std::endl;
    std::cout << "ln [fileA] [fileB] - create hard link of fileA with path and name of fileB" << std::endl;
}


std::vector<std::string> CLI::splitByWhitespace(const std::string& input) {
    auto stringStream = std::istringstream(input);
    return std::vector<std::string>
            {std::istream_iterator<std::string>(stringStream), std::istream_iterator<std::string>()};
}


