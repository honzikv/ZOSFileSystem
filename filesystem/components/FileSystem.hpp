
#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP


#include <cstdint>
#include <memory>
#include <cmath>
#include <unordered_set>
#include "FileSystemController.hpp"
#include "../util/FileStream.hpp"

#define INCORRECT_NUM_PARAMS std::string("Error, incorrect number of parameters for command ")

class SuperBlock;

class FileSystem {
      static constexpr uint16_t MAX_DISK_SIZE_GB = 8; // vic by asi windows / linux nezvladl

      FileStream& fileStream;

      std::string filePath;

      std::unordered_set<std::string> mountedCommands {
              "format", "mount", "cp", "mv", "rm", "mkdir",
              "rmdir", "ls", "cat", "cd", "pwd", "info",
              "incp", "outcp", "load",
              // debug
              "diskinfo"
      };

      std::shared_ptr<FileSystemController> fileSystemController = nullptr;

      bool isMounted = false;

    public:

      explicit FileSystem(FileStream& fstream);

/**
       * Formats filesystem file - creates all neccessary parts of the filesystem
       * @param filePath name of the disk (and the file as well) - default is "myfs.dat"
       * @param userSpaceSizeBytes file system size in bytes
       * @return
       */
      void format(std::vector<std::string>& args);

      void execute(const std::vector<std::string>& commandWithArguments);

};

#endif //FILESYSTEM_HPP
