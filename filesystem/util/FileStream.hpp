
#ifndef FILESTREAM_HPP
#define FILESTREAM_HPP

#include <fstream>
#include <filesystem>
#include <iostream>
#include "../io/model/FolderItem.hpp"
#include "FSException.hpp"

/**
 * Wrapper pro cteni a zapis do souboru
 */
class SuperBlock;

enum class LastOperation {
      Read,
      Write,
      None
};

class FileStream {
      static constexpr uint64_t FORMAT_BUFFER_SIZE_BYTES = 4096;

      std::fstream fstream; // fstream objekt pro cteni souboru
      std::string filePath; // cesta k souboru

      LastOperation lastOp = LastOperation::None; // posledni operace - pro prepnuti mezi ctenim a zapisem

    public:

      /**
       * Zjisti zda-li cesta - filePath exisutuje
       * @return vrati true, pokud soubor existuje, jinak false
       */
      bool fileExists();

      /**
       * Otevre stream pro dany soubor ve filePath
       */
      void open();

      void openAppendOnly();

      /**
       * Manualne zavre fstream, tuto funkci neni nutno volat na konci programu protoze fstream je RAII, tzn stream
       * se zavre sam po smazani tohoto objektu
       */
      void close();

      explicit FileStream(std::string filePath);

      /**
       * Vytvori soubor a slozky, pokud neexistuji
       * @param filePath path dane cesty
       */
      static void createFileIfNotExists(const std::filesystem::path& filePath);

      /**
       * Template pro zapis jakehokoliv souboru, ktery je zapsan v pameti spojite
       * @tparam T typ
       * @param object reference na objekt, do ktereho se data zapisi
       */
      template<typename T>
      void write(T& object) {
          // potreba provest seek pokud se predtim cetlo
          if (lastOp != LastOperation::Write) {
              fstream.seekp(fstream.tellp(), std::ios::beg);
              lastOp = LastOperation::Write;
          }
          fstream.write(reinterpret_cast<char*>(&object), sizeof(T));

          if (!fstream.good()) {
              throw FSException("Error while writing to file");
          }
      }

      /**
       * Template pro cteni jakehokoliv souboru, ktery je zapsan v pameti spojite (tzn bez referenci)
       * @tparam T typ
       * @param object reference na objekt
       */
      template<typename T>
      void read(T& object) {
          // potreba provest seek pokud se predtim zapisovalo
          if (lastOp != LastOperation::Read) {
              fstream.seekg(fstream.tellg(), std::ios::beg);
              lastOp = LastOperation::Read;
          }
          fstream.read(reinterpret_cast<char*>(&object), sizeof(T));
          if (!fstream.good()) {
              throw FSException("Error while reading from file");
          }
      }

      /**
       * Presun na danou pozici (relativne od zacatku)
       * @param pos pozice v souboru
       */
      void moveTo(uint64_t pos) {
          fstream.seekg(pos, std::fstream::beg);
      }

      /**
       * Zapise folder item na danou adresu
       * @param folderItem reference na folder item, ktery se ma zapsat
       * @param address adresa, na kterou se filestream presune
       */
      void writeFolderItem(FolderItem& folderItem, uint64_t address);

      /**
       * Zapise vektor do souboru od aktualni pozice. Umoznuje zapsani pouze primitivnich typu a objektu, ktere jsou
       * ulozene v pameti na jednom miste - tzn. nelze ukladat komplexni objekty s referencemi apod
       * @tparam T typ dat ve vektoru
       * @param vector reference na vektor
       */
      template<typename T>
      void writeVector(std::vector<T>& vector) {
          for (auto& obj : vector) {
              write(obj);
          }
      }

      /**
       * Precte do vektoru data. Vektor musi byt inicializovany na prazdne instance daneho typu a musi obsahovat
       * stejny pocet instanci, ktere chceme nacist
       * @tparam T typ objektu
       * @param vector vektor, kam se data zapisou
       */
      template<typename T>
      void readVector(std::vector<T>& vector) {
          for (auto& obj : vector) {
              read(obj);
          }
      }

      /**
       * Formatuje dany soubor od zacatku. Tato funkce by se mela pouzit pouze pro prazdny soubor a prave jednou.
       * @param bytes velikost souboroveho systemu
       */
      void format(uint64_t bytes);

      /**
       * Precte blok a interpretuje ho jako blok FolderItem objektu
       * @param address adresa zacatku bloku
       * @return vektor s nactenymi FolderItems
       */
      std::vector<FolderItem> readFolderItemBlock(uint64_t address);

      /**
       * Zapis super bloku
       * @param superBlock reference na super blok
       */
      void writeSuperBlock(SuperBlock& superBlock);

      /**
       * Cteni super bloku
       * @param superBlock reference na superblok, kam se data ulozi
       */
      void readSuperBlock(SuperBlock& superBlock);

      /**
       * Zapise INode
       * @param inode reference na INode
       */
      void writeINode(INode& inode);

      /**
       * Zda-li je fstream "good"
       * @return fstream.good()
       */
      bool good() { return fstream.good(); }

      /**
       * Zda-li je soubor prazdny
       * @return true, pokud je soubor prazdny
       */
      bool isFileEmpty();

      /**
       * Ziska velikost souboru
       * @return velikost souboru v bytech
       */
      uint64_t getFileSize() {
          fstream.seekg(0, std::ios::beg);
          auto beg = fstream.tellg();
          fstream.seekg(0, std::ios::end);
          auto end = fstream.tellg();
          fstream.seekg(0, std::ios::beg);
          return end - beg;
      }

      /**
       * Destruktor - provede flush fstreamu
       */
      virtual ~FileStream();

      /**
       * Precte INode
       * @param node reference na INode, do ktere se data ulozi
       */
      void readINode(INode& node);

      /**
       * Smaze soubor
       */
      void deleteFile();

      /**
       * Vytvori soubor
       */
      void createFile();

      /**
       * Precte n zaznamu slozky z dane adresy
       * @param address adresa pro cteni
       * @param n pocet predmetu
       * @return vektor s prectenymi predmety
       */
      std::vector<FolderItem> readNFolderItems(uint64_t address, uint32_t n);
};


#endif //FILESTREAM_HPP
