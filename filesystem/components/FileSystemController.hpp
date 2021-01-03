
#ifndef FILESYSTEMCONTROLLER_HPP
#define FILESYSTEMCONTROLLER_HPP

#include <string>
#include "MemoryAllocator.hpp"
#include "../io/AddressType.h"
#include "../util/FileStream.hpp"

class PathContext;

class INodeIO;

class FileOperations;


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
      friend class FileSystem;

      FileStream& fileStream; // reference na filestream ziskana z FileSystem objektu

      DriveState driveState = DriveState::Empty; // stav disku

      std::shared_ptr<SuperBlock> superBlock; // reference na super blok disku
      std::shared_ptr<MemoryAllocator> memoryAllocator; // reference na memory alokator pro prirazovani pameti
      std::shared_ptr<INodeIO> nodeIO; // reference na praci s INodes
      std::shared_ptr<FileOperations> fileOperations; // reference na praci se soubory

      void initDrive(); // inicializace disku - zapis inod, bitmapy apod.

    public:

      [[nodiscard]] DriveState getDriveState() const;

      explicit FileSystemController(FileStream& fileStream);

      void diskInfo();

      INode getRoot();

      void cp(const std::string& file, const std::string& path);

      void mv(const std::string& file, const std::string& path);

      void rm(const std::string& path);

      void mkdir(const std::string& path);

      void rmdir(const std::string& path);

      void ls(const std::string& path);

      void cat(const std::string& file);

      void cd(const std::string& path);

      void pwd();

      void info(const std::string& file);

      void incp(const std::string& readPath, const std::string& path);

      void outcp(const std::string& path, const std::string& exportPath);

      void ln(const std::string& file, const std::string& path);

      void debugBlockBitmap();

      void debugNodeBitmap();

      uint64_t nextBlock(AddressType type);

      void reclaimMemory(std::vector<uint64_t>& memoryBlocks);

      void refresh(INode& node);

      std::vector<FolderItem> getFolderItems(INode& node);

      INode getINodeFromAddress(uint64_t nodeAddress);

      uint64_t getINodeAddress(INode& node);

      INode getFreeINode(bool isFolder);

      void appendFolderItem(INode& parent, FolderItem child);

      void reclaimINode(INode& node);

      void createINodes();

      INode getUpdatedINode(INode& node);

      void linkFolderToParent(INode& child, uint64_t childAddress, uint64_t parentNodeAddress);

      void printINodeInfo(INode& node, FolderItem& folderItem);

      void removeFolderItem(INode& node, FolderItem& folderItem);

      void appendExternalFile(INode& parent, INode& child, FolderItem& folderItem, FileStream& externalFileStream);

      std::vector<uint64_t> nextNBlocks(uint32_t n, AddressType addressType);

      void readFile(INode& node);

      void exportFile(INode& node, FileStream& outputFileStream);

      void copyData(INode source, INode dest);
};


#endif //FILESYSTEMCONTROLLER_HPP
