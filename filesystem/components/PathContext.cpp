
#include "../util/FileSystemPath.h"
#include "PathContext.hpp"

PathContext::PathContext(FileSystemController& fileSystemController) : fileSystemController(fileSystemController) {
    initialize();
}

void PathContext::initialize() {
    auto root = fileSystemController.getRoot();
    absolutePath.push_back(root);
    auto rootItems = fileSystemController.getFolderItems(root);
    folderItems = rootItems;
}

void PathContext::update(INode& node) {
    auto nodeIndex = getNodeIndex(node);
    if (nodeIndex != -1) {
        absolutePath[nodeIndex] = node;
    }
}

void PathContext::listItems(const std::string& path) {
    auto fsPath = FileSystemPath(path);

    if (fsPath.getPathType() == PathType::Empty) {
        // pokud zadame pouze ls
        printFolderItemNames();
        return;
    }

    // zkopirujeme cestu, protoze bude potreba zmenit
    auto currentPath = absolutePath;

    try {
        if (fsPath.getPathType() == PathType::Relative) {
            printRelativePath(fsPath);
        } else if (fsPath.getPathType() == PathType::Absolute) {
            printAbsolutePath(fsPath);
        }
    }
    catch (FSException& ex) {
        std::cout << "Error, invalid path" << std::endl;
    }

    // vratime "aktualni" cestu zpet
    absolutePath = currentPath;
    refresh();
}


int PathContext::getNodeIndex(INode& node) {
    for (auto i = 0; i < absolutePath.size(); i += 1) {
        if (node == absolutePath[i]) {
            return i;
        }
    }
    return -1;
}


void PathContext::printFolderItemNames() {
    for (auto& item : folderItems) {
        std::cout << item.getItemName() << std::endl;
    }
}

void PathContext::printRelativePath(FileSystemPath& path) {
    for (auto i = 0; i < path.size() - 1; i++) {
        auto folder = path.get();
        moveTo(folder, false);
        path.next();
    }
    auto lastFolder = path.get();
    moveTo(lastFolder, true);
    printFolderItemNames();
}

void PathContext::printAbsolutePath(FileSystemPath& path) {
    moveToRoot();
    printRelativePath(path);
}

bool PathContext::folderItemExists(std::string& itemName) {
    for (auto& folderItem : folderItems) {
        if (folderItem.getItemName() == itemName) {
            return true;
        }
    }
    return false;
}

void PathContext::moveTo(std::string& folderItemName, bool fetchFolderItems) {
    if (folderItemName == Globals::CURRENT_FOLDER_SYMBOL) {
        // kazda slozka ma referenci sama na sebe
        return;
    }
    if (folderItemName == Globals::PARENT_FOLDER_SYMBOL) {
        // kazda slozka ma referenci na slozku pred ni (i root, ktery ma sam na sebe)
        if (absolutePath.size() == 1) {
            // pokud je ".." kdyz je posledni prvek root, nic se neudela, protoze root ma referenci sam na sebe
            return;
        }
        absolutePath.pop_back();
        auto previousNode = absolutePath.back();
        folderItems = fileSystemController.getFolderItems(previousNode);
        return;
    }

    auto folderItemIndex = getFolderItemIndex(folderItemName);
    if (folderItemIndex == -1) {
        throw FSException();
    }
    auto folderItem = folderItems[folderItemIndex];
    auto folderItemNode = fileSystemController.getINodeFromAddress(folderItem.nodeAddress);

    if (!folderItemNode.isFolder()) {
        throw FSException("Error, incorrect path");
    }

    absolutePath.push_back(folderItemNode);
    if (fetchFolderItems) {
        folderItems = fileSystemController.getFolderItems(folderItemNode);
    }
}

int PathContext::getFolderItemIndex(std::string& folderItemName) {
    auto folderItemLowerCase = StringParsing::toLowerCase(folderItemName);
    for (auto i = 0; i < folderItems.size(); i += 1) {
        auto itemName = folderItems[i].getItemName();
        auto itemNameLowerCase = StringParsing::toLowerCase(itemName);
        if (itemNameLowerCase == folderItemLowerCase) {
            return i;
        }
    }
    return -1;
}

void PathContext::printCurrentFolder() {
    auto path = std::vector<std::string>();
    if (absolutePath.size() == 1) {
        std::cout << "/" << std::endl;
    } else {
        path.emplace_back("/");
        for (auto i = 1; i < absolutePath.size(); i += 1) {
            auto parentFolderItems = fileSystemController.getFolderItems(absolutePath[i - 1]);
            auto itemName = getItemName(parentFolderItems, absolutePath[i]);
            path.push_back(itemName);
        }

        auto delim = "/";

        auto imploded = std::ostringstream();
        std::copy(path.begin(), path.end(),
                  std::ostream_iterator<std::string>(imploded, delim));

        std::cout << imploded.str() << std::endl;
    }
}

std::string PathContext::getItemName(const std::vector<FolderItem>& parentFolderItems, INode& node) {
    for (auto& folderItem : parentFolderItems) {
        if (folderItem.nodeAddress == fileSystemController.getNodeAddress(node)) {
            return folderItem.getItemName();
        }
    }

    throw FSException("Error, no such inode in current working directory"); // pouze pro debug
}

void PathContext::makeFolder(const std::string& path) {
    auto fsPath = FileSystemPath(path);
    auto currentAbsolutePath = absolutePath; // zkopirujeme absolute path protoze se muze zmenit

    if (fsPath.size() > 1) {
        if (fsPath.getPathType() == PathType::Absolute) {
            moveToRoot();
        } else if (fsPath.getPathType() == PathType::Empty) {
            throw FSException("Error, provided path was empty"); //debug
        }

        for (auto i = 0; i < fsPath.size() - 1; i += 1) {
            auto currItem = fsPath.get();
            moveTo(currItem, false);
            fsPath.next();
        }
        folderItems = fileSystemController.getFolderItems(absolutePath.back());
    }

    auto folderName = fsPath.get();
    if (folderItemExists(folderName)) {
        throw FSException("Error, this file/folder already exists");
    }

    auto newFolderNode = fileSystemController.getFreeINode(); // inode pro novou slozku
    auto newFolderNodeAddress = fileSystemController.getNodeAddress(newFolderNode); // adresa inode
    newFolderNode.folder = true; // nastaveni ze je inode slozka
    newFolderNode.refCount = 1; // nastaveni referenci na 1

    try {
        auto folderItem = FolderItem(folderName, newFolderNodeAddress, true);
        auto parent = absolutePath.back();
        auto parentAddress = fileSystemController.getNodeAddress(parent);
        fileSystemController.writeINode(newFolderNode);
        fileSystemController.linkFolderToParent(newFolderNode, newFolderNodeAddress, parentAddress);
        fileSystemController.append(parent, folderItem);
    }
    catch (FSException& nameException) {
        fileSystemController.reclaimINode(newFolderNode);
        throw FSException(nameException.what());
    }

    // vraceni absolutni cesty zpet
    absolutePath = currentAbsolutePath;
    refresh();
}

void PathContext::moveToRoot(bool fetchFolderItems) {
    auto root = absolutePath[0];
    absolutePath = std::vector<INode>();
    absolutePath.push_back(root);

    if (fetchFolderItems) {
        fileSystemController.getFolderItems(root);
    }
}

void PathContext::refresh() {
    auto updatedINodes = std::vector<INode>();
    for (auto& node : absolutePath) {
        updatedINodes.push_back(fileSystemController.getUpdatedINode(node));
    }
    absolutePath = updatedINodes;
    folderItems = fileSystemController.getFolderItems(absolutePath.back());
}

void PathContext::moveToPath(FileSystemPath& path) {
    for (auto i = 0; i < path.size(); i++) {
        auto nextFolder = path[i];

        if (nextFolder == ".." || nextFolder == ".") {
            continue;
        }

        folderItems = fileSystemController.getFolderItems(absolutePath.back());
        auto nextFolderIndex = getFolderItemIndex(nextFolder);
        if (nextFolderIndex == -1) {
            throw FSException("Error, specified path is not valid.");
        }

        auto nextFolderNodeAddress = folderItems[nextFolderIndex].nodeAddress;
        INode nextFolderNode;
        try {
            nextFolderNode = fileSystemController.getINodeFromAddress(nextFolderNodeAddress);
            absolutePath.push_back(nextFolderNode);
        }
        catch (FSException& ex) {
            throw FSException("Error while reading INode @ PathContext"); //debug
        }
    }
}

void PathContext::loadItems() {
    folderItems = fileSystemController.getFolderItems(absolutePath.back());
}

