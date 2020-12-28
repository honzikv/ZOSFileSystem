#include "FileStream.hpp"

#include <utility>
#include "FSException.hpp"
#include "../io/model/SuperBlock.hpp"

FileStream::FileStream(std::string filePath) : filePath(std::move(filePath)) {
}

bool FileStream::fileExists() {
    return std::filesystem::exists(filePath);
}

void FileStream::open() {
    fstream.open(filePath, std::ios::binary | std::ios::in | std::ios::out);
}

void FileStream::openAppendOnly() {
    fstream.open(filePath, std::ios::binary | std::ios::out | std::ios::app);
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

void FileStream::deleteFile() {
    fstream.close();
    if (fileExists()) {
        std::remove(filePath.c_str());
    }
}

void FileStream::createFile() {
    auto path = std::filesystem::path(filePath);
    createFileIfNotExists(path);
}


void FileStream::writeFolderItem(FolderItem& folderItem, uint64_t address) {
    moveTo(address);
    write(folderItem.nodeAddress);
    writeVector(folderItem.itemName);
}

std::vector<FolderItem> FileStream::readFolderItemBlock(uint64_t address) {
    return readNFolderItems(address, Globals::FOLDER_ITEMS_PER_BLOCK());
}

std::vector<FolderItem> FileStream::readNFolderItems(uint64_t address, uint32_t n) {
    moveTo(address);
    auto result = std::vector<FolderItem>();
    result.reserve(n);
    auto folderItem = FolderItem();
    for (auto i = 0; i < n; i++) {
        read(folderItem.nodeAddress);
        readVector(folderItem.itemName);
        result.push_back(folderItem);
    }
    return result;
}

void FileStream::format(uint64_t bytes) {
    moveTo(0);
    auto buffer = std::vector<char>(FORMAT_BUFFER_SIZE_BYTES, 0);
    auto bufferWrites = bytes / FORMAT_BUFFER_SIZE_BYTES;
    auto remainder = bytes % FORMAT_BUFFER_SIZE_BYTES;

    if (lastOp == LastOperation::Read) {
        fstream.seekp(fstream.tellp(), std::ios::beg);
        lastOp = LastOperation::Write;
    }

    for (auto i = 0; i < bufferWrites; i++) {
        fstream.write(buffer.data(), buffer.size());
    }
    auto remainingBytes = std::vector<char>(remainder, 0);
    fstream.write(remainingBytes.data(), remainingBytes.size());
    fstream.flush();
}

bool FileStream::isFileEmpty() {
    fstream.seekg(0, std::ios::end);
    return fstream.tellg() == 0;
}

void FileStream::writeSuperBlock(SuperBlock& superBlock) {
    write(superBlock.magicNumber);
    write(superBlock.totalSize);
    write(superBlock.blockSize);
    write(superBlock.blockCount);
    write(superBlock.blockBitmapAddress);
    write(superBlock.nodeCount);
    write(superBlock.nodeBitmapAddress);
    write(superBlock.nodeAddress);
    write(superBlock.dataAddress);
    write(superBlock.freeBlocks);
    write(superBlock.freeNodes);
}

void FileStream::readSuperBlock(SuperBlock& superBlock) {
    read(superBlock.magicNumber);
    read(superBlock.totalSize);
    read(superBlock.blockSize);
    read(superBlock.blockCount);
    read(superBlock.blockBitmapAddress);
    read(superBlock.nodeCount);
    read(superBlock.nodeBitmapAddress);
    read(superBlock.nodeAddress);
    read(superBlock.dataAddress);
    read(superBlock.freeBlocks);
    read(superBlock.freeNodes);
}

void FileStream::writeINode(INode& node) {
    write(node.folder);
    write(node.id);
    write(node.size);
    write(node.timestamp);
    write(node.refCount);
    write(node.t1Address);
    write(node.t2Address);
    writeVector(node.t0AddressList);
}

void FileStream::readINode(INode& node) {
    read(node.folder);
    read(node.id);
    read(node.size);
    read(node.timestamp);
    read(node.refCount);
    read(node.t1Address);
    read(node.t2Address);
    readVector(node.t0AddressList);
}

void FileStream::close() {
    fstream.close();
}

bool FileStream::isOpen() {
    return fstream.is_open();
}

long FileStream::getWritePosition() {
    return fstream.tellp();
}

long FileStream::getReadPosition() {
    return fstream.tellg();
}

FileStream::~FileStream() {
    fstream.flush();
}


