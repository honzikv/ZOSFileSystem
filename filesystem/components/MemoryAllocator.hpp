
#ifndef MEMORYALLOCATOR_HPP
#define MEMORYALLOCATOR_HPP

#include <memory>
#include "../io/model/Bitmap.hpp"
#include "../io/model/SuperBlock.hpp"

/**
 * Slouzi pro pridelovani a uvolnovani pameti
 */
class MemoryAllocator {

      std::shared_ptr<SuperBlock> superBlock;

      std::unique_ptr<Bitmap> nodeBitmap;

      std::unique_ptr<Bitmap> blockBitmap;

    public:
      MemoryAllocator(std::shared_ptr<SuperBlock> superBlock);
};


#endif //MEMORYALLOCATOR_HPP
