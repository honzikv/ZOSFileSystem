
#include <memory>
#include "FileSystemController.hpp"
#include "../io/model/SuperBlock.hpp"
#include "../util/FSException.hpp"

FileSystemController::FileSystemController(const std::string& fileName) : fstream(fileName) {
    auto superBlock = std::make_unique<SuperBlock>();
    fstream >> *superBlock;

    if (!superBlock->isValid()) {
        throw FSException("Error, loaded file is not a valid file system");
    }

    memoryAllocator = std::make_unique<MemoryAllocator>(superBlock);
}
