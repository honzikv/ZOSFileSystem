#include <iostream>
#include "FileSystem.hpp"
#include "../util/FSException.hpp"

const std::unordered_set<std::string> FileSystem::mountedCommands = {
        "format", "mount", "cp", "mv", "rm", "mkdir",
        "rmdir", "ls", "cat", "cd", "pwd", "info",
        "incp", "outcp", "load"
};

void FileSystem::mountDisk(const std::string& filePath) {
    try {
        fileSystemController = std::make_unique<FileSystemController>(filePath);
    }
    catch (FSException& ex) {
        std::cout << ex.what() << std::endl;
    }
    isMounted = true;
}


void FileSystem::execute(const std::vector<std::string>& commandWithArguments) {
    auto command = commandWithArguments[0];
    auto args = std::vector<std::string>();
    if (commandWithArguments.size() > 1) {
        args = std::vector<std::string>(commandWithArguments.begin() + 1, commandWithArguments.end());
    }

    if (!mountedCommands.contains(command)) {
        throw FSException("Unknown command, type \"help\" for list of all commands");
    } else if (!isMounted) {
        throw FSException("Error, no disk is mounted, please mount disk with \"mount\" [filepath] command");
    }

    if (command == "cp") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"cp\"");
        }
        fileSystemController->cp(args[0], args[1]);
    }

    if (command == "mv") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"mv\"");
        }
        fileSystemController->mv(args[0], args[1]);
    }

    if (command == "rm") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"rm\"");
        }
        fileSystemController->rm(args[0]);
    }

    if (command == "mkdir") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"mkdir\"");
        }
        fileSystemController->mkdir(args[0]);
    }

    if (command == "rmdir") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"\"");
        }
        fileSystemController->rmdir(args[0]);
    }

    if (command == "ls") {
        if (args.size() > 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"ls\"");
        }

        if (args.empty()) {
            fileSystemController->ls("");
        }
        else {
            fileSystemController->ls(std::string(args[0]);
        }
    }

    if (command == "cat") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"cat\"");
        }
        ioManager->cat(args[0]);
    }

    if (command == "cd") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"cd\"");
        }
        ioManager->cd(args[0]);
    }

    if (command == "pwd") {
        if (!args.empty()) {
            throw FSException(std::string("Parameters present for command ") + "\"pwd\"");
        }
        ioManager->pwd();
    }

    if (command == "info") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"info\"");
        }
        ioManager->info(args[0]);
    }

    if (command == "incp") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"incp\"");
        }
        ioManager->incp(args[0], args[1]);
    }

    if (command == "outcp") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"outcp\"");
        }
        ioManager->outcp(args[0], args[1]);
    }

}