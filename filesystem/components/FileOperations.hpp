
#ifndef FILEOPERATIONS_HPP
#define FILEOPERATIONS_HPP


#include <string>
#include "PathContext.hpp"

/**
 * Trida pro "sofistikovane" operace se soubory
 */
class FileOperations {
      friend class PathContext;

      std::unique_ptr<PathContext> pathContext;

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

      void removeDirectory(const std::string& path);

      void removeFile(const std::string& path);

      void copyIntoFileSystem(const std::string& outPath, const std::string& path);

      void readFile(const std::string& path);

      void exportFromFileSystem(const std::string& path, const std::string& exportPath);

      /**
       * Zmeni aktualni adresar
       * @param path
       */
      void changeDirectory(const std::string& path);

      std::string getFolderName(uint64_t nodeAddress, const std::vector<FolderItem>& vector);

      void printCurrentPath();

      void getInfo(const std::string& path);

    private:
      INode getRoot();

      std::vector<FolderItem> getFolderItems(INode& node);

      INode getINodeFromAddress(uint64_t address);

      INode getUpdatedINode(INode& node);

      /**
       * Vrati zpet stav PathContext objektu pred provedenim operace
       * @param absolutePath
       */
      void restorePathContextState(const std::vector<INode>& absolutePath);

};


#endif //FILEOPERATIONS_HPP
