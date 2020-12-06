
#include <memory>
#include "FileSystemController.hpp"
#include "INodeIO.hpp"

FileSystemController::FileSystemController(const std::string& fileName) : fstream(fileName) {
    superBlock = std::make_shared<SuperBlock>();
    fstream >> *superBlock;

    if (!superBlock->isValid()) {
        throw FSException("Error, loaded file is not a valid file system");
    }

    memoryAllocator = std::make_unique<MemoryAllocator>(superBlock);
    nodeIO = std::make_unique<INodeIO>(fstream, *this);
    pathInfo = std::make_unique<PathInfo>(); // todo call saveInfo

    auto root = fstream.readINode(superBlock->nodeAddress); // reference na root node pro nastaveni path info
    auto children = nodeIO->getItems(root);
    pathInfo->saveInfo()
}

void FileSystemController::reclaimMemory(std::vector<uint64_t>& memoryBlocks) {

}

void FileSystemController::update(INode& node) {

}
