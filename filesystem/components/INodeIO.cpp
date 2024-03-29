#include "INodeIO.hpp"


INodeIO::INodeIO(FileStream& fileStream, FileSystemController& fileSystemController) : fileStream(fileStream),
                                                                                       fileSystemController(
                                                                                               fileSystemController) {}

void INodeIO::appendToT1Block(uint64_t itemPosition, uint64_t t1Address, std::vector<uint64_t>& allocations,
                              FolderItem& folderItem) {
    auto t1Row = itemPosition / Globals::FOLDER_ITEMS_PER_BLOCK(); // dany index v 1. neprime adrese
    auto blockRow = itemPosition % Globals::FOLDER_ITEMS_PER_BLOCK(); // pozice v bloku

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


void INodeIO::appendToT2Block(uint32_t itemPosition, uint64_t t2Address, std::vector<uint64_t>& allocations,
                              FolderItem& folderItem) {
    auto t2Row = itemPosition / (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());

    uint64_t t1Pointer; // precteme pointer z bloku
    fileStream.moveTo(t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
    fileStream.read(t1Pointer);
    if (t1Pointer == Globals::INVALID_VALUE) { // potrebujeme alokovat blok, protoze jeste nebyl alokovany
        auto address = fileSystemController.nextBlock(AddressType::Pointer);
        allocations.push_back(address);
        t1Pointer = address;
        fileStream.moveTo(t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
        fileStream.write(t1Pointer);
    }

    auto relativePosition = itemPosition % (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());
    appendToT1Block(relativePosition, t1Pointer, allocations, folderItem);
}


void INodeIO::writeAt(INode& node, uint32_t index, FolderItem& folderItem) {
    // write na jiz alokovanou pozici lze zaridit pomoci INodeIO::append, kdy zmenime velikost INode na index, kam
    // chceme prvek zapsat
    auto swap = node.size;
    node.size = index;
    appendFolderItem(node, folderItem, false);
    node.size = swap;
}

void INodeIO::copyData(INode& source, INode& dest) {
    auto bytes = source.size;
    auto dataBlocksRequired = Globals::getBlockCount(bytes);
    auto extraBlocksRequired = getExtraBlocks(bytes);

    auto dataBlocks = fileSystemController.nextNBlocks(dataBlocksRequired, AddressType::RawData);
    auto pointerBlocks = fileSystemController.nextNBlocks(extraBlocksRequired, AddressType::Pointer);
    auto pointerBlockIndex = 0;

    auto buffer = std::vector<char>(Globals::BLOCK_SIZE_BYTES, '\0');
    auto bytesRemaining = bytes;

    for (auto currBlockIndex = 0; currBlockIndex < dataBlocksRequired; currBlockIndex += 1) {
        if (bytesRemaining < Globals::BLOCK_SIZE_BYTES) {
            // pokud zbyva mene bytu nez je velikost bloku musime precist mensi mnozstvi
            buffer = std::vector<char>(bytesRemaining, '\0');
            bytesRemaining = 0;
        } else {
            bytesRemaining -= Globals::BLOCK_SIZE_BYTES; // odecteme velikost bloku
        }

        // podle aktualniho indexu bloku rozhodneme, kam se bude zapisovat
        if (currBlockIndex < Globals::T0_ADDRESS_LIST_SIZE) {
            fileStream.moveTo(source.t0AddressList[currBlockIndex]);
            fileStream.readVector(buffer);

            dest.t0AddressList[currBlockIndex] = dataBlocks[currBlockIndex];
            fileStream.moveTo(dest.t0AddressList[currBlockIndex]);
            fileStream.writeVector(buffer);
        } else if (currBlockIndex - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
            if (dest.t1Address == Globals::INVALID_VALUE) {
                dest.t1Address = pointerBlocks[pointerBlockIndex];
                pointerBlockIndex += 1;
            }

            auto t1Row = currBlockIndex - Globals::T0_ADDRESS_LIST_SIZE; // relativni radek v t1 odkazu
            fileStream.moveTo(source.t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.readVector(buffer);

            fileStream.moveTo(dest.t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.writeVector(buffer);
        } else {
            if (dest.t2Address == Globals::INVALID_VALUE) {
                dest.t2Address = pointerBlocks[pointerBlockIndex];
                pointerBlockIndex += 1;
            }

            // relativni radek v t2 bloku
            auto relativeBlockIndex = currBlockIndex - Globals::T0_ADDRESS_LIST_SIZE - Globals::POINTERS_PER_BLOCK();
            auto t2Row = relativeBlockIndex / Globals::POINTERS_PER_BLOCK();
            auto t1Row = relativeBlockIndex % Globals::POINTERS_PER_BLOCK();

            if (t1Row == 0) {
                fileStream.moveTo(dest.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
                fileStream.write(pointerBlocks[pointerBlockIndex]);
                pointerBlockIndex += 1;
            }

            // cteni t1 adresy
            uint64_t t1Address;
            fileStream.moveTo(source.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(t1Address);

            // cteni adresy bloku
            uint64_t blockAddress;
            fileStream.moveTo(t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(blockAddress);

            fileStream.moveTo(blockAddress);
            fileStream.readVector(buffer);

            fileStream.moveTo(dest.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(t1Address);
            fileStream.moveTo(t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.write(dataBlocks[currBlockIndex]);

            fileStream.moveTo(dataBlocks[currBlockIndex]);
            fileStream.writeVector(buffer);
        }
    }

    dest.size = bytes;
    fileSystemController.refresh(dest);
}

void INodeIO::appendExternalFile(INode& parent, INode& node, FolderItem& folderItem, FileStream& externalFileStream) {
    auto bytes = externalFileStream.getFileSize();
    auto dataBlocksRequired = Globals::getBlockCount(bytes); // pocet bloku, kde jsou pouze data
    auto extraBlocksRequired = getExtraBlocks(bytes); // pocet bloku navic - bloky s pointery

    if (dataBlocksRequired + extraBlocksRequired > Globals::maxAddressableBlocksPerINode()) {
        throw FSException("Error, file is too large to contain in the filesystem");
    }

    auto dataBlocks = fileSystemController.nextNBlocks(dataBlocksRequired, AddressType::RawData);
    auto pointerBlocks = fileSystemController.nextNBlocks(extraBlocksRequired, AddressType::Pointer);
    auto pointerBlockIndex = 0;

    auto buffer = std::vector<char>(Globals::BLOCK_SIZE_BYTES, '\0');
    auto bytesRemaining = bytes;

    externalFileStream.moveTo(0);
    for (auto currentBlock = 0; currentBlock < dataBlocksRequired; currentBlock += 1) {
        if (bytesRemaining < Globals::BLOCK_SIZE_BYTES) {
            // pokud zbyva mene bytu nez je velikost bloku musime precist mensi mnozstvi (jinak se fstream rozbije)
            buffer = std::vector<char>(bytesRemaining, '\0');
            bytesRemaining = 0;
        } else {
            bytesRemaining -= Globals::BLOCK_SIZE_BYTES; // odecteme velikost bloku
        }
        externalFileStream.readVector(buffer); // precteme velikost bloku ze souboru

        if (currentBlock < Globals::T0_ADDRESS_LIST_SIZE) { // pokud je blok primy odkaz
            node.t0AddressList[currentBlock] = dataBlocks[currentBlock];
            fileStream.moveTo(dataBlocks[currentBlock]);
            fileStream.writeVector(buffer);
        } else if (currentBlock - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
            // pokud je blok v rozmezi 1. neprimeho odkazu
            if (node.t1Address == Globals::INVALID_VALUE) {
                node.t1Address = pointerBlocks[pointerBlockIndex];
                pointerBlockIndex += 1;
            }

            auto t1Row = currentBlock - Globals::T0_ADDRESS_LIST_SIZE; // relativni radek v 1. neprimeme odkazu
            fileStream.moveTo(node.t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.write(dataBlocks[currentBlock]);

            fileStream.moveTo(dataBlocks[currentBlock]);
            fileStream.writeVector(buffer);
        } else {
            if (node.t2Address == Globals::INVALID_VALUE) {
                node.t2Address = pointerBlocks[pointerBlockIndex];
                pointerBlockIndex += 1;
            }

            auto relativeBlock = currentBlock - Globals::T0_ADDRESS_LIST_SIZE - Globals::POINTERS_PER_BLOCK();
            auto t2Row = relativeBlock / Globals::POINTERS_PER_BLOCK();
            auto t1Row = relativeBlock % Globals::POINTERS_PER_BLOCK();


            if (t1Row == 0) {
                fileStream.moveTo(node.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
                fileStream.write(pointerBlocks[pointerBlockIndex]);
                pointerBlockIndex += 1;
            }

            uint64_t t1Address;
            fileStream.moveTo(node.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(t1Address);

            fileStream.moveTo(t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.write(dataBlocks[currentBlock]);

            fileStream.moveTo(dataBlocks[currentBlock]);
            fileStream.writeVector(buffer);
        }
    }
    node.size = bytes;
    fileSystemController.refresh(node);
    appendFolderItem(parent, folderItem);
}

void INodeIO::appendFolderItem(INode& node, FolderItem& folderItem, bool increaseRefCount) {
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
            auto t1ItemPosition = itemPosition - Globals::T0_ADDRESS_LIST_SIZE * Globals::FOLDER_ITEMS_PER_BLOCK();
            appendToT1Block(t1ItemPosition, node.getT1Address(), blockAllocations, folderItem);
        } else {
            if (node.t2Address == Globals::INVALID_VALUE) {
                // pokud neexistuje 2. neprimy odkaz musime ho vytvorit
                auto t2Address = fileSystemController.nextBlock(AddressType::Pointer);
                node.t2Address = t2Address;
                blockAllocations.push_back(t2Address);
            }
            auto t2ItemPosition = itemPosition - Globals::T0_ADDRESS_LIST_SIZE * Globals::FOLDER_ITEMS_PER_BLOCK() -
                                  Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK();
            appendToT2Block(t2ItemPosition, node.getT2Address(), blockAllocations, folderItem);
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
        throw FSException("Error, not enough memory");
    }
}


std::vector<FolderItem> INodeIO::getFolderItems(INode& node) {
    auto remainingItems = node.getFolderSize(); // zbyvajici pocet predmetu - aby se vedelo, kdy prestat
    auto result = std::vector<FolderItem>(); // vysledek - tento vektor obsahuje predmety slozky a vraci se
    result.reserve(node.getFolderSize());

    if (remainingItems == 0) {
        return result;
    }

    auto t0Capacity = Globals::T0_ADDRESS_LIST_SIZE * Globals::FOLDER_ITEMS_PER_BLOCK(); // kapacita primych odkazu
    auto t1Capacity = Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK(); // kapacita 1. neprimeho odkazu

    auto itemCount = remainingItems < t0Capacity ? remainingItems : t0Capacity; // pocet predmetu, ktere se maji precist
    auto t0Items = std::vector<FolderItem>(); // predmety z primych odkazu
    t0Items.reserve(itemCount);
    readFromDirectBlocks(node.t0AddressList, itemCount, t0Items);


    auto t1Items = std::vector<FolderItem>(); // predmety z 1. neprimeho odkazu
    remainingItems -= t0Items.size();
    if (remainingItems > 0) {
        itemCount = remainingItems < t1Capacity ? remainingItems : t1Capacity;
        t1Items.reserve(itemCount);
        readFromT1Address(node.t1Address, itemCount, t1Items);
    }


    auto t2Items = std::vector<FolderItem>(); // predmety z 2. neprimeho odkazu
    remainingItems -= t1Items.size();
    if (remainingItems > 0) {
        t2Items.reserve(remainingItems);
        readFromT2Address(node.t2Address, remainingItems, t2Items);
    }

    // nakonec predmety z primych a neprimych odkazu pridame
    result.insert(result.end(), t0Items.begin(), t0Items.end());
    result.insert(result.end(), t1Items.begin(), t1Items.end());
    result.insert(result.end(), t2Items.begin(), t2Items.end());

    return result;
}


void INodeIO::readFromDirectBlocks(std::vector<uint64_t>& addressList, uint32_t count, std::vector<FolderItem>& result) {
    auto fullBlocks = count / Globals::FOLDER_ITEMS_PER_BLOCK(); // pocet plnych bloku
    auto remainder = count % Globals::FOLDER_ITEMS_PER_BLOCK(); // zbytek

    for (auto blockIndex = 0; blockIndex < fullBlocks; blockIndex += 1) {
        auto items = fileStream.readFolderItemBlock(addressList[blockIndex]); // precteme blok zaznamu
        result.insert(result.end(), items.begin(), items.end()); // pridame do vysledku
    }

    auto remainderBlockIndex = fullBlocks + 1;
    if (fullBlocks == 0) {
        remainderBlockIndex = 0;
    }

    auto remainderItems = fileStream.readNFolderItems(addressList[remainderBlockIndex], remainder);
    result.insert(result.end(), remainderItems.begin(), remainderItems.end());
}

void INodeIO::readFromT1Address(uint64_t t1Address, uint64_t count, std::vector<FolderItem>& result) {
    auto addressList = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
    fileStream.moveTo(t1Address); // presun na t1 adresu a nacteni adres
    fileStream.readVector(addressList);
    readFromDirectBlocks(addressList, count, result); //cteni jako pri primych blocich
}

void INodeIO::readFromT2Address(uint64_t t2Address, uint32_t itemCount, std::vector<FolderItem>& result) {
    auto fullBlocks = itemCount / (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());
    auto remainder = itemCount % (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());

    // stejna operace jako pri readFromT1Address, akorat musime pro kazdy radek v t2 bloku
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
    auto dot = FolderItem(Globals::CURRENT_FOLDER_SYMBOL, currentNodeAddress);
    auto dotDot = FolderItem(Globals::PARENT_FOLDER_SYMBOL, parentNodeAddress);
    // pridame "." a ".."
    appendFolderItem(current, dot, false);
    appendFolderItem(current, dotDot, false);
}

void INodeIO::printINodeInfo(INode& node, FolderItem& folderItem) {
    // tisk pozadovanych informaci
    std::cout << "NAME: " << folderItem.getItemName() << " - SIZE: " << node.size << " - NUMBER: "
              << node.id << " - REF_COUNT: " << node.refCount
              << std::endl;

    // nacteni neprazdnych bloku a tisk bloku
    auto blocks = getINodeBlocks(node);
    std::cout << "Initialized Blocks:" << std::endl;
    for (auto block : blocks) {
        std::cout << +block << std::endl;
    }
    std::cout << std::endl;
}

std::vector<uint64_t> INodeIO::getINodeBlocks(INode& node) {
    auto result = std::vector<uint64_t>();

    // Nacitame adresy, pokud je nejaka nevalidni ukoncime, protoze se data vzdy nejprve ukladaji do primych bloku,
    // pak do neprimeho a druheho neprimeho odkazu ...
    for (auto block : node.t0AddressList) {
        if (block == Globals::INVALID_VALUE) {
            return result;
        }
        result.push_back(block);
    }

    if (node.t1Address == Globals::INVALID_VALUE) { // pokud je adresa neprimeho bloku INVALID vratime se
        return result;
    }

    // jinak nacteme vsechny bloky a iterujeme pres ne
    auto t1Blocks = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
    fileStream.moveTo(node.t1Address);
    fileStream.readVector(t1Blocks);

    for (auto block : t1Blocks) {
        if (block == Globals::INVALID_VALUE) {
            return result;
        }
        result.push_back(block);
    }

    // stejne pro odkaz 2. radu, akorat navic pro kazdy prvek nacteme blok 1. radu
    if (node.t2Address == Globals::INVALID_VALUE) {
        return result;
    }

    auto t2Blocks = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
    fileStream.moveTo(node.t2Address);
    fileStream.readVector(t2Blocks);

    for (auto t2Block : t2Blocks) {
        if (t2Block == Globals::INVALID_VALUE) {
            return result;
        }
        result.push_back(t2Block);

        t1Blocks = std::vector<uint64_t>(Globals::POINTERS_PER_BLOCK(), Globals::INVALID_VALUE);
        fileStream.moveTo(t2Block);
        fileStream.readVector(t1Blocks);
        for (auto block : t1Blocks) {
            if (block == Globals::INVALID_VALUE) {
                return result;
            }
            result.push_back(block);
        }
    }

    return result;
}

void INodeIO::removeFolderItem(INode& node, FolderItem& folderItem) {
    auto items = getFolderItems(node); // ziskame predmety slozky pro zjisteni indexu
    auto index = -1;

    for (auto i = 0; i < items.size(); i++) {
        if (items[i] == folderItem) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        throw FSException("Error, item not found");
    }

    auto last = items.back();
    if (index != items.size() - 1) {
        writeAt(node, index,
                last); // zapiseme posledni prvek misto indexu, abychom nemuseli kopirovat vetsi mnozsti prvku
    }

    removeLast(node);
    fileSystemController.refresh(node);
}

void INodeIO::removeLast(INode& node) {
    auto last = node.size - 1; // index posledniho predmetu
    auto blockIndex = last / Globals::FOLDER_ITEMS_PER_BLOCK(); // index bloku, kde se predmet nachazi
    auto itemPosition = last % Globals::FOLDER_ITEMS_PER_BLOCK(); // pozice predmetu

    auto deallocations = std::vector<uint64_t>();
    if (blockIndex < Globals::T0_ADDRESS_LIST_SIZE) {
        if (itemPosition == 0) {
            deallocations.push_back(node.t0AddressList[blockIndex]);
            node.t0AddressList[blockIndex] = Globals::INVALID_VALUE;
        }
    } else if (blockIndex - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
        auto itemRelativePosition = itemPosition - (Globals::T0_ADDRESS_LIST_SIZE * Globals::FOLDER_ITEMS_PER_BLOCK());
        auto t1Row = itemRelativePosition / Globals::FOLDER_ITEMS_PER_BLOCK();
        auto itemIndex = itemRelativePosition % Globals::FOLDER_ITEMS_PER_BLOCK();

        if (itemIndex == 0) { // pokud je predmet na nultem indexu v bloku, musime blok smazat
            uint64_t blockAddress;
            fileStream.moveTo(node.t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(blockAddress);
            deallocations.push_back(blockAddress);

            fileStream.moveTo(node.t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            auto invalid = Globals::INVALID_VALUE;
            fileStream.write(invalid);

            if (t1Row == 0) { // pokud je navic i index bloku 0, tak odstranenim musime smazat i blok s pointery
                deallocations.push_back(node.t1Address);
                node.t1Address = Globals::INVALID_VALUE; // nastavime v INode hodnotu na invalid value
            }
        }

    } else {
        // relativni index v t2 bloku
        auto t2Index = itemPosition - Globals::T0_ADDRESS_LIST_SIZE * Globals::FOLDER_ITEMS_PER_BLOCK() -
                       Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK();

        // index v t2 bloku
        auto t2Row = t2Index / (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());
        auto t1Row = t2Index % (Globals::POINTERS_PER_BLOCK() * Globals::FOLDER_ITEMS_PER_BLOCK());
        auto blockRow = t1Row / Globals::FOLDER_ITEMS_PER_BLOCK();
        auto itemIndex = t1Row % Globals::FOLDER_ITEMS_PER_BLOCK();

        uint64_t t1Address;
        fileStream.moveTo(node.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
        fileStream.read(t1Address);

        if (itemIndex == 0) { // pokud je index v bloku 0, pak odstranime index v t1 tabulce
            uint64_t blockAddress;
            fileStream.moveTo(t1Address + blockRow * Globals::POINTER_SIZE_BYTES);
            fileStream.read(blockAddress);
            deallocations.push_back(blockAddress);

            fileStream.moveTo(t1Address + blockRow * Globals::POINTER_SIZE_BYTES);
            auto invalid = Globals::INVALID_VALUE;
            fileStream.write(invalid);
        }

        if (itemIndex == 0 && blockRow == 0) { // pokud je index v bloku 0 a index v t1 0 odstranime i t1 blok
            deallocations.push_back(t1Address);
            fileStream.moveTo(node.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
            auto invalid = Globals::INVALID_VALUE;
            fileStream.write(invalid);
        }

        if (itemIndex == 0 && blockRow == 0 && t2Row == 0) {
            // pokud je index v bloku 0 a index v t1 0 a index v t2 0 odstranime i t2 blok
            deallocations.push_back(node.t2Address);
            node.t2Address = Globals::INVALID_VALUE;
        }

    }

    fileSystemController.reclaimMemory(deallocations); // smazani vsech bloku, ktere jsme oznacili pri odstranovani
    node.refCount -= 1; // snizeni poctu referenci
    node.size -= 1; // snizeni velikosti slozky

}

uint64_t INodeIO::getExtraBlocks(uint64_t bytes) {
    auto remainingBytes = bytes;
    if (remainingBytes <= Globals::T0_ADDRESS_LIST_CAPACITY_BYTES()) {
        return 0;
    }

    remainingBytes -= Globals::T0_ADDRESS_LIST_CAPACITY_BYTES();
    if (remainingBytes <= Globals::T1_POINTER_CAPACITY_BYTES()) {
        return 1;
    }

    remainingBytes -= Globals::T1_POINTER_CAPACITY_BYTES();
    uint64_t t1Blocks;
    t1Blocks = remainingBytes % Globals::T1_POINTER_CAPACITY_BYTES() == 0 ?
               remainingBytes / Globals::T1_POINTER_CAPACITY_BYTES() :
               remainingBytes / Globals::T1_POINTER_CAPACITY_BYTES() + 1;

    return t1Blocks + 2; // + 1 t2 blok + t1 blok pro INode
}


void INodeIO::exportFile(INode& node, FileStream& outputFileStream) {
    auto bytes = node.size;
    auto blockCount = Globals::getBlockCount(bytes);
    auto remainingBytes = bytes;

    // buffer pro cteni dat
    auto buffer = std::vector<char>(Globals::BLOCK_SIZE_BYTES, '\0');
    for (auto currentBlock = 0; currentBlock < blockCount; currentBlock += 1) {
        if (remainingBytes < Globals::BLOCK_SIZE_BYTES) {
            buffer = std::vector<char>(remainingBytes, '\0'); // nechceme precist vice nez je mozno
            remainingBytes = 0;
        } else {
            remainingBytes -= Globals::BLOCK_SIZE_BYTES;
        }

        // vyber daneho bloku
        if (currentBlock < Globals::T0_ADDRESS_LIST_SIZE) {
            fileStream.moveTo(node.t0AddressList[currentBlock]);
            fileStream.readVector(buffer);
        } else if (currentBlock - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
            auto t1Row = currentBlock - Globals::T0_ADDRESS_LIST_SIZE;

            uint64_t address;
            fileStream.moveTo(node.t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(address);
            fileStream.moveTo(address);
            fileStream.readVector(buffer);
        } else {
            auto relativeIndex = currentBlock - Globals::T0_ADDRESS_LIST_SIZE - Globals::POINTERS_PER_BLOCK();
            auto t2Row = relativeIndex / Globals::POINTERS_PER_BLOCK();
            auto t1Row = relativeIndex % Globals::POINTERS_PER_BLOCK();

            uint64_t t1Address;
            fileStream.moveTo(node.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(t1Address);

            uint64_t blockAddress;
            fileStream.moveTo(t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(blockAddress);

            fileStream.moveTo(blockAddress);
            fileStream.readVector(buffer);
        }

        outputFileStream.writeVector(buffer);
    }
}


void INodeIO::readFile(INode& node) {
    auto bytes = node.size;
    auto blockCount = Globals::getBlockCount(bytes);
    auto remainingBytes = bytes;

    auto buffer = std::vector<char>(Globals::BLOCK_SIZE_BYTES, '\0');
    for (auto currentBlock = 0; currentBlock < blockCount; currentBlock += 1) {
        if (remainingBytes < Globals::BLOCK_SIZE_BYTES) {
            buffer = std::vector<char>(remainingBytes, '\0');
            remainingBytes = 0;
        } else {
            remainingBytes -= Globals::BLOCK_SIZE_BYTES;
        }

        // vyber daneho bloku a precteni dat
        if (currentBlock < Globals::T0_ADDRESS_LIST_SIZE) {
            fileStream.moveTo(node.t0AddressList[currentBlock]);
            fileStream.readVector(buffer);
        } else if (currentBlock - Globals::T0_ADDRESS_LIST_SIZE < Globals::POINTERS_PER_BLOCK()) {
            auto t1Row = currentBlock - Globals::T0_ADDRESS_LIST_SIZE;

            uint64_t address;
            fileStream.moveTo(node.t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(address);
            fileStream.moveTo(address);
            fileStream.readVector(buffer);
        } else {
            auto relativeIndex = currentBlock - Globals::T0_ADDRESS_LIST_SIZE - Globals::POINTERS_PER_BLOCK();
            auto t2Row = relativeIndex / Globals::POINTERS_PER_BLOCK();
            auto t1Row = relativeIndex % Globals::POINTERS_PER_BLOCK();

            uint64_t t1Address;
            fileStream.moveTo(node.t2Address + t2Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(t1Address);

            uint64_t blockAddress;
            fileStream.moveTo(t1Address + t1Row * Globals::POINTER_SIZE_BYTES);
            fileStream.read(blockAddress);

            fileStream.moveTo(blockAddress);
            fileStream.readVector(buffer);
        }

        printBuffer(buffer); // tisk do konzole
    }
}

void INodeIO::printBuffer(std::vector<char>& vector) {
    auto text = std::string(vector.begin(), vector.end());
    std::cout << text << std::flush;
}

