
#ifndef SUPERBLOCK_HPP
#define SUPERBLOCK_HPP


#include <cstdint>
#include <string>
#include <ostream>
#include "../../global/Globals.hpp"
#include "../../util/FileStream.hpp"

/**
* Obsahuje informace o super bloku
*/
struct SuperBlock {

      uint16_t magicNumber = (uint16_t) Globals::INVALID_VALUE; // cislo pro kontrolu
      uint64_t totalSize = Globals::INVALID_VALUE; // celkova velikost

      uint64_t blockSize = Globals::INVALID_VALUE; // velikost jednoho bloku
      uint64_t blockCount = Globals::INVALID_VALUE; // Pocet datovych bloku
      uint64_t blockBitmapAddress = Globals::INVALID_VALUE; // Pocatek bitmapy pro datove bloky

      uint64_t nodeCount = Globals::INVALID_VALUE; // Pocet INodes
      uint64_t nodeBitmapAddress = Globals::INVALID_VALUE; // Adresa bitmapy pro INodes

      uint64_t nodeAddress = Globals::INVALID_VALUE; // Pocatek INode objektu
      uint64_t dataAddress = Globals::INVALID_VALUE; // Pocatek datovych bloku

      uint64_t freeNodes = Globals::INVALID_VALUE; // nakonec nevyuzito
      uint64_t freeBlocks = Globals::INVALID_VALUE; // nakonec nevyuzito

      /**
       * Konstruktor pro vytvoreni super bloku, automaticky inicializuej vsechna potrebna data
       * @param totalSize
       */
      explicit SuperBlock(uint64_t totalSize);

      SuperBlock() = default;

      /**
       * Vrati, zda-li je super blok validni (porovnani cisla)
       * @return true, pokud je super blok validni, jinak false
       */
      [[nodiscard]] bool isValid() const;

    private:
      /**
       * Vypocte pocet bloku
       * @param sizeBytes velikost disku v bytech
       * @return pocet datovych bloku
       */
      static uint64_t getBlockCount(uint64_t sizeBytes);

      /**
       * Vypocte pocet INodes pro disk
       * @param sizeBytes velikost disku v bytech
       * @return pocet inode
       */
      static uint64_t getNodeCount(uint64_t sizeBytes);

      /**
       * Vrati velikost bitmapy pro dany pocet objektu
       * @param objectCount pocet objektu
       * @return velikost bitmapy v bytech
       */
      static uint64_t getBitmapSize(uint64_t objectCount);

    public:
      /**
       * Tisk informaci o superbloku pro debug
       */
      void printInfo() const;
};


#endif //SUPERBLOCK_HPP
