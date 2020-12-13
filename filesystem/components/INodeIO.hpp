
#ifndef INODEIO_HPP
#define INODEIO_HPP


#include "FileSystemController.hpp"

class INodeIO {

      FileStream& fileStream;

      FileSystemController& fileSystemController; // reference pro callbacky

    public:

      INodeIO(FileStream& fileStream, FileSystemController& fileSystemController);

      void append(INode& node, FolderItem& folderItem, bool increaseRefCount = true);

      std::vector<FolderItem> getFolderItems(INode& node);

      /**
       * Vytvori "." a ".." reference pro danou INode
       * @param current INode pro kterou se reference vytvori
       * @param parent slozka ve ktere se slozka vyskytuje
       */
      void linkFolderToParent(INode& current, uint64_t currentNodeAddress, uint64_t parentNodeAddress);

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