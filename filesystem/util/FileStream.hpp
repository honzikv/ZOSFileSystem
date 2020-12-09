
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

      std::fstream fstream;

    public:

      explicit FileStream(const std::string& filePath);

      /**
       * Vytvori soubor a slozky, pokud neexistuji
       * @param filePath path dane cesty
       */
      static void createFileIfNotExists(const std::filesystem::path& filePath);

      template<typename T>
      void write(T& object) {
          fstream.write(reinterpret_cast<char*>(&object), sizeof(T));
          fstream.flush();
      }

      template<typename T, typename... Args>
      void write(T& object, Args& ... args) {
          write(object);
          write(args...);
      }

      template<typename T>
      void read(T& object) {
          fstream.read(reinterpret_cast<char*>(&object), sizeof(T));
      }


      template<typename T, typename... Args>
      void read(T& object, Args& ... args) {
          read(object);
          read(args...);
      }


      void moveTo(uint64_t pos) {
          fstream.flush();
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

      void formatSpace(uint64_t bytes);

      std::vector<FolderItem> readFolderItems(uint64_t address);

      INode readINode(uint64_t address);


};


#endif //FILESTREAM_HPP
