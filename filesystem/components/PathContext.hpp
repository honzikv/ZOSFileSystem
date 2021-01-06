
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

      FileOperations& fileOperations; // reference na praci se soubory pro volani nekterych funkci

    public:
      explicit PathContext(FileOperations& fileSystemController);

      /**
       * Provede refresh - to se provadi po urcitych operacich aby byla zajistena konsistence se souborem.
       * Nacte znovu vsechny INodes v ceste a nacte predmety
       */
      void refresh();

      /**
       * Ziska index predmetu ve slozce podle jeho jmena
       * @param folderItemName jmeno predmetu
       * @return index, pokud predmet existuje, jinak -1
       */
      int getFolderItemIndex(std::string& folderItemName);

      /**
       * Presune se do korenoveho adresare (root)
       * @param fetchFolderItems zda-li ma nacist predmety adresare
       */
      void moveToRoot(bool fetchFolderItems = false);

      /**
       * Presune se do dane cesty a nebo vyhodi exception, pokud cesta neexistuje
       * @param path dana cesta
       */
      void moveTo(FileSystemPath& path);

      /**
       * Nacte predmety v aktualni slozce
       */
      void loadItems();

};


#endif //PATHCONTEXT_HPP
