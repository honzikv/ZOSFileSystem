
#include "../util/FileSystemPath.h"
#include "PathContext.hpp"
#include "FileOperations.hpp"

PathContext::PathContext(FileOperations& fileOperations) : fileOperations(fileOperations) {

    auto root = fileOperations.getRoot();
    absolutePath.push_back(root);
    auto rootItems = fileOperations.getFolderItems(root);
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
        fileOperations.getFolderItems(root);
    }
}

void PathContext::refresh() {
    auto updatedINodes = std::vector<INode>();
    for (auto& node : absolutePath) {
        updatedINodes.push_back(fileOperations.getUpdatedINode(node));
    }
    absolutePath = updatedINodes;
    folderItems = fileOperations.getFolderItems(absolutePath.back());
}

void PathContext::moveTo(FileSystemPath& path) {
    if (path.getPathType() == PathType::Absolute) {
        moveToRoot(false); // presuneme se na root pokud je cesta absolutni
    }

    for (auto i = 0; i < path.size(); i++) {
        auto nextFolder = path[i];

        if (nextFolder == ".." && absolutePath.size() > 1) {
            absolutePath.pop_back();
            continue;
        } else if (nextFolder == ".") {
            continue;
        }

        folderItems = fileOperations.getFolderItems(absolutePath.back());
        auto nextFolderIndex = getFolderItemIndex(nextFolder);
        if (nextFolderIndex == -1) {
            throw FSException("Error, specified path is not valid.");
        }

        auto nextFolderNodeAddress = folderItems[nextFolderIndex].nodeAddress;
        INode nextFolderNode;
        try {
            nextFolderNode = fileOperations.getINodeFromAddress(nextFolderNodeAddress);
            if (!nextFolderNode.isFolder()) {
                throw FSException("Error, path contains file instead of a folder");
            }
            absolutePath.push_back(nextFolderNode);
        }
        catch (FSException& ex) {
            throw FSException("Error while reading INode @ PathContext"); //debug, nemelo by se stat
        }
    }
}

void PathContext::loadItems() {
    folderItems = fileOperations.getFolderItems(absolutePath.back());
}

