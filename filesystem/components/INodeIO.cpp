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

void INodeIO::append(INode& node, FolderItem& folderItem, bool increaseRefCount) {
    if (node.getFolderSize() == Globals::MAX_FOLDER_ITEMS()) {
        throw FSException("Error, this INode cannot hold more files / folders");
    }

    auto itemPosition = node.getFolderSize(); // "index" v celkovem umisteni
    auto blockIndex = itemPosition / Globals::FOLDER_ITEMS_PER_BLOCK(); // index bloku

    // muze se stat ze na disku neni dostatek bloku a v nektere casti bloky dojdou, ale budou potreba dalsi - tzn.
    // aby nedoslo k "memory leaku" tak kazdou alokaci ulozime do vektoru a pokud memory allocator vyhodi FSException,
    // tak vsechny alokovane bloky vratime a vyhodime exception ze neslo predmet pridat kvuli nedostatku pameti
    auto blockAllocations = std::vector<uint64_t>();

    try {
        if (blockIndex < Globals::T0_ADDRESS_LIST_SIZE) {
            if (node.t0AddressList[blockIndex] == Globals::INVALID_VALUE) {
                // inicializace pokud je blok neinicializovany
                auto address = fileSystemController.nextBlock(AddressType::FolderItem);
                node.t0AddressList[blockIndex] = address;
                blockAllocations.push_back(address);
            }
            auto blockRow = itemPosition % Globals::FOLDER_ITEMS_PER_BLOCK(); // radka v datovem bloku
            fileStream.writeFolderItem(folderItem,
                                       node.t0AddressList[blockIndex] + blockRow * Globals::FOLDER_ITEM_SIZE_BYTES);
        } else if ((blockIndex - Globals::T0_ADDRESS_LIST_SIZE) < Globals::POINTERS_PER_BLOCK()) {
            if (node.t1Address == Globals::INVALID_VALUE) {
                // pokud neexistuje 1. neprimy odkaz musime ho vytvorit
                auto t1Address = fileSystemController.nextBlock(AddressType::Pointer);
                node.t1Address = t1Address;
                blockAllocations.push_back(t1Address);
            }
            appendToT1Block(itemPosition, node.getT1Address(), blockAllocations, folderItem);
        } else {
            if (node.t2Address == Globals::INVALID_VALUE) {
                // pokud neexistuje 2. neprimy odkaz musime ho vytvorit
                auto t2Address = fileSystemController.nextBlock(AddressType::Pointer);
                node.t2Address = t2Address;
                blockAllocations.push_back(t2Address);
            }
            appendToT2Block(itemPosition, node.getT2Address(), blockAllocations, folderItem);
        }

        // zvysime ref count, resp. zvysime pokud se nejedna o reference ".." a "."
        if (increaseRefCount) {
            node.incrRefCount(); // zvyseni poctu referenci na INode
        }
        node.incrFolderItemCount(); // zvyseni poctu predmetu ve slozce
        fileSystemController.refresh(node); // aktualizace INode pro PathContext a v souboru
    }
    catch (FSException& oom) {
        // preda filesystem controlleru alokovane bloky pro uvolneni
        fileSystemController.reclaimMemory(blockAllocations);
        // neni potreba dale nijak upravovat INode, protoze se INode v souboru neaktualizuje pokud nekdy v prubehu vyhodi
        // memory allocator FSException ze doslo misto
        throw FSException("FolderItem could not have been added");
    }
}


std::vector<FolderItem> INodeIO::getFolderItems(INode& node) {
    auto remainingItems = node.getFolderSize();
    auto result = std::vector<FolderItem>();
    result.reserve(node.getFolderSize());

    if (remainingItems == 0) {
        return result;
    }

    auto t0Capacity = Globals::T0_ADDRESS_LIST_SIZE * Globals::FOLDER_ITEMS_PER_BLOCK();
    auto t1Capacity = Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK();

    auto itemCount = remainingItems < t0Capacity ? remainingItems : t0Capacity;
    auto t0Items = std::vector<FolderItem>();
    t0Items.reserve(itemCount);
    readFromDirectBlocks(node.t0AddressList, itemCount, t0Items);


    auto t1Items = std::vector<FolderItem>();
    remainingItems -= t0Items.size();
    if (remainingItems > 0) {
        itemCount = remainingItems < t1Capacity ? remainingItems : t1Capacity;
        t1Items.reserve(itemCount);
        readFromT1Address(node.t1Address, itemCount, t1Items);
    }


    auto t2Items = std::vector<FolderItem>();
    remainingItems -= t1Items.size();
    if (remainingItems > 0) {
        t2Items.reserve(remainingItems);
        readFromT2Address(node.t2Address, remainingItems, t2Items);
    }

    result.insert(result.end(), t0Items.begin(), t0Items.end());
    result.insert(result.end(), t1Items.begin(), t1Items.end());
    result.insert(result.end(), t2Items.begin(), t2Items.end());

    return result;
}


void INodeIO::readFromBlockAddress(std::vector<FolderItem>& folderItems, uint64_t address) {
    auto blockItems = fileStream.readFolderItemBlock(address);
    folderItems.insert(folderItems.end(), blockItems.begin(), blockItems.end());
}

void INodeIO::readNFolderItems(std::vector<FolderItem>& folderItems, uint64_t blockAddress, uint32_t count) {
    auto blockItems = fileStream.readNFolderItems(blockAddress, count);
    folderItems.insert(folderItems.end(), blockItems.begin(), blockItems.end());
}

void INodeIO::readFromDirectBlocks(std::vector<uint64_t> addressList, uint32_t count, std::vector<FolderItem>& result) {
    auto fullBlocks = count / Globals::FOLDER_ITEMS_PER_BLOCK();
    auto remainder = count % Globals::FOLDER_ITEMS_PER_BLOCK();

    for (auto blockIndex = 0; blockIndex < fullBlocks; blockIndex += 1) {
        auto items = fileStream.readFolderItemBlock(addressList[blockIndex]);
        result.insert(result.end(), items.begin(), items.end());
    }

    auto remainderBlockIndex = fullBlocks + 1;
    if (fullBlocks == 0) {
        remainderBlockIndex = 0;
    }

    auto remainderItems = fileStream.readNFolderItems(addressList[remainderBlockIndex], remainder);
    result.insert(result.end(), remainderItems.begin(), remainderItems.end());
}

void INodeIO::readFromT1Address(uint64_t t1Address, uint64_t count, std::vector<FolderItem> result) {
    auto addressList = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
    fileStream.moveTo(t1Address);
    fileStream.readVector(addressList);
    readFromDirectBlocks(addressList, count, result);
}

void INodeIO::readFromT2Address(uint64_t t2Address, uint32_t itemCount, std::vector<FolderItem> result) {
    auto fullBlocks = itemCount / (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());
    auto remainder = itemCount % (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());

    auto t1BlockList = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
    fileStream.moveTo(t2Address);
    fileStream.readVector(t1BlockList);

    auto blockItemCount = Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK();
    for (auto blockIndex = 0; blockIndex < fullBlocks; blockIndex += 1) {
        readFromT1Address(t1BlockList[blockIndex], blockItemCount, result);
    }

    auto remainderBlockIndex = fullBlocks + 1;
    if (fullBlocks == 0) {
        remainderBlockIndex = 0;
    }

    readFromT1Address(t1BlockList[remainderBlockIndex], remainder, result);
}

void INodeIO::linkFolderToParent(INode& current, uint64_t currentNodeAddress, uint64_t parentNodeAddress) {
    auto dot = FolderItem(Globals::CURRENT_FOLDER_SYMBOL, currentNodeAddress, true);
    auto dotDot = FolderItem(Globals::PARENT_FOLDER_SYMBOL, parentNodeAddress, true);
    append(current, dot, false);
    append(current, dotDot, false);
}



