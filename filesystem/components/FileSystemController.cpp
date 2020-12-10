
#include <memory>
#include <iostream>
#include "FileSystemController.hpp"
#include "INodeIO.hpp"


FileSystemController::FileSystemController(FileStream& fileStream) : fileStream(fileStream) {
    if (fileStream.isFileEmpty()) {
        std::cout << "File is empty" << std::endl;
        throw FSException("File is empty");
    }
    auto superBlock =  std::make_shared<SuperBlock>();
    std::cout << fileStream.good() << std::endl;
    fileStream.moveTo(0);
    fileStream.readSuperBlock(*superBlock);

    std::cout << fileStream.good() << std::endl;
    if (!superBlock->isValid()) {
        fileStream.moveTo(0);
        throw FSException("Filesystem is corrupt!");
    } else {

        std::cout << "Filesystem loaded" << std::endl;
        std::cout << "Superblock : {\n" << "\tfreeBlocks: " << superBlock->freeBlocks << ", \n\tfreeNodes: "
                  << superBlock->freeNodes << ", \n\tblockSize: " << superBlock->blockSize << "\n}" << std::endl;
    }
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
