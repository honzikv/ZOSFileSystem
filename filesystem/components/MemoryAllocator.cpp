
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
    if (node.id == (uint32_t) Globals::INVALID_VALUE) {
        throw FSException("Error node does not have id"); // to by se nikdy nemelo stat, pouze pro debug
    }

    fileStream.moveTo(superBlock->nodeAddress + node.id * Globals::INODE_SIZE_BYTES());
    fileStream.writeINode(node); // update INode na disku
}

void MemoryAllocator::free(uint64_t address) {
    blockBitmap->setAddress(address, true);
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

