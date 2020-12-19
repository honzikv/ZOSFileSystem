
#include <memory>
#include <iostream>
#include "FileSystemController.hpp"
#include "INodeIO.hpp"
#include "PathContext.hpp"
#include "FileOperations.hpp"


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
    fileOperations = std::make_shared<FileOperations>(*this);
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
}


void FileSystemController::writeINode(INode& node) {
    memoryAllocator->update(node);
}

uint64_t FileSystemController::nextBlock(AddressType type) {
    return memoryAllocator->getDataBlock(type);
}

void FileSystemController::outcp(const std::string& path, const std::string& exportPath) {
    fileOperations->exportFromFileSystem(path, exportPath);
}

void FileSystemController::incp(const std::string& readPath, const std::string& path) {
    fileOperations->copyIntoFileSystem(readPath, path);
}

void FileSystemController::info(const std::string& file) {
    fileOperations->getInfo(file);
}

void FileSystemController::pwd() {
    fileOperations->printCurrentPath();
}

void FileSystemController::cd(const std::string& path) {
    fileOperations->changeDirectory(path);
}

void FileSystemController::cat(const std::string& file) {
    fileOperations->readFile(file);
}

void FileSystemController::ls(const std::string& path) {
    fileOperations->listItems(path);
}

void FileSystemController::rmdir(const std::string& path) {
    fileOperations->removeDirectory(path);
}

void FileSystemController::mkdir(const std::string& path) {
    fileOperations->makeDirectory(path);
}

void FileSystemController::rm(const std::string& path) {
    fileOperations->removeFile(path);
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
    fileOperations = std::make_shared<FileOperations>(*this);
    driveState = DriveState::Valid;
}


void FileSystemController::createINodes() {
    // vytvoreni root inode
    auto root = INode::createRoot();

    fileStream.moveTo(this->superBlock->nodeAddress);
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

INode FileSystemController::getINodeFromAddress(uint64_t nodeAddress) {
    auto node = INode();
    fileStream.moveTo(nodeAddress);
    fileStream.readINode(node);
    return node;
}

uint64_t FileSystemController::getINodeAddress(INode& node) {
    return memoryAllocator->getNodeAddress(node);
}

INode FileSystemController::getFreeINode(bool isFolder) {
    return memoryAllocator->getINode(isFolder);
}

void FileSystemController::appendFolder(INode& parent, FolderItem child) {
    nodeIO->appendFolderItem(parent, child);
}

void FileSystemController::reclaimINode(INode& node) {
    memoryAllocator->freeINode(node);
}

INode FileSystemController::getUpdatedINode(INode& node) {
    return memoryAllocator->getINodeWithId(node.getId());
}

void FileSystemController::linkFolderToParent(INode& child, uint64_t childAddress, uint64_t parentNodeAddress) {
    nodeIO->linkFolderToParent(child, childAddress, parentNodeAddress);
}

void FileSystemController::printINodeInfo(INode& node) {
    nodeIO->printINodeInfo(node);
}

void FileSystemController::removeFolderItem(INode& node, FolderItem& folderItem) {
    nodeIO->removeFolderItem(node, folderItem);
}

void
FileSystemController::appendFile(INode& parent, INode& child, FolderItem& folderItem, FileStream& externalFileStream) {
    nodeIO->appendFile(parent, child, folderItem, externalFileStream);
}

std::vector<uint64_t> FileSystemController::nextNBlocks(uint32_t n, AddressType addressType) {
    return memoryAllocator->getNDataBlocks(n, addressType);
}

void FileSystemController::readFile(INode& node) {
    nodeIO->readFile(node);
}

void FileSystemController::exportFile(INode& node, FileStream& outputFileStream) {
    nodeIO->exportFile(node, outputFileStream);
}


