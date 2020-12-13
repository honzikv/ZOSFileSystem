
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

      std::shared_ptr<SuperBlock> superBlock; // reference (shared pointer) na super blok

      FileStream& fileStream; // reference na filestream

      std::shared_ptr<Bitmap> nodeBitmap; // reference na bitmapu pro INodes

      std::shared_ptr<Bitmap> blockBitmap; // reference na bitmapu pro bloky

    public:
      MemoryAllocator(std::shared_ptr<SuperBlock> superBlock, FileStream& fileStream);

      void update(INode& indode);

      void freeMemory(uint64_t address);

      void format(uint64_t address, AddressType addressType);

      uint64_t getNodeAddress(INode& node);

      uint64_t getDataBlock(AddressType addressType);

      INode getINode();

      std::vector<uint64_t> getNDataBlocks(uint64_t n, AddressType addressType);

      void freeINode(INode& node);

      INode getINodeWithId(uint32_t nodeId);
};


#endif //MEMORYALLOCATOR_HPP
