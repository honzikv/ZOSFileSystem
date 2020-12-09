
#ifndef INODEIO_HPP
#define INODEIO_HPP


#include "FileSystemController.hpp"

class INodeIO {

      FileStream& fstream;

      FileSystemController& fileSystemController; // reference pro callbacky

    public:

      INodeIO(FileStream& fstream, FileSystemController& fileSystemController);

      void append(INode& node, FolderItem& folderItem);

      std::pair<std::vector<INode>, std::vector<std::string>> getItems(INode& node);

    private:
      void readFromBlockAddress(std::vector<FolderItem>& folderItems, uint64_t address);
};


#endif //INODEIO_HPP
