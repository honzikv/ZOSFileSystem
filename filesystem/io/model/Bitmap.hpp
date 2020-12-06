
#ifndef BITMAP_HPP
#define BITMAP_HPP


#include <cstdint>
#include "../../util/FStreamWrapper.hpp"

/**
 * Trida reprezentujici bitmapu objektu - tzn. INode nebo bloku. Obsahuje operace pro manipulaci s externim souborem
 */
class Bitmap {

      FStreamWrapper& fstream; // fstream wrapper reference

      uint64_t startAddress; // pocatecni adresa bitmapy
      uint64_t sizeOfObject; // velikost daneho objektu - INode, Data block apod.

      std::vector<uint8_t> bitmap; // bitmapa abychom nemuseli neustale cist ze souboru

    public:
      /**
       * @param startAddress pocatecni adresa v souboru
       * @param count pocet prvku
       * @param sizeOfObject velikost prvku
       * @param fStreamWrapper reference na fstream wrapper pro snazsi cteni
       */
      Bitmap(uint64_t startAddress, uint32_t count, uint64_t sizeOfObject, FStreamWrapper& fStreamWrapper);

      bool isAddressEmpty(uint64_t address);

      uint64_t getFirstEmptyAddress();

      void updateBitmap(uint64_t bitmapIndex, uint8_t value);

      void setAddress(uint64_t address, bool empty);

      static uint8_t getFirstEmptyBit(uint8_t byte);

      bool setPosition(uint8_t value, uint32_t pos, FStreamWrapper& fstream);

      uint32_t findFirstEmpty();

      /**
       * Zjisti vsechny volne pozice (bity) pro dany index a vrati je v seznamu
       * @param positions seznam, do ktereho se pozice zapisou
       * @param index index objektu - inode nebo blok
       */
      void getEmptyPositions(std::vector<uint8_t>& positions, uint32_t index);
};


#endif //BITMAP_HPP
