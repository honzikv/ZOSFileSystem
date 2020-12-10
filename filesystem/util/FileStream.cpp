
#include "FileStream.hpp"
#include "FSException.hpp"
#include "../io/model/SuperBlock.hpp"

FileStream::FileStream(std::fstream& fstream) : fstream(fstream) {
    if (!fstream.good()) {
        throw FSException("Error file could not be open");
    }
}

void FileStream::createFileIfNotExists(const std::filesystem::path& filePath) {
    if (filePath.has_parent_path()) {
        auto folders = filePath.parent_path();
        std::filesystem::create_directories(folders);
    }

    auto ifstream = std::ifstream(filePath.string());
    if (!ifstream.good()) {
        ifstream.close();
        std::ofstream(filePath.string());
    }
}


void FileStream::writeFolderItem(FolderItem& folderItem, uint64_t address) {
    moveTo(address);
    *this << folderItem;
}

std::vector<FolderItem> FileStream::readFolderItems(uint64_t address) {
    moveTo(address);
    auto result = std::vector<FolderItem>();
    for (auto i = 0; i < Globals::FOLDER_ITEMS_PER_BLOCK(); i++) {
        auto folderItem = FolderItem();
        *this >> folderItem;
        result.push_back(folderItem);
    }
    return result;
}

INode FileStream::readINode(uint64_t address) {
    auto node = INode();
    moveTo(address);
    *this >> node;
    return node;
}


void FileStream::formatSpace(uint64_t bytes) {
    auto buffer = std::vector<char>(FORMAT_BUFFER_SIZE_BYTES, 0);
    auto bufferWrites = bytes / FORMAT_BUFFER_SIZE_BYTES;
    auto remainder = bytes % FORMAT_BUFFER_SIZE_BYTES;

    for (auto i = 0; i < bufferWrites; i++) {
        fstream.write(buffer.data(), buffer.size());
    }
    auto remainingBytes = std::vector<char>(remainder, 0);
    fstream.write(remainingBytes.data(), remainingBytes.size());

    moveTo(0);
}

bool FileStream::isFileEmpty() {
    fstream.seekg(0, std::ios::end);
    auto a = fstream.tellg() == 0;
    return a;
}

void FileStream::writeSuperBlock(SuperBlock& superBlock) {
    write(superBlock.magicNumber,
          superBlock.totalSize,
          superBlock.blockSize,
          superBlock.blockCount,
          superBlock.blockBitmapAddress,
          superBlock.nodeCount,
          superBlock.nodeBitmapAddress,
          superBlock.nodeAddress,
          superBlock.dataAddress,
          superBlock.freeNodes,
          superBlock.freeBlocks);
}

void FileStream::readSuperBlock(SuperBlock& superBlock) {
    read(superBlock.magicNumber,
         superBlock.totalSize,
         superBlock.blockSize,
         superBlock.blockCount,
         superBlock.blockBitmapAddress,
         superBlock.nodeCount,
         superBlock.nodeBitmapAddress,
         superBlock.nodeAddress,
         superBlock.dataAddress,
         superBlock.freeNodes,
         superBlock.freeBlocks);
}
