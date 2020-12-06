
#ifndef MEMORYALLOCATOR_HPP
#define MEMORYALLOCATOR_HPP

#include <bits/unique_ptr.h>
#include "../io/model/Bitmap.hpp"
#include "../io/model/SuperBlock.hpp"

/**
 * Slouzi pro pridelovani a uvolnovani pameti
 */
class MemoryAllocator {

      std::unique_ptr<SuperBlock> superBlock;

      std::unique_ptr<Bitmap> nodeBitmap;

      std::unique_ptr<Bitmap> blockBitmap;

    public:
      MemoryAllocator(std::unique_ptr<SuperBlock>& superBlock);
};


#endif //MEMORYALLOCATOR_HPP
