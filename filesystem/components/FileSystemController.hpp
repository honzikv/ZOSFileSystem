
#ifndef FILESYSTEMCONTROLLER_HPP
#define FILESYSTEMCONTROLLER_HPP

#include <string>
#include "MemoryAllocator.hpp"
#include "../io/AddressType.h"
#include "../util/FileStream.hpp"

class PathContext;

class INodeIO;

class FileOperations;


/**
 * Stav disku (souboru)
 */
enum class DriveState {
      NonExistent, // soubor (cesta) neexistuje
      Valid, // soubor obsahuje validni super block
      Empty, // soubor je prazdny
      Invalid // v souboru neni od adresy 0 super block
};

/**
 * Trida, ktera slouzi pro ovladani filesystemu
 */
class FileSystemController {
      friend class FileSystem;

      FileStream& fileStream; // reference na filestream ziskana z FileSystem objektu

      DriveState driveState = DriveState::Empty; // stav disku

      std::shared_ptr<SuperBlock> superBlock; // reference na super blok disku
      std::shared_ptr<MemoryAllocator> memoryAllocator; // reference na memory alokator pro prirazovani pameti
      std::shared_ptr<INodeIO> nodeIO; // reference na praci s INodes
      std::shared_ptr<FileOperations> fileOperations; // reference na praci se soubory

      void initDrive(); // inicializace disku - zapis inod, bitmapy apod.

    public:

      /**
       * Ziska stav disku
       * @return
       */
      [[nodiscard]] DriveState getDriveState() const;

      /**
       * Konstruktor potrebuje referenci na file stream pro cteni ze souboru
       * @param fileStream
       */
      explicit FileSystemController(FileStream& fileStream);

      /**
       * Informace o disku
       */
      void diskInfo();

      /**
       * Ziska root INode
       * @return Root disku
       */
      INode getRoot();

      // ** FUNKCE PRO PRIKAZY ** //

      void cp(const std::string& file, const std::string& path);

      void mv(const std::string& file, const std::string& path);

      void rm(const std::string& path);

      void mkdir(const std::string& path);

      void rmdir(const std::string& path);

      void ls(const std::string& path);

      void cat(const std::string& file);

      void cd(const std::string& path);

      void pwd();

      void info(const std::string& file);

      void incp(const std::string& readPath, const std::string& path);

      void outcp(const std::string& path, const std::string& exportPath);

      void ln(const std::string& file, const std::string& path);

      // **  ** //

      /**
       * Pro debug bitmapy bloku - vytiskne informace
       */
      void debugBlockBitmap();

      /**
       * Debug bitmapy INodes
       */
      void debugNodeBitmap();

      /**
       * Ziska dalsi volny blok
       * @param type typ formatu
       * @return adresu volneho bloku
       */
      uint64_t nextBlock(AddressType type);

      /**
       * Uvolneni pameti
       * @param memoryBlocks seznam s bloky na uvolneni
       */
      void reclaimMemory(std::vector<uint64_t>& memoryBlocks);

      /**
       * Aktualizace INode
       * @param node INode, ktera se ma aktualizovat
       */
      void refresh(INode& node);

      /**
       * Ziska predmety dane INode
       * @param node INode, pro kterou chceme predmety ziskat
       * @return seznam FolderItem objektu
       */
      std::vector<FolderItem> getFolderItems(INode& node);

      /**
       * Ziska INode z dane adresy
       * @param nodeAddress adresa INode
       * @return INode na dane adrese
       */
      INode getINodeFromAddress(uint64_t nodeAddress);

      /**
       * Ziska Adresu INode
       * @param node INode, pro kterou chceme ziskat adresu
       * @return Adresu ve filesystemu, na ktere INode zacina
       */
      uint64_t getINodeAddress(INode& node);

      /**
       * Ziska prvni volnou INode a vrati ji
       * @param isFolder zda-li ma byt INode slozka
       * @return INode objekt
       */
      INode getFreeINode(bool isFolder);

      /**
       * Prida FolderItem do slozky parent
       * @param parent nadrazena slozka
       * @param child zaznam - predmet, ktery se ve slozce ma vyskytovat
       */
      void appendFolderItem(INode& parent, FolderItem child);

      /**
       * Uvolneni INode
       * @param node INode, ktera se ma uvolnit
       */
      void reclaimINode(INode& node);

      /**
       * Vytvoreni INodes a zapis do souboru (pri formatovani)
       */
      void createINodes();

      /**
       * Ziska aktualizovanou INode ze souboru
       * @param node INode, kterou chceme aktualizovat
       * @return aktualizovanou INode
       */
      INode getUpdatedINode(INode& node);

      /**
       * Vytvori dva FolderItemy - rodic (..) a slozku samotnou (.) pro navigaci
       * @param child dite (aktualni slozka)
       * @param childAddress adresa aktualni slozky
       * @param parentNodeAddress adresa rodice (nadrazene slozky)
       */
      void linkFolderToParent(INode& child, uint64_t childAddress, uint64_t parentNodeAddress);

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
       * Prida externi soubor do filesystemu do dane slozky (parent)
       * @param parent nadrazena slozka
       * @param child soubor, ktery ma byt ve slozce
       * @param folderItem folder item souboru
       * @param externalFileStream file stream externiho souboru
       */
      void appendExternalFile(INode& parent, INode& child, FolderItem& folderItem, FileStream& externalFileStream);

      /**
       * Ziska N dalsich bloku pro zapis pameti
       * @param n pocet bloku
       * @param addressType typ formatovani - zda-li bude obsahovat FolderItems, nuly nebo nevalidni pointery apod.
       * @return vektor s adresami nebo vyhodi exception, ze doslo misto
       */
      std::vector<uint64_t> nextNBlocks(uint32_t n, AddressType addressType);

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
      void copyData(INode source, INode dest);
};


#endif //FILESYSTEMCONTROLLER_HPP
