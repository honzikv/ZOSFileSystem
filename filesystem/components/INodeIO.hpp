
#ifndef INODEIO_HPP
#define INODEIO_HPP


#include "FileSystemController.hpp"

class INodeIO {

      FileStream& fileStream; // reference na file stream pro zapis a cteni ze souboru

      FileSystemController& fileSystemController; // reference pro callbacky

    public:

      INodeIO(FileStream& fileStream, FileSystemController& fileSystemController);

      /**
       * Prida FolderItem k dane INode a podle increaseRefCount zvysi pocet referenci - nezvysuje se pouze pro "." a ".."
       * @param node INode, ke ktere FolderItem pridavame
       * @param folderItem FolderItem, ktery chceme pridat
       * @param increaseRefCount true, pokud chceme zvysit pocet referenci INode, jinak false
       */
      void appendFolderItem(INode& node, FolderItem& folderItem, bool increaseRefCount = true);

      /**
       * Prida externi soubor do filesystemu do dane slozky (parent)
       * @param parent nadrazena slozka
       * @param child soubor, ktery ma byt ve slozce
       * @param folderItem folder item souboru
       * @param externalFileStream file stream externiho souboru
       */
      void appendExternalFile(INode& parent, INode& node, FolderItem& folderItem, FileStream& externalFileStream);

      /**
       * Ziska predmety dane INode
       * @param node INode, pro kterou chceme predmety ziskat
       * @return seznam FolderItem objektu
       */
      std::vector<FolderItem> getFolderItems(INode& node);

      /**
       * Vytvori "." a ".." reference pro danou INode
       * @param current INode pro kterou se reference vytvori
       * @param parent slozka ve ktere se slozka vyskytuje
       */
      void linkFolderToParent(INode& current, uint64_t currentNodeAddress, uint64_t parentNodeAddress);

      /**
       * Vytiskne informace o INode
       * @param node INode pro kterou vytiskne funkce informace
       * @param folderItem FolderItem pro predmet dane INode (pro nazev)
       */
      void printINodeInfo(INode& node, FolderItem& folderItem);

      /**
       * Odstrani FolderItem z dane INode
       * @param node INode, ze ktere se odstrani FolderItem
       * @param folderItem FolderItem, ktery se ma odstranit
       */
      void removeFolderItem(INode& node, FolderItem& folderItem);

      /**
       * Precteni souboru z dane INode
       * @param node INode, ze ktere se bude soubor cist
       */
      void readFile(INode& node);

      /**
       * Export souboru z INode do externiho souboru pomoci file streamu
       * @param node INode daneho souboru, ktery se bude exportovat
       * @param outputFileStream file stream pro zapis
       */
      void exportFile(INode& node, FileStream& outputFileStream);

      /**
       * Zkopirovani dat z source INode do dest INode
       * @param source zdrojova INode
       * @param dest cilova INode
       */
      void copyData(INode& source, INode& dest);

    private:

      /**
       * Prida FolderItem k do 1. neprime adresy
       * @param itemPosition pozice (index) ve slozce
       * @param t1Address 1. neprima adresa
       * @param allocations seznam s alokacemi pro dealokaci pri chybe
       * @param folderItem predmet, ktery se ma pridat
       */
      void appendToT1Block(uint64_t itemPosition, uint64_t t1Address, std::vector<uint64_t>& allocations,
                           FolderItem& folderItem);

      /**
      * Prida FolderItem k do 2. neprime adresy
      * @param itemPosition pozice (index) ve slozce
      * @param t2Address 2. neprima adresa
      * @param allocations seznam s alokacemi pro dealokaci pri chybe
      * @param folderItem predmet, ktery se ma pridat
      */
      void
      appendToT2Block(uint32_t itemPosition, uint64_t t2Address, std::vector<uint64_t>& allocations,
                      FolderItem& folderItem);

      /**
       * Precteni predmetu slozky z primych odkazu
       * @param addressList seznam adres pro precteni
       * @param count pocet k precteni
       * @param result reference na vektor s vysledky
       */
      void readFromDirectBlocks(std::vector<uint64_t>& addressList, uint32_t count, std::vector<FolderItem>& result);

      /**
       * Precteni predmetu slozky z 1. neprimeho odkazu
       * @param t1Address 1. neprimy odkaz
       * @param count pocet k precteni
       * @param result reference na vektor s vysledky
       */
      void readFromT1Address(uint64_t t1Address, uint64_t count, std::vector<FolderItem>& result);

      /**
       * Precteni predmetu slozky z 2. neprimeho odkazu
       * @param t1Address 2. neprimy odkaz
       * @param count pocet k precteni
       * @param result reference na vektor s vysledky
       */
      void readFromT2Address(uint64_t t2Address, uint32_t itemCount, std::vector<FolderItem>& result);

      /**
       * Ziska adresy vsech alokovanych bloku INode
       * @param node INode, pro kterou chceme ziskat adresy
       * @return vektor s adresami, ktere ma INode alokovane
       */
      std::vector<uint64_t> getINodeBlocks(INode& node);

      /**
       * Odstrani posledni FolderItem z dane INode
       * @param node INode, ze ktere se ma prvek odstranit
       */
      void removeLast(INode& node);

      /**
       * Zapise na dany index v INode dany FolderItem
       * @param node INode, pro kterou se predmet zapisuje
       * @param index index ve slozce
       * @param folderItem predmet slozky
       */
      void writeAt(INode& node, uint32_t index, FolderItem& folderItem);

      /**
       * Ziska pocet potrebnych extra bloku pro zapis
       * @param bytes velikost v bytech
       * @return pocet extra bloku pro zapis
       */
      uint64_t getExtraBlocks(uint64_t bytes);

      /**
       * Vytiskne buffer - vektor bytu
       * @param vector vektor bytu (char)
       */
      static void printBuffer(std::vector<char>& vector);
};

#endif //INODEIO_HPP