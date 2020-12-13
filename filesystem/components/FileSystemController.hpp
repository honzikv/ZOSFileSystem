
#ifndef FILESYSTEMCONTROLLER_HPP
#define FILESYSTEMCONTROLLER_HPP

#include <string>
#include "MemoryAllocator.hpp"
#include "../io/AddressType.h"
#include "../util/FileStream.hpp"

class PathContext;

class INodeIO;

/**
 * Stav disku (souboru)
 */
enum class DriveState {
      NonExistent, // soubor (cesta) neexistuje
      Valid, // soubor obsahuje validni super block
      Empty, // soubor je prazdny
      Invalid // v souboru neni od adresy 0 super block
};

/**
 * Trida, ktera slouzi pro ovladani filesystemu
 */
class FileSystemController {

      FileStream& fileStream; // reference na filestream ziskana z FileSystem tridy

      DriveState driveState = DriveState::Empty;

      std::shared_ptr<SuperBlock> superBlock;

      std::shared_ptr<MemoryAllocator> memoryAllocator;

      std::shared_ptr<INodeIO> nodeIO;

      std::shared_ptr<PathContext> pathContext;

    public:

      [[nodiscard]] DriveState getDriveState() const;

      explicit FileSystemController(FileStream& fileStream);

      void initDrive();

      void diskInfo();

      INode getRoot();

      void cp(const std::string& file, const std::string& path);

      void mv(const std::string& file, const std::string& path);

      void rm(const std::string& file);

      void mkdir(const std::string& path);

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

      void refresh(INode& node);

      void writeINode(INode& node);

      std::vector<FolderItem> getFolderItems(INode& node);

      INode getFolderItemINode(uint64_t nodeAddress);

      uint64_t getNodeAddress(INode& node);

      INode getFreeINode();

      void addItem(INode& parent, FolderItem child);

      void reclaimINode(INode& node);

      void createINodes();

      INode getUpdatedINode(INode& node);
};


#endif //FILESYSTEMCONTROLLER_HPP
