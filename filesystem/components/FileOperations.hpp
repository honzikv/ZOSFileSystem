
#ifndef FILEOPERATIONS_HPP
#define FILEOPERATIONS_HPP


#include <string>
#include "PathContext.hpp"

/**
 * Trida pro "sofistikovane" operace se soubory
 */
class FileOperations {

      PathContext pathContext;

      FileSystemController& fileSystemController;

    public:

      explicit FileOperations(FileSystemController& fileSystemController);

      /**
       * Vytiskne jmena vsech predmetu v aktualni slozce
       */
      void printCurrentFolderItems();

      /**
       * Vytiskne jmena vsech predmetu ve slozce v dane ceste
       * @param path cesta
       */
      void listItems(const std::string& path);

      /**
       * Vytvori slozku v dane ceste pokud je mozno
       * @param path cesta
       */
      void makeDirectory(const std::string& path);

      /**
       * Zmeni aktualni adresar
       * @param path
       */
      void changeDirectory(const std::string& path);

      std::string getFolderName(uint64_t nodeAddress, const std::vector<FolderItem>& vector);

      void printCurrentPath();
};


#endif //FILEOPERATIONS_HPP
