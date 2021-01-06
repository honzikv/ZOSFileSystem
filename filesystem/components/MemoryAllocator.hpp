
#ifndef MEMORYALLOCATOR_HPP
#define MEMORYALLOCATOR_HPP

#include <memory>
#include "../io/model/Bitmap.hpp"
#include "../io/model/SuperBlock.hpp"
#include "../io/AddressType.h"

/**
 * Slouzi pro pridelovani a uvolnovani pameti
 */
class MemoryAllocator {

      FileStream& fileStream; // reference na filestream

      std::shared_ptr<SuperBlock> superBlock; // reference (shared pointer) na super blok

      std::shared_ptr<Bitmap> nodeBitmap; // reference na bitmapu pro INodes
      std::shared_ptr<Bitmap> blockBitmap; // reference na bitmapu pro bloky

    public:
      MemoryAllocator(std::shared_ptr<SuperBlock> superBlock, FileStream& fileStream);

      void update(INode& indode);

      /**
       * Uvolni jeden datovy blok. Zaroven pricte do superbloku volne misto
       * @param address
       */
      void freeMemory(uint64_t address);

      /**
       * Uvolni pamet pro INode pomoci freeMemory funkce
       * @param node inode, pro kterou se pamet dealokuje
       */
      void freeMemory(INode& node);

      /**
       * Uvolneni 1. neprimeho odkazu
       * @param t1Address adresa 1. neprimeho odkazu
       */
      void freeT1Address(uint64_t t1Address);

      /**
       * Uvolneni 2. neprimeho odkazu
       * @param t2Address 2. neprimy odkaz
       */
      void freeT2Address(uint64_t t2Address);

      /**
       * Provede formatovani daneho bloku pro dany typ adresy
       * @param address adresa zacatku bloku
       * @param addressType typ adresy
       */
      void format(uint64_t address, AddressType addressType);

      /**
       * Ziska adresu INode
       * @param node INode, pro kterou chceme ziskat adresu
       * @return adresu INode
       */
      uint64_t getNodeAddress(INode& node);

      /**
       * Ziska volny datovy blok, vyhodi exception pokud neexistuje
       * @param addressType typ formatovani (typ adresy)
       * @return adresu k zacatku bloku
       */
      uint64_t getDataBlock(AddressType addressType);

      /**
       * Ziska novou volnou INode, vyhodi exception pokud neexistuje
       * @param isFolder zda-li ma byt INode slozka
       * @return novou INode
       */
      INode getINode(bool isFolder);

      /**
       * Ziska N datovych bloku a vrati je ve vektoru
       * @param n pocet datovych bloku
       * @param addressType typ formatovani
       * @return vektor s volnymi datovymi bloky
       */
      std::vector<uint64_t> getNDataBlocks(uint64_t n, AddressType addressType);

      /**
       * Uvolni danou INode
       * @param node INode, ktera se ma uvolnit
       */
      void freeINode(INode& node);

      /**
       * Ziska INode pro dane id
       * @param nodeId id INode
       * @return INode s danym id
       */
      INode getINodeWithId(uint32_t nodeId);

      /**
       * Debug bitmapy pro INodes
       */
      void debugNodeBitmap() {
          nodeBitmap->debug();
      }

      /**
       * Debug bitmapy pro bloky dat
       */
      void debugBlockBitmap() {
          blockBitmap->debug();
      }
};


#endif //MEMORYALLOCATOR_HPP
