
#include <chrono>
#include <iostream>
#include "INode.hpp"

uint32_t INode::getFolderSize() const { return size; }

INode::INode() {}

bool INode::isFree() const { return id != (uint32_t) Globals::INVALID_VALUE; }

bool INode::isFolder() const { return folder; }

const std::vector<uint64_t>& INode::getT0AddressList() const { return t0AddressList; }

uint64_t INode::getT1Address() const { return t1Address; }

uint64_t INode::getT2Address() const { return t2Address; }

uint32_t INode::getSize() const { return size; }

void INode::setSize(uint32_t size) { this->size = size; }

void INode::setDirectAddress(uint32_t index, uint64_t address) { t0AddressList[index] = address; }

void INode::setT1Address(uint64_t t1Address) { this->t1Address = t1Address; }

void INode::setT2Address(uint64_t t2Address) { this->t2Address = t2Address; }

INode::INode(bool isFolder, uint32_t id) {
    this->folder = isFolder;
    this->id = id;
}

void INode::incrFolderItemCount() {
    size += 1;
}

bool INode::operator==(const INode& rhs) const {
    return folder == rhs.folder && id == rhs.id;
}

bool INode::operator!=(const INode& rhs) const {
    return !(rhs == *this);
}

void INode::incrRefCount() {
    refCount += 1;
}

bool INode::isFolderFull() const {
    return size == Globals::MAX_FOLDER_ITEMS();
}

uint32_t INode::getId() const {
    return id;
}

INode INode::createRoot() {
    auto root = INode();
    root.id = 0;
    root.refCount = 1; // ref count = 1 aby root nesel nikdy odstranit
    root.folder = true;
    root.timestamp = std::chrono::system_clock::now().time_since_epoch().count(); // nastaveni casu vytvoreni
    return root;
}

void INode::setId(uint32_t id) {
    INode::id = id;
}

void INode::setTimestamp(uint64_t timestamp) {
    INode::timestamp = timestamp;
}

void INode::printInfo() {
    if (folder) {
        std::cout << "INode with id: " << id << " folder " << "items: " << size << " refCount:" << refCount << std::endl;
    }
    else {
        std::cout << "Inode with id: " << id << " file " << "size in bytes: " << size << " refCount" << refCount << std::endl;
    }
//
//    bool checkNextBlock = t0AddressList[0] != Globals::INVALID_VALUE;
//    while (chec)
}
