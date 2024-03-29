
#include <chrono>
#include <iostream>
#include "INode.hpp"

uint32_t INode::getFolderSize() const { return size; }

INode::INode() = default;

bool INode::isFolder() const { return folder; }

const std::vector<uint64_t>& INode::getT0AddressList() const { return t0AddressList; }

uint64_t INode::getT1Address() const { return t1Address; }

uint64_t INode::getT2Address() const { return t2Address; }

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

void INode::setFolder(bool folder) {
    INode::folder = folder;
}

uint64_t INode::getRefCount() const {
    return refCount;
}

void INode::decrRefCount() {
    refCount -= 1;
}
