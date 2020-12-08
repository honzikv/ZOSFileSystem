
#include "INode.hpp"
#include "../../util/FStreamWrapper.hpp"

uint32_t INode::getFolderSize() { return size; }

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

FStreamWrapper& operator<<(FStreamWrapper& fs, INode& node) {
    fs.write(node.folder, node.id, node.size, node.timestamp, node.refCount,
             node.t1Address, node.t2Address);
    fs.writeVector(node.t0AddressList);
    return fs;
}
FStreamWrapper& operator>>(FStreamWrapper& fs, INode& node) {
    fs.read(node.folder, node.id, node.size, node.timestamp, node.refCount,
            node.t1Address, node.t2Address);
    auto vector = std::vector<uint64_t>(Globals::T0_ADDRESS_LIST_SIZE, Globals::INVALID_VALUE);
    fs.readVector(vector);
    node.t0AddressList = vector;
    return fs;
}
