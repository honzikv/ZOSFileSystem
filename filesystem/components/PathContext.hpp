
#ifndef PATHCONTEXT_HPP
#define PATHCONTEXT_HPP


#include <util/FileSystemPath.h>
#include "FileSystemController.hpp"

/**
 * Objekt, ktery slouzi pro zpracovavani navigace ve filesystemu. Tzn umi zmenit aktualni adresar, obsahuje
 * informace o aktualnim adresari, souborech apod.
 */
class PathContext {

      std::vector<INode> absolutePath; // absolutni cesta od rootu, kdy prvni INode je root

      std::vector<FolderItem> folderItems; // folder item objekty pro posledni INode v absolute path

      FileSystemController& fileSystemController; // reference na filesystem controller

    public:
      explicit PathContext(FileSystemController& fileSystemController);

      void initialize();

      void update(INode& node);

      void listItems(const std::string& path);

      int getNodeIndex(INode& node);

      void printFolderItemNames();

      bool folderItemExists(std::string& itemName);

      void printLocalFolderItem(FileSystemPath& path);

      void printRelativePath(FileSystemPath& path);

      void printAbsolutePath(FileSystemPath& path);

      void moveTo(std::string& folderItemName);

      int getFolderItemIndex(std::string& folderItemName);
};


#endif //PATHCONTEXT_HPP
