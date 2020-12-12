#include "INodeIO.hpp"
#include "../io/AddressType.h"


INodeIO::INodeIO(FileStream& fileStream, FileSystemController& fileSystemController) : fileStream(fileStream),
                                                                                       fileSystemController(
                                                                                               fileSystemController) {}

void INodeIO::appendToT1Block(uint64_t itemPosition, uint64_t t1Address, std::vector<uint64_t> allocations,
                              FolderItem& folderItem) {
    auto t1Index = itemPosition - (Globals::T0_ADDRESS_LIST_SIZE *
                                   Globals::FOLDER_ITEMS_PER_BLOCK()); // relativni index v t1 indirect bloku
    auto t1Row = t1Index / Globals::FOLDER_ITEM_SIZE_BYTES; // dany index v 1. neprime adrese
    auto blockRow = t1Index % Globals::FOLDER_ITEM_SIZE_BYTES; // pozice v bloku

    uint64_t pointer;
    fileStream.moveTo(t1Address + t1Row * Globals::POINTER_SIZE_BYTES); // precteme pointer
    fileStream.read(pointer);
    if (pointer == Globals::INVALID_VALUE) {
        // vytvoreni nove adresy
        auto address = fileSystemController.nextBlock(AddressType::FolderItem);
        allocations.push_back(address);
        pointer = address;
        // presuneme se na adresu pointeru a zapiseme
        fileStream.moveTo(t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
        fileStream.write(pointer);
    }

    fileStream.writeFolderItem(folderItem, pointer + blockRow * Globals::FOLDER_ITEM_SIZE_BYTES);
}


void INodeIO::appendToT2Block(uint32_t itemPosition, uint64_t t2Address, std::vector<uint64_t> allocations,
                              FolderItem& folderItem) {
    auto t2Index = itemPosition - Globals::T0_ADDRESS_LIST_SIZE -
                   Globals::POINTERS_PER_BLOCK() *
                   Globals::FOLDER_ITEMS_PER_BLOCK(); // index v t2 indirect bloku
    auto t2Row = t2Index / (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());

    uint64_t t1Pointer;
    fileStream.moveTo(t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
    fileStream.read(t1Pointer);
    if (t1Pointer == Globals::INVALID_VALUE) {
        auto address = fileSystemController.nextBlock(AddressType::Pointer);
        allocations.push_back(address);
        t1Pointer = address;
        fileStream.moveTo(t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
        fileStream.write(t1Pointer);
    }
    appendToT1Block(itemPosition, t1Pointer, allocations, folderItem);
}

void INodeIO::append(INode& node, FolderItem& folderItem) {
    auto itemPosition = node.getFolderSize(); // "index" v celkovem umisteni
    auto blockIndex = itemPosition / Globals::FOLDER_ITEMS_PER_BLOCK(); // index bloku

    // muze se stat ze na disku neni dostatek bloku a v nektere casti bloky dojdou, ale budou potreba dalsi - tzn.
    // aby nedoslo k "memory leaku" tak kazdou alokaci ulozime do vektoru a pokud memory allocator vyhodi FSException,
    // tak vsechny alokovane bloky vratime a vyhodime exception ze neslo predmet pridat kvuli nedostatku pameti
    auto blockAllocations = std::vector<uint64_t>();

    try {
        if (blockIndex < Globals::T0_ADDRESS_LIST_SIZE) {
            if (node.t0AddressList[blockIndex] == Globals::INVALID_VALUE) { // inicializace pokud je blok neinicializovany
                auto address = fileSystemController.nextBlock(AddressType::FolderItem);
                node.t0AddressList[blockIndex] = address;
                blockAllocations.push_back(address);
            }
            auto blockRow = itemPosition % Globals::FOLDER_ITEMS_PER_BLOCK(); // radka v datovem bloku
            fileStream.writeFolderItem(folderItem,
                                       node.t0AddressList[blockIndex] + blockRow * Globals::FOLDER_ITEM_SIZE_BYTES);
        } else if (itemPosition - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
            if (node.t1Address == Globals::INVALID_VALUE) {
                // pokud neexistuje 1. neprimy odkaz musime ho vytvorit
                auto t1Address = fileSystemController.nextBlock(AddressType::Pointer);
                node.t1Address = t1Address;
                blockAllocations.push_back(t1Address);
            }
            appendToT1Block(itemPosition, node.getT1Address(), blockAllocations, folderItem);
        } else {
            if (node.t2Address == Globals::INVALID_VALUE) {
                auto t2Address = fileSystemController.nextBlock(AddressType::Pointer);
                node.t2Address = t2Address;
                blockAllocations.push_back(t2Address);
            }
            appendToT2Block(itemPosition, node.getT2Address(), blockAllocations, folderItem);
        }

        // aktualizujeme INode ve filesystemu
        node.increaseFolderItems();
        fileSystemController.update(node);
    }
    catch (FSException& oom) {
        // preda filesystem controlleru alokovane bloky pro uvolneni
        fileSystemController.reclaimMemory(blockAllocations);
        // neni potreba dale nijak upravovat INode, protoze se INode v souboru neaktualizuje pokud nekdy v prubehu vyhodi
        // memory allocator FSException ze doslo misto
    }
}


//void INodeIO::append(INode& node, FolderItem& folderItem) {
//    auto itemPosition = node.getFolderSize();
//    auto row = itemPosition % Globals::FOLDER_ITEMS_PER_BLOCK();
//    auto blockIndex = row == 0 ?
//                      itemPosition / Globals::FOLDER_ITEMS_PER_BLOCK() :
//                      itemPosition / Globals::FOLDER_ITEMS_PER_BLOCK() + 1;
//
//    auto allocations = std::vector<uint64_t>();
//
//    try {
//        if (blockIndex < Globals::T0_ADDRESS_LIST_SIZE) {
//            if (node.getT0AddressList()[blockIndex] == Globals::INVALID_VALUE) {
//                auto address = fileSystemController.nextBlock(AddressType::FolderItem);
//                node.setDirectAddress(blockIndex, address);
//                allocations.push_back(address);
//            }
//            auto address = node.getT0AddressList()[blockIndex];
//            fileStream.writeFolderItem(folderItem, address + row * Globals::FOLDER_ITEM_SIZE_BYTES);
//        } else if (blockIndex - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
//            if (node.getT1Address() == Globals::INVALID_VALUE) {
//                auto address = fileSystemController.nextBlock(AddressType::Pointer);
//                node.setT1Address(address);
//                allocations.push_back(address);
//            }
//            auto t1Index = blockIndex - Globals::T0_ADDRESS_LIST_SIZE;
//            auto row = blockIndex - Globals::T0_ADDRESS_LIST_SIZE % Globals::FOLDER_ITEMS_PER_BLOCK();
//            if (row == 0) {
//                auto address = fileSystemController.nextBlock(AddressType::FolderItem);
//                fileStream.moveTo(node.getT1Address() + t1Index * Globals::POINTER_SIZE_BYTES);
//                fileStream.write(address);
//                fileStream.writeFolderItem(folderItem, address);
//                allocations.push_back(address);
//            } else {
//                uint64_t address;
//                fileStream.moveTo(node.getT1Address() + t1Index);
//                fileStream.read(address);
//                fileStream.moveTo(address + row * Globals::FOLDER_ITEM_SIZE_BYTES);
//                fileStream.writeFolderItem(folderItem, address);
//            }
//        } else {
//            auto t2Index = (blockIndex - Globals::T0_ADDRESS_LIST_SIZE) /
//                           (Globals::POINTERS_PER_BLOCK() * Globals::POINTERS_PER_BLOCK());
//            auto t1Index = (blockIndex - Globals::T0_ADDRESS_LIST_SIZE) / Globals::POINTERS_PER_BLOCK();
//            auto row = (blockIndex - Globals::T0_ADDRESS_LIST_SIZE) % Globals::FOLDER_ITEMS_PER_BLOCK();
//            if (node.getT2Address() == Globals::INVALID_VALUE) {
//                node.setT2Address(fileSystemController.nextBlock(AddressType::Pointer));
//                auto t1Address = fileSystemController.nextBlock(AddressType::Pointer);
//                fileStream.moveTo(node.getT2Address());
//                fileStream.write(t1Address);
//                allocations.push_back(t1Address);
//
//                auto blockAddress = fileSystemController.nextBlock(AddressType::FolderItem);
//                fileStream.moveTo(t1Address);
//                fileStream.write(blockAddress);
//                fileStream.writeFolderItem(folderItem, blockAddress);
//                allocations.push_back(blockAddress);
//            } else {
//                uint64_t t1Address;
//                fileStream.moveTo(node.getT2Address() + t2Index * Globals::POINTER_SIZE_BYTES);
//                fileStream.read(t1Address);
//
//                uint64_t blockAddress;
//                fileStream.moveTo(t1Address + t1Index * Globals::POINTER_SIZE_BYTES);
//                fileStream.read(blockAddress);
//
//                fileStream.writeFolderItem(folderItem, blockAddress + row * Globals::FOLDER_ITEM_SIZE_BYTES);
//            }
//        }
//
//        node.increaseFolderItems();
//        fileSystemController.update(node);
//
//    } catch (FSException& outOfMemoryBlocks) {
//        fileSystemController.reclaimMemory(allocations);
//    }
//
//}

std::pair<std::vector<INode>, std::vector<std::string>> INodeIO::getItems(INode& node) {
    if (node.getFolderSize() == 0) {
        return std::pair<std::vector<INode>, std::vector<std::string>>(std::vector<INode>(),
                                                                       std::vector<std::string>());
    }
    auto itemCount = node.getFolderSize();
    auto folderItems = std::vector<FolderItem>();
    folderItems.reserve(itemCount);

    auto blockCount = itemCount / Globals::FOLDER_ITEMS_PER_BLOCK();
    auto remainder = itemCount % Globals::FOLDER_ITEMS_PER_BLOCK();

    if (blockCount == 0) {
        readNFolderItems(folderItems, node.getT0AddressList()[0], itemCount);
    } else {
        if (remainder != 0) {
            blockCount += 1; // precteme jeden extra blok navic a pote odstranime prebytecne predmety
        }

        for (auto currBlockIndex = 0; currBlockIndex < blockCount; currBlockIndex++) {
            if (currBlockIndex < Globals::T0_ADDRESS_LIST_SIZE) {
                readFromBlockAddress(folderItems, node.getT0AddressList()[currBlockIndex]);
            } else if (currBlockIndex - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
                uint64_t blockAddress;
                auto t1Index = (currBlockIndex - Globals::T0_ADDRESS_LIST_SIZE);
                fileStream.moveTo(node.getT1Address() +
                                  t1Index * Globals::POINTER_SIZE_BYTES);
                fileStream.read(blockAddress);
                readFromBlockAddress(folderItems, blockAddress);
            } else {
                // vypocteme index v danem t2 bloku - tzn odecteme predchozi limity pro prime odkazy a 1. neprimy odkaz
                auto relativeBlockIndex =
                        currBlockIndex - Globals::T0_ADDRESS_LIST_SIZE - Globals::POINTERS_PER_BLOCK();
                auto t2Index = relativeBlockIndex / (Globals::POINTERS_PER_BLOCK() * Globals::POINTERS_PER_BLOCK());
                auto t1Index = relativeBlockIndex / Globals::POINTERS_PER_BLOCK();

                uint64_t t1Address;
                fileStream.moveTo(node.getT2Address() + t2Index * Globals::POINTER_SIZE_BYTES);
                fileStream.read(t1Address);

                uint64_t blockAddress;
                fileStream.moveTo(t1Address + t1Index * Globals::POINTER_SIZE_BYTES);
                fileStream.read(blockAddress);
                readFromBlockAddress(folderItems, blockAddress);
            }
        }

        auto toRemove = Globals::FOLDER_ITEMS_PER_BLOCK() - remainder;
        folderItems.resize(folderItems.size() - toRemove);
    }

    auto inodes = std::vector<INode>();
    auto itemNames = std::vector<std::string>();
    for (const auto& folderItem : folderItems) {
        itemNames.push_back(folderItem.getItemName());
        auto parent = INode();
        fileStream.moveTo(folderItem.getNodeAddress());
        fileStream.readINode(parent);
        inodes.push_back(parent);
    }

    return std::pair(inodes, itemNames);
}

void INodeIO::readFromBlockAddress(std::vector<FolderItem>& folderItems, uint64_t address) {
    auto blockItems = fileStream.readFolderItems(address);
    folderItems.insert(folderItems.end(), blockItems.begin(), blockItems.end());
}

void INodeIO::readNFolderItems(std::vector<FolderItem>& folderItems, uint64_t blockAddress, uint32_t count) {
    auto blockItems = fileStream.readNFolderItems(blockAddress, count);
    folderItems.insert(folderItems.end(), blockItems.begin(), blockItems.end());
}


