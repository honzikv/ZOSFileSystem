#include <iostream>
#include "FileSystem.hpp"
#include "../util/StringParsing.hpp"
#include "PathContext.hpp"

FileSystem::FileSystem(FileStream& fstream) : fileStream(fstream) {
    fileSystemController = std::make_shared<FileSystemController>(fileStream);

    if (fileSystemController->getDriveState() == DriveState::Valid) {
        std::cout << "FileSystem is usable" << std::endl;
        isMounted = true;
    } else if (fileSystemController->getDriveState() == DriveState::Invalid) {
        std::cout << "Error, disk is either corrupt or incorrect parameter present.\n "
                     "File is not empty but does not contain super block."
                  << std::endl;
    } else {
        std::cout << "File is either empty or not created, please format it with the \"format\" command" << std::endl;
    }
}

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
        format(args);
    } else if (!mountedCommands.contains(command)) {
        throw FSException("Unknown command, type \"help\" for list of all commands");
    } else if (!isMounted) {
        throw FSException("Error unable to read disk file, this is probably due to wrong parameter or disk not having"
                          " been formatted yet, to format it use format command");
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
        if (args.size() > 1) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"info\"");
        }
        if (args.empty()) {
            fileSystemController->info("");
        } else {
            fileSystemController->info(args[0]);
        }
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
    } else if (command == "diskinfo") {
        if (!args.empty()) {
            throw FSException(INCORRECT_NUM_PARAMS + "\"diskinfo\"");
        }
        fileSystemController->diskInfo();
    }

}

void FileSystem::format(std::vector<std::string>& args) {
    auto string = std::string();
    for (const auto& arg : args) {
        string += arg;
    }
    std::transform(string.begin(), string.end(), string.begin(),
                   [](auto c) { return std::tolower(c); }); // to lower
    if (!StringParsing::matchesFormatRegex(string)) {
        throw FSException("Incorrect parameters for \"format\"");
    }

    uint64_t userSpaceSizeBytes;
    try {
        userSpaceSizeBytes = StringParsing::parseDriveFormatSize(string);
    }
    catch (FSException& ex) {
        throw FSException("Incorrect parameters for \"format\"");
    }

    fileStream.deleteFile(); // smazeme predchozi soubor
    fileStream.createFile(); // vytvorime prazdny soubor
    fileStream.open(); // otevreme fstream

    auto superBlock = SuperBlock(userSpaceSizeBytes);
    fileStream.moveTo(0);
    fileStream.format(superBlock.totalSize);
    fileStream.moveTo(0);
    fileStream.writeSuperBlock(superBlock);
    fileSystemController->initDrive();
    isMounted = true;
}
