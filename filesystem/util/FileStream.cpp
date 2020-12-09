
#include "FileStream.hpp"

FileStream::FileStream(const std::string& filePath) : filePath(filePath) {
    auto path = std::filesystem::path(filePath);
    createFileIfNotExists(path);
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
    fstream.open(filePath, std::ios::binary | std::ios::in | std::ios::out);
    fstream.seekg(address);
    auto result = std::vector<FolderItem>();
    for (auto i = 0; i < Globals::FOLDER_ITEMS_PER_BLOCK(); i++) {
        auto folderItem = FolderItem();
        *this >> folderItem;
        result.push_back(folderItem);
    }
    currentReadPosition = fstream.tellg();
    fstream.close();
    return result;
}

INode FileStream::readINode(uint64_t address) {
    auto node = INode();
    moveTo(address);
    *this >> node;
    return node;
}

void FileStream::moveToStart() { moveTo(0); }

void FileStream::moveTo(uint64_t pos) {
    currentReadPosition = pos;
}

void FileStream::formatSpace(uint64_t bytes) {
    openToRead()
    auto buffer = std::vector<char>(FORMAT_BUFFER_SIZE_BYTES, 0);
    auto bufferWrites = bytes / FORMAT_BUFFER_SIZE_BYTES;
    auto remainder = bytes % FORMAT_BUFFER_SIZE_BYTES;

    for (auto i = 0; i < bufferWrites; i++) {
        fstream.write(buffer.data(), buffer.size());
    }
    auto remainingBytes = std::vector<char>(remainder, 0);
    fstream.write(remainingBytes.data(), remainingBytes.size());
    fstream.flush();
}

FileStream::~FileStream() {
    fstream.close();
}



