
#include "MemoryAllocator.hpp"


MemoryAllocator::MemoryAllocator(std::shared_ptr<SuperBlock> superBlock, FileStream& fileStream) : fileStream(
        fileStream) {
    this->superBlock = superBlock;
    blockBitmap = std::make_shared<Bitmap>(superBlock->blockBitmapAddress, superBlock->blockCount,
                                           superBlock->dataAddress, Globals::BLOCK_SIZE_BYTES, fileStream, "blockBitmap");
    nodeBitmap = std::make_shared<Bitmap>(superBlock->nodeBitmapAddress, superBlock->nodeCount, superBlock->nodeAddress,
                                          Globals::INODE_SIZE_BYTES(), fileStream, "nodeBitmap");
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

    auto nodeAddress = superBlock->nodeAddress + node.getId() * Globals::INODE_SIZE_BYTES();
    auto emptyNode = INode();
    fileStream.moveTo(nodeAddress);
    fileStream.writeINode(emptyNode); // update INode na disku
    nodeBitmap->setAddress(nodeAddress, true);
    freeMemory(node);
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
    auto nodeAddress = nodeBitmap->getFirstEmptyAddress();
    auto id = nodeBitmap->getIdFromAddress(nodeAddress);
    auto result = INode();
    result.setId(id);
    result.setFolder(isFolder);
    result.setTimestamp(std::chrono::system_clock::now().time_since_epoch().count());
    nodeBitmap->setAddress(nodeAddress, false);
    return result;
}

std::vector<uint64_t> MemoryAllocator::getNDataBlocks(uint64_t n, AddressType addressType) {
    auto dataBlocks = std::vector<uint64_t>();
    dataBlocks.reserve(n);
    for (auto i = 0; i < n; i++) {
        try {
            auto emptyAddress = getDataBlock(addressType);
            dataBlocks.push_back(emptyAddress);
        }
        catch (FSException& ex) {
            for (auto block : dataBlocks) {
                blockBitmap->setAddress(block, true);
                superBlock->freeBlocks += 1;
            }

            throw FSException(ex.what());
        }
    }

    return dataBlocks;
}

uint64_t MemoryAllocator::getNodeAddress(INode& node) {
    if (node.getId() == (uint32_t) Globals::INVALID_VALUE) {
        throw FSException("Error, node id is invalid"); // nemelo by se stat, pouze pro debug
    }

    return superBlock->nodeAddress + node.getId() * Globals::INODE_SIZE_BYTES();
}

void MemoryAllocator::freeMemory(INode& node) {
    for (auto address : node.getT0AddressList()) {
        if (address == Globals::INVALID_VALUE) {
            break;
        }
        freeMemory(address);
    }

    freeT1Address(node.getT1Address()); // vycisteni 1. neprimeho odkazu pokud existuje
    freeT2Address(node.getT2Address()); // vycisteni 2. neprimeho odkazu pokud existuje
}

void MemoryAllocator::freeT1Address(uint64_t t1Address) {
    if (t1Address == Globals::INVALID_VALUE) {
        return;
    }

    fileStream.moveTo(t1Address);
    auto t1AddressList = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
    fileStream.readVector(t1AddressList);

    for (auto address : t1AddressList) {
        if (address == Globals::INVALID_VALUE) { // pokud je adresa neplatna, vratime se
            break;
        }

        freeMemory(address); // jinak oznacime jako volnou a pokracujeme
    }

    freeMemory(t1Address); // dale oznacime blok s adresami jako volny
}

void MemoryAllocator::freeT2Address(uint64_t t2Address) {
    if (t2Address == Globals::INVALID_VALUE) { // pokud neexistuje navrat zpet
        return;
    }

    fileStream.moveTo(t2Address); // presun na t2 adresu a nacteni dat
    auto t2AddressList = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
    fileStream.readVector(t2AddressList);

    for (auto t1Address : t2AddressList) { // iterujeme a pro kazdy odkaz pouzijeme freeT1Address funkci
        if (t1Address == Globals::INVALID_VALUE) {
            break;
        }

        freeT1Address(t1Address);
    }

    freeMemory(t2Address); // nakonec dealokujeme i 2. neprimy odkaz
}



