
#ifndef PATHCONTEXT_HPP
#define PATHCONTEXT_HPP


#include <util/FileSystemPath.h>
#include "FileSystemController.hpp"

/**
 * Objekt, ktery slouzi pro zpracovavani navigace ve filesystemu. Tzn umi zmenit aktualni adresar, obsahuje
 * informace o aktualnim adresari, souborech apod.
 */
class PathContext {
      friend class FileOperations;

      std::vector<INode> absolutePath; // absolutni cesta od rootu, kdy prvni INode je root

      std::vector<FolderItem> folderItems; // folder item objekty pro posledni INode v absolute path

      FileSystemController& fileSystemController; // reference na filesystem controller

    public:
      explicit PathContext(FileSystemController& fileSystemController);

      void initialize();

      void update(INode& node);

      void refresh();

      void listItems(const std::string& path);

      int getNodeIndex(INode& node);

      void printFolderItemNames();

      bool folderItemExists(std::string& itemName);

      void printCurrentFolder();

      void printRelativePath(FileSystemPath& path);

      void printAbsolutePath(FileSystemPath& path);

      void moveTo(std::string& folderItemName, bool fetchFolderItems);

      int getFolderItemIndex(std::string& folderItemName);

      std::string getItemName(const std::vector<FolderItem>& parentFolderItems, INode& node);

      void makeFolder(const std::string& path);

      void moveToRoot(bool fetchFolderItems = false);

      void moveToPath(FileSystemPath& path);

      void loadItems();
};


#endif //PATHCONTEXT_HPP
