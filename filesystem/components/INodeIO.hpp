
#ifndef INODEIO_HPP
#define INODEIO_HPP


#include "../util/FStreamWrapper.hpp"
#include "FileSystemController.hpp"

class INodeIO {

      FStreamWrapper& fstream; // reference pro cteni

      FileSystemController& fileSystemController; // reference pro callbacky

    public:

      INodeIO(FStreamWrapper& fstream, FileSystemController& fileSystemController);

      void append(INode& node, FolderItem& folderItem);

      std::pair<std::vector<INode>, std::vector<std::string>> getItems(INode& node);

    private:
      void readFromBlockAddress(std::vector<FolderItem>& folderItems, uint64_t address);
};


#endif //INODEIO_HPP
