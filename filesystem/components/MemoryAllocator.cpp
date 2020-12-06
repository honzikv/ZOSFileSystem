
#include "MemoryAllocator.hpp"

MemoryAllocator::MemoryAllocator(std::unique_ptr<SuperBlock>& superBlock) {
    superBlock = std::move(superBlock);
}
