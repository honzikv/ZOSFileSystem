
#ifndef INODEIO_HPP
#define INODEIO_HPP


#include "FileSystemController.hpp"

class INodeIO {

      FileStream& fileStream;

      FileSystemController& fileSystemController; // reference pro callbacky

    public:

      INodeIO(FileStream& fileStream, FileSystemController& fileSystemController);

      void append(INode& node, FolderItem& folderItem);

      std::vector<FolderItem> getFolderItems(INode& node);

    private:
      void readFromBlockAddress(std::vector<FolderItem>& folderItems, uint64_t address);

      void readNFolderItems(std::vector<FolderItem>& folderItems, uint64_t blockAddress, uint32_t count);


      void appendToT1Block(uint64_t itemPosition, uint64_t t1Address, std::vector<uint64_t> allocations,
                           FolderItem& folderItem);

      void
      appendToT2Block(uint32_t itemPosition, uint64_t t2Address, std::vector<uint64_t> allocations, FolderItem& folderItem);


      void readFromDirectBlocks(std::vector<uint64_t> addressList, uint32_t count, std::vector<FolderItem>& result);

      void readFromT1Address(uint64_t t1Address, uint64_t count, std::vector<FolderItem> result);

      void readFromT2Address(uint64_t t2Address, uint32_t itemCount, std::vector<FolderItem> result);
};

#endif //INODEIO_HPP