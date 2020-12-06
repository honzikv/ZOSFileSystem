
#ifndef INODEIO_HPP
#define INODEIO_HPP


#include "../util/FStreamWrapper.hpp"
#include "FileSystemController.hpp"

class INodeIO {

      FStreamWrapper& fstream; // reference pro cteni

      FileSystemController& fileSystemController; // reference pro callbacky

    public:
      void append(INode& node, FolderItem& folderItem);
};


#endif //INODEIO_HPP
