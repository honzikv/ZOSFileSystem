
#ifndef INODEIO_HPP
#define INODEIO_HPP


#include "FileSystemController.hpp"

class INodeIO {

      FileStream& fileStream;

      FileSystemController& fileSystemController; // reference pro callbacky

    public:

      INodeIO(FileStream& fileStream, FileSystemController& fileSystemController);

      void appendFolderItem(INode& node, FolderItem& folderItem, bool increaseRefCount = true);

      void appendFile(INode& parent, INode& node, FolderItem& folderItem, FileStream& externalFileStream);

      std::vector<FolderItem> getFolderItems(INode& node);

      /**
       * Vytvori "." a ".." reference pro danou INode
       * @param current INode pro kterou se reference vytvori
       * @param parent slozka ve ktere se slozka vyskytuje
       */
      void linkFolderToParent(INode& current, uint64_t currentNodeAddress, uint64_t parentNodeAddress);

      void printINodeInfo(INode& node);

      void removeFolderItem(INode& node, FolderItem& folderItem);

      void readFile(INode& node);

    private:
      void readFromBlockAddress(std::vector<FolderItem>& folderItems, uint64_t address);

      void readNFolderItems(std::vector<FolderItem>& folderItems, uint64_t blockAddress, uint32_t count);


      void appendToT1Block(uint64_t itemPosition, uint64_t t1Address, std::vector<uint64_t> allocations,
                           FolderItem& folderItem);

      void
      appendToT2Block(uint32_t itemPosition, uint64_t t2Address, std::vector<uint64_t> allocations,
                      FolderItem& folderItem);


      void readFromDirectBlocks(std::vector<uint64_t> addressList, uint32_t count, std::vector<FolderItem>& result);

      void readFromT1Address(uint64_t t1Address, uint64_t count, std::vector<FolderItem> result);

      void readFromT2Address(uint64_t t2Address, uint32_t itemCount, const std::vector<FolderItem>& result);

      std::vector<uint64_t> getINodeBlocks(INode& node);

      void removeLast(INode& node);

      void writeAt(INode& node, uint32_t index, FolderItem& folderItem);

      uint64_t getExtraBlocks(uint64_t bytes);

      static void printBuffer(std::vector<char> vector);
};

#endif //INODEIO_HPP