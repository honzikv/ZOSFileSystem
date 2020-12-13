
#include <memory>
#include <iostream>
#include "FileSystemController.hpp"
#include "INodeIO.hpp"
#include "PathContext.hpp"


FileSystemController::FileSystemController(FileStream& fileStream) : fileStream(fileStream) {
    if (!fileStream.fileExists()) {
        driveState = DriveState::NonExistent;
        return;
    }
    fileStream.open();
    if (!fileStream.good()) { // pokud z jakehokoliv duvodu neni fstream ve filestream objektu "good" nelze dale cist
        driveState = DriveState::Invalid;
        fileStream.close();
        return;
    }

    if (fileStream.isFileEmpty()) { // prazdny soubor neni validni disk
        driveState = DriveState::Empty;
        fileStream.close();
        return;
    }

    if (fileStream.getFileSize() < Globals::MIN_DRIVE_SIZE) { // soubor, mensi nez minimalni velikost disku neni validni
        driveState = DriveState::Invalid;
        fileStream.close();
        return;
    }

    fileStream.moveTo(0);
    auto fileSuperBlock = SuperBlock();
    fileStream.readSuperBlock(fileSuperBlock);
    if (!fileSuperBlock.isValid()) { // pokud neni super blok validni zavreme stream a nastavime DriveState na invalid
        driveState = DriveState::Invalid;
        fileStream.close();
        return;
    }

    driveState = DriveState::Valid;
    superBlock = std::make_shared<SuperBlock>(fileSuperBlock); // zkopirujeme super blok objekt na heap do shared ptr
    memoryAllocator = std::make_shared<MemoryAllocator>(superBlock, fileStream);
    nodeIO = std::make_shared<INodeIO>(fileStream, *this);
    pathContext = std::make_shared<PathContext>(*this);
}


INode FileSystemController::getRoot() {
    auto root = INode();
    fileStream.moveTo(superBlock->nodeAddress);
    fileStream.readINode(root);
    return root;
}


void FileSystemController::diskInfo() {
    if (driveState == DriveState::Valid) {
        superBlock->printInfo();
    } else if (driveState == DriveState::Invalid) {
        std::cout << "Non-empty file without super block" << std::endl;
    } else if (driveState == DriveState::Empty) {
        std::cout << "File is empty" << std::endl;
    } else {
        std::cout << "File with drive has not been created yet" << std::endl;
    }
}

void FileSystemController::reclaimMemory(std::vector<uint64_t>& memoryBlocks) {
    for (auto& block : memoryBlocks) {
        memoryAllocator->freeMemory(block);
    }
}

void FileSystemController::refresh(INode& node) {
    memoryAllocator->update(node);
    pathContext->update(node);
}


void FileSystemController::writeINode(INode& node) {
    memoryAllocator->update(node);
}

uint64_t FileSystemController::nextBlock(AddressType type) {
    return memoryAllocator->getDataBlock(type);
}

void FileSystemController::outcp(const std::string& fileIn, const std::string& fileOut) {

}

void FileSystemController::incp(const std::string& fileOut, const std::string& fileFS) {

}

void FileSystemController::info(const std::string& file) {

}

void FileSystemController::pwd() {
    pathContext->printCurrentFolder();
}

void FileSystemController::cd(const std::string& path) {

}

void FileSystemController::cat(const std::string& file) {

}

void FileSystemController::ls(const std::string& path) {
    pathContext->listItems(path);

}

void FileSystemController::rmdir(const std::string& dirName) {

}

void FileSystemController::mkdir(const std::string& path) {
    pathContext->makeFolder(path);
}

void FileSystemController::rm(const std::string& file) {

}

void FileSystemController::mv(const std::string& file, const std::string& path) {

}

void FileSystemController::cp(const std::string& file, const std::string& path) {

}

DriveState FileSystemController::getDriveState() const {
    return driveState;
}

void FileSystemController::initDrive() {
    fileStream.moveTo(0);
    if (fileStream.isFileEmpty() || fileStream.getFileSize() < Globals::MIN_DRIVE_SIZE) {
        throw FSException(
                "Error while formatting the file, OS might be preventing from writing to the file, "
                "please restart the program.");
    }
    superBlock = std::make_shared<SuperBlock>();
    fileStream.readSuperBlock(*this->superBlock);

    if (!this->superBlock->isValid()) {
        throw FSException(
                "Error super block is not properly formatted @FileSystemController::initDrive()");
    }

    memoryAllocator = std::make_shared<MemoryAllocator>(superBlock, fileStream);
    nodeIO = std::make_shared<INodeIO>(fileStream, *this);

    createINodes();
    pathContext = std::make_shared<PathContext>(*this);

    driveState = DriveState::Valid;
}


void FileSystemController::createINodes() {
    // vytvoreni root inode
    auto root = INode();
    root.id = 0;
    root.refCount = 1; // ref count = 1 aby root nesel nikdy odstranit
    root.folder = true;
    root.timestamp = std::chrono::system_clock::now().time_since_epoch().count(); // nastaveni casu vytvoreni

    fileStream.moveTo(this->superBlock->nodeAddress);
    std::cout << fileStream.getWritePosition() << std::endl;
    fileStream.writeINode(root);

    auto empty = INode();
    for (auto i = 1; i < superBlock->nodeCount; i++) {
        fileStream.writeINode(empty);
    }

    nodeIO->linkFolderToParent(root, superBlock->nodeAddress, superBlock->nodeAddress);
}

std::vector<FolderItem> FileSystemController::getFolderItems(INode& node) {
    return nodeIO->getFolderItems(node);
}

INode FileSystemController::getFolderItemINode(uint64_t nodeAddress) {
    auto node = INode();
    fileStream.moveTo(nodeAddress);
    fileStream.readINode(node);
    return node;
}

uint64_t FileSystemController::getNodeAddress(INode& node) {
    return memoryAllocator->getNodeAddress(node);
}

INode FileSystemController::getFreeINode() {
    return memoryAllocator->getINode();
}

void FileSystemController::addItem(INode& parent, FolderItem child) {
    nodeIO->append(parent, child);
}

void FileSystemController::reclaimINode(INode& node) {
    memoryAllocator->freeINode(node);
}

INode FileSystemController::getUpdatedINode(INode& node) {
    return memoryAllocator->getINodeWithId(node.id);
}


