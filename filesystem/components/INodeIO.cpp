#include "INodeIO.hpp"
#include "../io/AddressType.h"


INodeIO::INodeIO(FileStream& fileStream, FileSystemController& fileSystemController)
        : fstream(fileStream),
          fileSystemController(fileSystemController) {}

void INodeIO::append(INode& node, FolderItem& folderItem) {
    auto itemPosition = node.getFolderSize();
    auto row = itemPosition % Globals::FOLDER_ITEMS_PER_BLOCK();
    auto blockIndex = row == 0 ?
                      itemPosition / Globals::FOLDER_ITEMS_PER_BLOCK() :
                      itemPosition / Globals::FOLDER_ITEMS_PER_BLOCK() + 1;

    auto allocations = std::vector<uint64_t>();

    try {
        if (blockIndex < Globals::T0_ADDRESS_LIST_SIZE) {
            if (node.getT0AddressList()[blockIndex] == Globals::INVALID_VALUE) {
                auto address = fileSystemController.nextBlock(AddressType::FolderItem);
                node.setDirectAddress(blockIndex, address);
                allocations.push_back(address);
            }
            auto address = node.getT0AddressList()[blockIndex];
            fstream.writeFolderItem(folderItem, address + row * Globals::FOLDER_ITEM_SIZE_BYTES);
        } else if (blockIndex - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
            if (node.getT1Address() == Globals::INVALID_VALUE) {
                auto address = fileSystemController.nextBlock(AddressType::Pointer);
                node.setT1Address(address);
                allocations.push_back(address);
            }
            auto t1Index = blockIndex - Globals::T0_ADDRESS_LIST_SIZE;
            auto row = blockIndex - Globals::T0_ADDRESS_LIST_SIZE % Globals::FOLDER_ITEMS_PER_BLOCK();
            if (row == 0) {
                auto address = fileSystemController.nextBlock(AddressType::FolderItem);
                fstream.moveTo(node.getT1Address() + t1Index * Globals::POINTER_SIZE_BYTES);
                fstream.write(address);
                fstream.writeFolderItem(folderItem, address);
                allocations.push_back(address);
            } else {
                uint64_t address;
                fstream.moveTo(node.getT1Address() + t1Index);
                fstream.read(address);
                fstream.moveTo(address + row * Globals::FOLDER_ITEM_SIZE_BYTES);
                fstream.writeFolderItem(folderItem, address);
            }
        } else {
            auto t2Index = (blockIndex - Globals::T0_ADDRESS_LIST_SIZE) /
                           (Globals::POINTERS_PER_BLOCK() * Globals::POINTERS_PER_BLOCK());
            auto t1Index = (blockIndex - Globals::T0_ADDRESS_LIST_SIZE) / Globals::POINTERS_PER_BLOCK();
            auto row = (blockIndex - Globals::T0_ADDRESS_LIST_SIZE) % Globals::FOLDER_ITEMS_PER_BLOCK();
            if (node.getT2Address() == Globals::INVALID_VALUE) {
                node.setT2Address(fileSystemController.nextBlock(AddressType::Pointer));
                auto t1Address = fileSystemController.nextBlock(AddressType::Pointer);
                fstream.moveTo(node.getT2Address());
                fstream.write(t1Address);
                allocations.push_back(t1Address);

                auto blockAddress = fileSystemController.nextBlock(AddressType::FolderItem);
                fstream.moveTo(t1Address);
                fstream.write(blockAddress);
                fstream.writeFolderItem(folderItem, blockAddress);
                allocations.push_back(blockAddress);
            } else {
                uint64_t t1Address;
                fstream.moveTo(node.getT2Address() + t2Index * Globals::POINTER_SIZE_BYTES);
                fstream.read(t1Address);

                uint64_t blockAddress;
                fstream.moveTo(t1Address + t1Index * Globals::POINTER_SIZE_BYTES);
                fstream.read(blockAddress);

                fstream.writeFolderItem(folderItem, blockAddress + row * Globals::FOLDER_ITEM_SIZE_BYTES);
            }
        }

        node.increaseFolderItems();
        fileSystemController.update(node);

    } catch (FSException& outOfMemoryBlocks) {
        fileSystemController.reclaimMemory(allocations);
    }

}

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
        readFromBlockAddress(folderItems,
                             node.getT0AddressList()[0]); // precteme prvni blok a odstranime prebytecne predmety
        auto toRemove = Globals::FOLDER_ITEMS_PER_BLOCK() - remainder;
        folderItems.resize(folderItems.size() - toRemove);

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
                fstream.moveTo(node.getT1Address() +
                               t1Index * Globals::POINTER_SIZE_BYTES);
                fstream.read(blockAddress);
                readFromBlockAddress(folderItems, blockAddress);
            } else {
                // vypocteme index v danem t2 bloku - tzn odecteme predchozi limity pro prime odkazy a 1. neprimy odkaz
                auto relativeBlockIndex =
                        currBlockIndex - Globals::T0_ADDRESS_LIST_SIZE - Globals::POINTERS_PER_BLOCK();
                auto t2Index = relativeBlockIndex / (Globals::POINTERS_PER_BLOCK() * Globals::POINTERS_PER_BLOCK());
                auto t1Index = relativeBlockIndex / Globals::POINTERS_PER_BLOCK();

                uint64_t t1Address;
                fstream.moveTo(node.getT2Address() + t2Index * Globals::POINTER_SIZE_BYTES);
                fstream.read(t1Address);

                uint64_t blockAddress;
                fstream.moveTo(t1Address + t1Index * Globals::POINTER_SIZE_BYTES);
                fstream.read(blockAddress);
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
        fstream.moveTo(folderItem.getNodeAddress());
        fstream.readINode(parent);
        inodes.push_back(parent);
    }
    return std::pair(inodes, itemNames);
}

void INodeIO::readFromBlockAddress(std::vector<FolderItem>& folderItems, uint64_t address) {
    auto blockItems = fstream.readFolderItems(address);
    folderItems.insert(folderItems.end(), blockItems.begin(), blockItems.end());
}

