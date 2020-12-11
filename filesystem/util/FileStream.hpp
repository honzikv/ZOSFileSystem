
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

      LastOperation lastOp = LastOperation::None;

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

      template<typename T>
      void write(T& object) {
          // potreba provest seek pokud se predtim cetlo
          if (lastOp != LastOperation::Write) {
              fstream.seekp(fstream.tellp(), std::ios::beg);
              lastOp = LastOperation::Write;
          }
          fstream.write(reinterpret_cast<char*>(&object), sizeof(T));
      }

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

      std::vector<FolderItem> readFolderItems(uint64_t address);

      void writeSuperBlock(SuperBlock& superBlock);

      void readSuperBlock(SuperBlock& superBlock);

      void writeINode(INode& inode);

      bool good() { return fstream.good(); }

      bool isFileEmpty();

      uint64_t getFileSize() {

          fstream.seekg(0, std::ios::beg);
          auto beg = fstream.tellg();
          fstream.seekg(0, std::ios::end);
          auto end = fstream.tellg();
          fstream.seekg(0, std::ios::beg);
          return end - beg;
      }

      virtual ~FileStream();

      void readINode(INode& node);

      bool isOpen();

      void deleteFile();

      void createFile();

      long getWritePosition();

      long getReadPosition();
};


#endif //FILESTREAM_HPP
