
#ifndef FILEOPERATIONS_HPP
#define FILEOPERATIONS_HPP


#include <string>
#include "PathContext.hpp"

/**
 * Trida pro "sofistikovane" operace se soubory
 */
class FileOperations {
      friend class PathContext; // PathContext muze vyuzivat tridu bez pristupovych prav

      std::unique_ptr<PathContext> pathContext; // kontext, kde se aktualne uzivatel nachazi

      FileSystemController& fileSystemController; // reference na FileSystemController pro zavolani operaci

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
       * Odstrani adresar
       * @param path cesta k adresari
       */
      void removeDirectory(const std::string& path);

      /**
       * Odstrani soubor
       * @param path  cesta k souboru
       */
      void removeFile(const std::string& path);

      /**
       * Zkopiruje soubor z externi cesty do interni cesty
       * @param outPath externi cesta
       * @param path cesta ve filesystemu
       */
      void copyIntoFileSystem(const std::string& outPath, const std::string& path);

      /**
       * Precte (cat) soubor v dane ceste
       * @param path
       */
      void readFile(const std::string& path);

      /**
       * Exportuje z interniho file systemu na danou cestu
       * @param path cesta ve filesystemu
       * @param exportPath cesta v OS
       */
      void exportFromFileSystem(const std::string& path, const std::string& exportPath);

      /**
       * Zmeni aktualni adresar
       * @param path cesta, na kterou se ma aktualni adresar zmenit
       */
      void changeDirectory(const std::string& path);

      /**
       * Ziska nazev slozky
       * @param nodeAddress adresa slozky
       * @param vector
       * @return
       */
      static std::string getFolderName(uint64_t nodeAddress, const std::vector<FolderItem>& vector);

      /**
       * Vytiskne aktualni adresar, ve kterem se uzivatel nachazi
       */
      void printCurrentPath();

      /**
       * Ziska informaci o INode daneho souboru
       * @param path cesta k souboru
       */
      void getInfo(const std::string& path);

      /**
       * Presune soubor z cesty fileSource na cestu fileDest
       * @param fileSource puvodni cesta
       * @param fileDest cilova cesta
       * @param deleteFromSource zda-li se ma odstranit z puvodni cesty - pro hardnlink se nastavi na false, jinak true
       */
      void moveFile(const std::string& fileSource, const std::string& fileDest, bool deleteFromSource);

      /**
       * Zkopiruje soubor z cesty fileSource do cesty fileDest
       * @param fileSource puvodni cesta
       * @param fileDest cilova cesta
       */
      void copyFile(const std::string& fileSource, const std::string& fileDest);

    private:
      /**
       * Vrati root
       * @return root
       */
      INode getRoot();

      /**
       * Ziska predmety dane INode
       * @param node INode, jejiz predmety chceme ziskat
       * @return vektor FolderItem objektu, ktere patri dane INode
       */
      std::vector<FolderItem> getFolderItems(INode& node);

      /**
       * Ziska INode z dane adresy
       * @param address adresa INode
       * @return INode z adresy
       */
      INode getINodeFromAddress(uint64_t address);

      /**
       * Ziska aktualizovanou INode
       * @param node node z predchozi operace
       * @return aktualizovanou INode
       */
      INode getUpdatedINode(INode& node);

      /**
       * Vrati zpet stav PathContext objektu pred provedenim operace
       * @param absolutePath
       */
      void restorePathContextState(const std::vector<INode>& absolutePath);
};


#endif //FILEOPERATIONS_HPP
