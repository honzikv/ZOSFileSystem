
#include <memory>
#include "FileSystemController.hpp"
#include "INodeIO.hpp"

FileSystemController::FileSystemController(const std::string& fileName) : fstream(fileName) {
    if (fstream.getFileLength() < Globals::SUPERBLOCK_SIZE_BYTES) {
        throw FSException("Loaded file does not contain valid file system, please format it with format command");
    }
    superBlock = std::make_shared<SuperBlock>();
    fstream >> *superBlock;

    if (!superBlock->isValid()) {
        throw FSException("Loaded file does not contain valid file system, please format it with format command");
    }

    memoryAllocator = std::make_shared<MemoryAllocator>(superBlock);
    nodeIO = std::make_shared<INodeIO>(fstream, *this);
    pathInfo = std::make_shared<PathInfo>(); // todo call saveInfo

    auto root = fstream.readINode(superBlock->nodeAddress); // reference na root node pro nastaveni path info
    auto children = nodeIO->getItems(root);
    pathInfo->saveInfo(children);
}

void FileSystemController::reclaimMemory(std::vector<uint64_t>& memoryBlocks) {

}

void FileSystemController::update(INode& node) {

}

uint64_t FileSystemController::nextBlock(AddressType type) {
    return 0;
}

void FileSystemController::outcp(const std::string& fileIn, const std::string& fileOut) {

}

void FileSystemController::incp(const std::string& fileOut, const std::string& fileFS) {

}

void FileSystemController::info(const std::string& file) {

}

void FileSystemController::pwd() {

}

void FileSystemController::cd(const std::string& path) {

}

void FileSystemController::cat(const std::string& file) {

}

void FileSystemController::ls(const std::string& path) {

}

void FileSystemController::rmdir(const std::string& dirName) {

}

void FileSystemController::mkdir(const std::string& dirName) {

}

void FileSystemController::rm(const std::string& file) {

}

void FileSystemController::mv(const std::string& file, const std::string& path) {

}

void FileSystemController::cp(const std::string& file, const std::string& path) {

}
