
#include "MemoryAllocator.hpp"

#include <utility>

MemoryAllocator::MemoryAllocator(std::shared_ptr<SuperBlock> superBlock) {
    superBlock = std::move(superBlock);
}
