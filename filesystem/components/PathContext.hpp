
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

      FileOperations& fileOperations;

    public:
      explicit PathContext(FileOperations& fileSystemController);

      void refresh();

      bool folderItemExists(std::string& itemName);

      int getFolderItemIndex(std::string& folderItemName);

      void moveToRoot(bool fetchFolderItems = false);

      /**
       * Presune se do dane cesty a nebo vyhodi exception, pokud cesta neexistuje
       * @param path dana cesta
       */
      void moveTo(FileSystemPath& path);

      void loadItems();

};


#endif //PATHCONTEXT_HPP
