
#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP



#include <cstdint>
#include <memory>
#include <cmath>
#include <unordered_set>
#include "FileSystemController.hpp"
#include "../util/FStreamWrapper.hpp"

#define INCORRECT_NUM_PARAMS std::string("Error, incorrect number of parameters for command ")

class SuperBlock;

class FileSystem {
      static constexpr uint16_t MAX_DISK_SIZE_GB = 256; // vic by asi windows / linux nezvladl

      static const std::unordered_set<std::string> mountedCommands;

      std::unique_ptr<FileSystemController> fileSystemController = nullptr;

      bool isMounted = false;

    public:

      /**
       * Mount disku z FilePath
       * @param filePath cesta k souboru s diskem
       */
      void mountDisk(const std::string& filePath);

      /**
       * Formats filesystem file - creates all neccessary parts of the filesystem
       * @param filePath name of the disk (and the file as well) - default is "myfs.dat"
       * @param userSpaceSizeBytes file system size in bytes
       * @return
       */
      static void format(uint64_t userSpaceSizeBytes, const std::string& filePath);

      void execute(const std::vector<std::string>& commandWithArguments);

    private:


      static void getFSSuperBlock(uint64_t userSpaceSizeBytes, SuperBlock& superBlock);

      static void formatSpace(FStreamWrapper& fstream, uint64_t bytes, uint64_t start = 0);

      static void writeSuperBlock(FStreamWrapper& fstream, SuperBlock& block);

      /**
       * Zapise INodes
       * @param fstream
       * @param nodeCount
       * @param start
       */
      static void writeNodes(FStreamWrapper& fstream, SuperBlock& superBlock);


};

#endif //FILESYSTEM_HPP
