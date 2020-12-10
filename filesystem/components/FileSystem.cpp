#include <iostream>
#include "FileSystem.hpp"
#include "../util/StringParsing.hpp"

void FileSystem::execute(const std::vector<std::string>& commandWithArguments) {
    auto command = commandWithArguments[0];
    auto args = std::vector<std::string>();
    if (commandWithArguments.size() > 1) {
        args = std::vector<std::string>(commandWithArguments.begin() + 1, commandWithArguments.end());
    }

    if (command == "format") {
        if (args.size() > 2 || args.empty()) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"format\"");
        }
        format(StringParsing::getSizeBytes(args), filePath);
    } else if (!mountedCommands.contains(command)) {
        throw FSException("Unknown command, type \"help\" for list of all commands");
    } else if (!isMounted) {
        throw FSException("Error unable to read disk file, this is probably due to wrong parameter or disk not having"
                          "been formatted yet, to format it use format command");
    } else if (command == "cp") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"cp\"");
        }
        fileSystemController->cp(args[0], args[1]);
    } else if (command == "mv") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"mv\"");
        }
        fileSystemController->mv(args[0], args[1]);
    } else if (command == "rm") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"rm\"");
        }
        fileSystemController->rm(args[0]);
    } else if (command == "mkdir") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"mkdir\"");
        }
        fileSystemController->mkdir(args[0]);
    } else if (command == "rmdir") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"\"");
        }
        fileSystemController->rmdir(args[0]);
    } else if (command == "ls") {
        if (args.size() > 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"ls\"");
        }

        if (args.empty()) {
            fileSystemController->ls("");
        } else {
            fileSystemController->ls(std::string(args[0]));
        }
    } else if (command == "cat") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"cat\"");
        }
        fileSystemController->cat(args[0]);
    } else if (command == "cd") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"cd\"");
        }
        fileSystemController->cd(args[0]);
    } else if (command == "pwd") {
        if (!args.empty()) {
            throw FSException(std::string("Parameters present for command ") + "\"pwd\"");
        }
        fileSystemController->pwd();
    } else if (command == "info") {
        if (args.size() != 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"info\"");
        }
        fileSystemController->info(args[0]);
    } else if (command == "incp") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"incp\"");
        }
        fileSystemController->incp(args[0], args[1]);
    } else if (command == "outcp") {
        if (args.size() != 2) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"outcp\"");
        }
        fileSystemController->outcp(args[0], args[1]);
    }

}

void FileSystem::format(uint64_t userSpaceSizeBytes, const std::string& filePath) {
    auto superBlock = SuperBlock(userSpaceSizeBytes);
    fileStream.formatSpace(superBlock.totalSize);
    fileStream.moveTo(0);
    fileStream.writeSuperBlock(superBlock);

    fileStream.moveTo(superBlock.nodeAddress);
    auto rootNode = INode(true, 0);
    fileStream << rootNode;
    auto emptyNode = INode();
    for (auto i = 1; i < superBlock.nodeCount; i++) {
        fileStream << rootNode;
    }

    fileStream.moveTo(0);
    fileSystemController = std::make_shared<FileSystemController>(fileStream);
    std::cout << "controller complete" << std::endl;
}

FileSystem::FileSystem(FileStream& fstream) : fileStream(fstream) {

    try {
        fileSystemController = std::make_shared<FileSystemController>(fileStream);
        std::cout << fileStream.good();
    }
    catch (FSException& exception) {
        std::cout << "Error file is not formatted yet, please format it with the \"format\" command" << std::endl;
    }
}
