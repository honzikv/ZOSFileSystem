
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

      void updateSuperBlock();

      void freeT1Address(uint64_t t1Address);

      void freeT2Address(uint64_t t2Address);

      void format(uint64_t address, AddressType addressType);

      uint64_t getNodeAddress(INode& node);

      uint64_t getDataBlock(AddressType addressType);

      INode getINode(bool isFolder);

      std::vector<uint64_t> getNDataBlocks(uint64_t n, AddressType addressType);

      void freeINode(INode& node);

      INode getINodeWithId(uint32_t nodeId);
};


#endif //MEMORYALLOCATOR_HPP
