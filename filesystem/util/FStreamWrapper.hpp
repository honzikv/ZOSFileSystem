#ifndef FSTREAMWRAPPER_HPP
#define FSTREAMWRAPPER_HPP

#include <cstdint>
#include <fstream>
#include <vector>
#include <cstring>

#include "../global/Globals.hpp"
#include "../io/model/FolderItem.hpp"


class FStreamWrapper {

      static constexpr uint64_t FORMAT_BUFFER_BYTES = 4096;

      std::fstream fstream; // fstream na soubor

    public:
      explicit FStreamWrapper(const std::string& fileName) {
          createFileIfNotExists(fileName);
          fstream.open(fileName, std::ios::binary | std::ios::in | std::ios::out);
      }

      std::vector<FolderItem> readFolderItems(uint64_t blockAddress) {
          auto result = std::vector<FolderItem>();
          for (auto i = 0; i < Globals::FOLDER_ITEMS_PER_BLOCK(); i++) {
              auto folderItem = FolderItem();
              *this >> folderItem; // vyuziti friend operatoru, ktery implementuje FolderItem
              result.push_back(folderItem);
          }
          return result;
      }

      /**
       * Zapis libovolneho (primitivniho) datoveho typu ze streamu
       * @tparam T typ
       * @param t reference
       */
      template<typename T>
      void write(T& t) { fstream.write(reinterpret_cast<const char*>(&t), sizeof(T)); }

      /**
        * Varargs varianta pro zapis dat
        */
      template<typename T, typename ... Args>
      void write(T& first, Args& ... args) {
          write(first);
          write(args...);
      }

      /**
       * Funkce pro zapis vektoru dat do souboru. Toto lze pouzit pouze pro primitivni typy - tzn. uint, float apod.
       * @tparam T typ dat
       * @param vector reference na vektor
       */
      template<typename T>
      void writeVector(const std::vector<T>& vector) {
          for (auto byteObj : vector) {
              fstream.write(byteObj);
          }
      }

      /**
       * Precteni libovolneho (primitivniho) datoveho typu ze streamu
       * @tparam T typ
       * @param t reference na objekt do ktereho se data zapisi
       */
      template<typename T>
      void read(T& t) { fstream.read(&t, sizeof(T)); }

      /**
       * Varargs varianta pro precteni dat
       */
      template<typename T, typename... Args>
      void read(T& t, Args& ... args) {
          read(t);
          read(args...);
      }

      /**
       * Nacte do daneho vektoru data ze souboru. Funguje pouze pro primitivni datove typy jako uint, float apod.
       * Vektor musi mit predem alokovany pocet prvku, ktere se pote nahradi
       */
      template<typename T>
      std::vector<T> readVector(std::vector<T>& vector) {
          auto amount = vector.size();
          auto byteSize = sizeof(T) * amount;
          auto buffer = std::vector<char>(byteSize);
          fstream.read(buffer.data(), byteSize);
          for (auto i = 0; i < amount; i++) {
              std::memcpy(&vector[i], &buffer[i * sizeof(T)], sizeof(T));
          }
      }

      void formatSpace(uint64_t bytes);

      /**
       * Presun na danou adresu v souboru
       * @param pos adresa v souboru
       */
      void moveTo(uint64_t pos) {
          fstream.seekp(pos);
          fstream.seekg(pos);
      }

      /**
       * Ziska delku souboru
       * @return delku souboru
       */
      uint64_t getFileLength() {
          fstream.seekg(0, std::ios::end);
          return fstream.tellg();
      }

    private:
      /**
       * Vytvori soubor, pokud neexistuje
       * @param fileName nazev souboru
       */
      static void createFileIfNotExists(const std::string& fileName) {
          auto ifstream = std::ifstream(fileName, std::ios::in | std::ios::binary);
          if (!ifstream.good()) {
              ifstream.close();
              std::ofstream(fileName, std::ios::out | std::ios::binary)
                      .close();
          }
      }
};

#endif //ZOSREWORK_FSTREAMWRAPPER_HPP
