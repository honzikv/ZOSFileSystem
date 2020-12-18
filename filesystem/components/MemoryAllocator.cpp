
#include "MemoryAllocator.hpp"


MemoryAllocator::MemoryAllocator(std::shared_ptr<SuperBlock> superBlock, FileStream& fileStream) : fileStream(
        fileStream) {
    this->superBlock = superBlock;
    blockBitmap = std::make_shared<Bitmap>(superBlock->blockBitmapAddress, superBlock->blockCount,
                                           superBlock->dataAddress, Globals::BLOCK_SIZE_BYTES, fileStream);
    nodeBitmap = std::make_shared<Bitmap>(superBlock->nodeBitmapAddress, superBlock->nodeCount, superBlock->nodeAddress,
                                          Globals::INODE_SIZE_BYTES(), fileStream);

}

void MemoryAllocator::update(INode& node) {
    if (node.getId() == (uint32_t) Globals::INVALID_VALUE) {
        throw FSException("Error, node does not have id"); // to by se nikdy nemelo stat, pouze pro debug
    }

    auto nodeAddress = superBlock->nodeAddress + node.getId() * Globals::INODE_SIZE_BYTES();
    fileStream.moveTo(nodeAddress);
    fileStream.writeINode(node); // update INode na disku
    nodeBitmap->setAddress(nodeAddress, false);
}

void MemoryAllocator::freeMemory(uint64_t address) {
    blockBitmap->setAddress(address, true);
}

INode MemoryAllocator::getINodeWithId(uint32_t nodeId) {
    if (nodeId == (uint32_t) Globals::INVALID_VALUE) {
        throw FSException("Error, node does not have id"); // to by se nikdy nemelo stat, pouze pro debug
    }
    auto nodeWithId = INode();
    auto nodeAddress = superBlock->nodeAddress + nodeId * Globals::INODE_SIZE_BYTES();
    fileStream.moveTo(nodeAddress);
    fileStream.readINode(nodeWithId);
    return nodeWithId;
}

void MemoryAllocator::freeINode(INode& node) {
    if (node.getId() == (uint32_t) Globals::INVALID_VALUE) {
        throw FSException("Error, node does not have id"); // to by se nikdy nemelo stat, pouze pro debug
    }

    node.setId((uint32_t) Globals::INVALID_VALUE);
    auto nodeAddress = superBlock->nodeAddress + node.getId() * Globals::INODE_SIZE_BYTES();
    fileStream.moveTo(nodeAddress);
    fileStream.writeINode(node); // update INode na disku
    nodeBitmap->setAddress(nodeAddress, true);
}

void MemoryAllocator::format(uint64_t address, AddressType addressType) {
    if (addressType == AddressType::Pointer) {
        fileStream.moveTo(address);
        auto invalidPointer = Globals::INVALID_VALUE; // potreba pro referenci do fileStream.write
        for (auto i = 0; i < Globals::POINTERS_PER_BLOCK(); i++) {
            fileStream.write(invalidPointer); // pointer je 64 bitove cislo stejne jako Globals::INVALID_VALUE
        }
    } else if (addressType == AddressType::FolderItem) {
        fileStream.moveTo(address);
        auto emptyFolderItem = FolderItem();
        for (auto i = 0; i < Globals::FOLDER_ITEMS_PER_BLOCK(); i++) {
            fileStream.write(emptyFolderItem);
        }
    }
}

uint64_t MemoryAllocator::getDataBlock(AddressType addressType) {
    auto address = blockBitmap->getFirstEmptyAddress();
    blockBitmap->setAddress(address, false);
    format(address, addressType);
    return address;
}

INode MemoryAllocator::getINode(bool isFolder) {
    auto nodeAddress = blockBitmap->getFirstEmptyAddress();
    auto id = blockBitmap->getIdFromAddress(nodeAddress);
    auto result = INode();
    result.setId(id);
    result.setFolder(isFolder);
    result.setTimestamp(std::chrono::system_clock::now().time_since_epoch().count());
    blockBitmap->setAddress(nodeAddress, false);
    return result;
}

std::vector<uint64_t> MemoryAllocator::getNDataBlocks(uint64_t n, AddressType addressType) {
    auto dataBlocks = std::vector<uint64_t>();
    dataBlocks.reserve(n);
    for (auto i = 0; i < n; i++) {
        dataBlocks.push_back(blockBitmap->getFirstEmptyAddress()); // muze vyhodit exception pokud nebude misto
    }

    for (auto dataBlock : dataBlocks) {
        blockBitmap->setAddress(dataBlock, false);
        format(dataBlock, addressType);
    }

    return dataBlocks;
}

uint64_t MemoryAllocator::getNodeAddress(INode& node) {
    if (node.getId() == (uint32_t) Globals::INVALID_VALUE) {
        throw FSException("Error, node id is invalid"); // nemelo by se stat, pouze pro debug
    }

    return superBlock->nodeAddress + node.getId() * Globals::INODE_SIZE_BYTES();
}


