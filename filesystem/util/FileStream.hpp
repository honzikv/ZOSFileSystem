
#ifndef FILESTREAM_HPP
#define FILESTREAM_HPP

#include <fstream>
#include <filesystem>
#include "../io/model/FolderItem.hpp"

/**
 * Wrapper pro cteni a zapis do souboru
 */
class FileStream {

      static constexpr uint64_t FORMAT_BUFFER_SIZE_BYTES = 4096;

      /**
       * Z nejakeho duvodu se data nezapisi dokud se soubor nezavre
       */
      void openToRead() {
          ifstream.open(filePath, std::ios::binary | std::ios::in);
          ifstream.seekg(currentReadPosition);
      }

      void openToWrite() {
          ofstream.open(filePath, std::ios::binary |std::ios::out);
          ofstream.seekp(currentWritePosition);
      }

      void closeWrite() {
          currentWritePosition = ofstream.tellp();
          ofstream.close();
      }

      void closeRead() {
          currentReadPosition = ifstream.tellg();
          ifstream.close();
      }


      std::string filePath;
      std::ifstream ifstream; // ifstream pro pristup k souboru
      std::ofstream ofstream; // ofstream pro pristup k souboru

      uint64_t currentReadPosition = 0;
      uint64_t currentWritePosition = 0;

    private:

      template<typename T>
      void _writeWithoutClosing(T& object) {
          ofstream.write(reinterpret_cast<const char*>(&object), sizeof(T));
      }

    public:

      /**
       * Vytvori soubor a slozky, pokud neexistuji
       * @param filePath path dane cesty
       */
      static void createFileIfNotExists(const std::filesystem::path& filePath);

      explicit FileStream(const std::string& filePath);

      bool isOpen();

      template<typename T>
      void write(T& object) {
          openToWrite();
          ofstream.write(reinterpret_cast<const char*>(&object), sizeof(T));
          closeWrite();
      }

      template<typename T, typename... Args>
      void write(T& object, Args& ... args) {
          openToWrite();
          _writeWithoutClosing(object);
          _writeWithoutClosing(args...);
          closeWrite();
      }

      template<typename T>
      void read(T& object) {
          openToRead();
          ifstream.read(reinterpret_cast<char*>(&object), sizeof(T));
          closeRead();
      }


      template<typename T, typename... Args>
      void read(T& object, Args& ... args) {
          read(object);
          read(args...);
      }

      void moveTo(uint64_t pos);

      void moveToStart();

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
      void writeVector(const std::vector<T>& vector) {
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

      void formatSpace(uint64_t bytes);

      std::vector<FolderItem> readFolderItems(uint64_t address);

      INode readINode(uint64_t address);

      uint64_t getPosition();

      virtual ~FileStream();
};


#endif //FILESTREAM_HPP
