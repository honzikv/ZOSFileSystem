
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
    if (nodeIndex == absolutePath.size() - 1) {
        folderItems = fileSystemController.getFolderItems(absolutePath.back());
    }
}

void PathContext::listItems(const std::string& path) {
    auto fsPath = FileSystemPath(path);

    if (fsPath.getPathType() == PathType::CurrentFolder) {
        // pokud zadame pouze ls
        printFolderItemNames();
        return;
    }

    // zkopirujeme aktualni hodnoty protoze je nahradime novymi pro zlisteni ls
    auto currentPath = std::vector<INode>();
    currentPath = absolutePath;
    auto currentFolderItems = std::vector<FolderItem>();
    currentFolderItems = folderItems;

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

    absolutePath = currentPath;
    folderItems = currentFolderItems;
}


int PathContext::getNodeIndex(INode& node) {
    for (auto i = 0; i < absolutePath.size(); i++) {
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

}

void PathContext::printAbsolutePath(FileSystemPath& path) {

}

bool PathContext::folderItemExists(std::string& itemName) {
    for (auto& folderItem : folderItems) {
        if (folderItem.getItemName() == itemName) {
            return true;
        }
    }
    return false;
}

void PathContext::moveTo(std::string& folderItemName) {
    if (folderItemName == Globals::CURRENT_FOLDER_SYMBOL) {
        // kazda slozka ma referenci sama na sebe
        return;
    }
    if (folderItemName == Globals::PREVIOUS_FOLDER_SYMBOL) {
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
    auto folderItemNode = fileSystemController.getFolderItemINode(folderItem.nodeAddress);

    if (!folderItemNode.isFolder()) {
        throw FSException("Error, incorrect path");
    }

    absolutePath.push_back(folderItemNode);
    folderItems = fileSystemController.getFolderItems(folderItemNode);
}

int PathContext::getFolderItemIndex(std::string& folderItemName) {
    for (auto i = 0; i < folderItems.size(); i += 1) {
        if (folderItems[i].getItemName() == folderItemName) {
            return i;
        }
    }
    return -1;
}

