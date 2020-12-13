
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


bool PathContext::folderItemExists(std::string& itemName) {
    for (auto& folderItem : folderItems) {
        if (folderItem.getItemName() == itemName) {
            return true;
        }
    }
    return false;
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

