
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

      void refresh();

      bool folderItemExists(std::string& itemName);

      int getFolderItemIndex(std::string& folderItemName);

      void moveToRoot(bool fetchFolderItems = false);

      void moveToPath(FileSystemPath& path);

      void loadItems();
};


#endif //PATHCONTEXT_HPP
