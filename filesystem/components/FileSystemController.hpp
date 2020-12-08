
#ifndef FILESYSTEMCONTROLLER_HPP
#define FILESYSTEMCONTROLLER_HPP

#include <string>
#include "../util/FStreamWrapper.hpp"
#include "MemoryAllocator.hpp"
#include "PathInfo.hpp"
#include "../io/AddressType.h"

class INodeIO;

/**
 * Trida, ktera slouzi pro ovladani filesystemu
 */

class FileSystemController {

      FStreamWrapper fstream;

      std::shared_ptr<SuperBlock> superBlock;

      std::shared_ptr<MemoryAllocator> memoryAllocator;

      std::shared_ptr<INodeIO> nodeIO;

      std::shared_ptr<PathInfo> pathInfo;

    public:

      explicit FileSystemController(const std::string& fileName);

      void cp(const std::string& file, const std::string& path);

      void mv(const std::string& file, const std::string& path);

      void rm(const std::string& file);

      void mkdir(const std::string& dirName);

      void rmdir(const std::string& dirName);

      void ls(const std::string& path);

      void cat(const std::string& file);

      void cd(const std::string& path);

      void pwd();

      void info(const std::string& file);

      void incp(const std::string& fileOut, const std::string& fileFS);

      void outcp(const std::string& fileIn, const std::string& fileOut);

      uint64_t nextBlock(AddressType type);

      void reclaimMemory(std::vector<uint64_t>& memoryBlocks);

      void update(INode& node);
};


#endif //FILESYSTEMCONTROLLER_HPP
