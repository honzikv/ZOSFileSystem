#include <iostream>
#include "FStreamWrapper.hpp"


void FStreamWrapper::formatSpace(uint64_t bytes) {
    auto bufferFullWrites = bytes / FORMAT_BUFFER_BYTES;
    auto remainder = bytes % FORMAT_BUFFER_BYTES;
    auto buffer = std::vector<char>(FORMAT_BUFFER_BYTES, (int) 0);
    auto remainderBuffer = std::vector<char>(remainder, (int) 0);

    for (auto i = 0; i < bufferFullWrites; i++) { fstream.write(buffer.data(), buffer.size()); }
    fstream.write(remainderBuffer.data(), remainderBuffer.size());
}

INode FStreamWrapper::readINode(uint64_t address) {
    auto node = INode();
    moveTo(address);
    *this >> node;
    return node;
}

void FStreamWrapper::writeFolderItem(FolderItem& folderItem, uint64_t address) {
    moveTo(address);
    *this << folderItem;
}

FStreamWrapper::FStreamWrapper(const std::string& fileName) {
    createFileIfNotExists(fileName);
    fstream.open(fileName, std::ios::binary | std::ios::in | std::ios::out);
}

std::vector<FolderItem> FStreamWrapper::readFolderItems(uint64_t blockAddress) {
    auto result = std::vector<FolderItem>();
    for (auto i = 0; i < Globals::FOLDER_ITEMS_PER_BLOCK(); i++) {
        auto folderItem = FolderItem();
        *this >> folderItem; // vyuziti friend operatoru, ktery implementuje FolderItem
        result.push_back(folderItem);
    }
    return result;
}

void FStreamWrapper::createFileIfNotExists(const std::string& fileName) {
    auto ifstream = std::ifstream(fileName, std::ios::in | std::ios::binary);
    if (!ifstream.good()) {
        ifstream.close();
        std::ofstream(fileName, std::ios::out | std::ios::binary)
                .close();
    }
}

uint64_t FStreamWrapper::getPosition() {
    return fstream.tellp(); // nebo tellg
}

FStreamWrapper::~FStreamWrapper() {
    std::cout << "bye" << std::endl;
}

FStreamWrapper::FStreamWrapper(std::fstream& fstream) : fstream(fstream) {}
