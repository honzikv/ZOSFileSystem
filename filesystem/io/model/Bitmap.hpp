
#ifndef BITMAP_HPP
#define BITMAP_HPP


#include <cstdint>
#include "../../util/FileStream.hpp"

/**
 * Trida reprezentujici bitmapu objektu - tzn. INode nebo bloku. Obsahuje operace pro manipulaci s externim souborem
 */
class Bitmap {

      FileStream& fstream; // fstream wrapper reference

      uint64_t bitmapStartAddress; // pocatecni adresa bitmapy
      uint64_t objectStartAddress; // pocatecni adresa objektu
      uint64_t objectSizeBytes; // velikost daneho objektu - INode, Data block apod.

      std::vector<uint8_t> bitmap; // bitmapa abychom nemuseli neustale cist ze souboru
      std::string name;

    public:
      /**
       * @param bitmapStartAddress pocatecni adresa v souboru
       * @param count pocet prvku
       * @param objectSizeBytes velikost prvku
       * @param fileStream reference na fstream wrapper pro snazsi cteni
       */
      Bitmap(uint64_t bitmapStartAddress, uint32_t count, uint64_t objectStartAddress, uint64_t objectSizeBytes,
             FileStream& fileStream, std::string name);

      uint64_t getFirstEmptyAddress();

      void updateBitmap(uint64_t bitmapIndex, uint8_t value);

      void setAddress(uint64_t address, bool empty);

      uint8_t getFirstEmptyBit(uint8_t byte);

      [[nodiscard]] uint64_t getIdFromAddress(uint64_t itemAddress) const;

      void debug();
};


#endif //BITMAP_HPP
