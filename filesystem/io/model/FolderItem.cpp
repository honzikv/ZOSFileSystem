#include <sstream>
#include "FolderItem.hpp"
#include "../../util/FSException.hpp"
#include "../../util/FileStream.hpp"

void FolderItem::validateFileName(const std::string& input) {
    if (input.empty()) {
        throw FSException("Given name is empty");
    }

    if (input.length() > Globals::MAX_FILE_STRING_LENGTH) {
        throw FSException("File name with extension is too long");
    }

    auto stringStream = std::istringstream(input);
    auto token = std::string();
    auto tokens = std::vector<std::string>();

    while (std::getline(stringStream, token, '.')) {
        tokens.push_back(token);
    }

    if (tokens[tokens.size() - 1].length() > Globals::FILE_EXTENSION_CHAR_LIMIT) {
        throw FSException(
                "Extension is too long, make sure it is less or equal to " +
                std::to_string(Globals::FILE_EXTENSION_CHAR_LIMIT)
        );
    }

    auto trailingZeros = Globals::FILE_NAME_CHAR_ARRAY_LENGTH - input.length();
    itemName = std::vector(input.begin(), input.end());

    // doplneni \0 do maximalni velikosti nazvu
    for (auto i = 0; i < trailingZeros; i++) {
        itemName.push_back('\0');
    }

}

FolderItem::FolderItem(const std::string& itemName, uint64_t nodeAddress, bool isFolder) : nodeAddress(nodeAddress) {
    if (isFolder) {
        validateFolderName(itemName);
    } else {
        validateFileName(itemName);
    }
    this->itemName = std::vector<char>(itemName.begin(), itemName.end());
    auto paddingBytes = Globals::FILE_NAME_CHAR_ARRAY_LENGTH - itemName.size();
    for (auto i = 0; i < paddingBytes; i++) {
        this->itemName.push_back('\0');
    }
}

FolderItem::FolderItem() {
    itemName = std::vector<char>('\0', Globals::MAX_FILE_STRING_LENGTH);
}



std::string FolderItem::getItemName() const {
    auto terminalIndex = -1;
    for (auto i = 0; i < itemName.size(); i++) {
        if (itemName[i] == '\0') {
            terminalIndex = i;
        }
    }

    return std::string(itemName.begin(), terminalIndex == -1 ? itemName.end() :
                                         itemName.end() - terminalIndex);
}

uint64_t FolderItem::getNodeAddress() const {
    return nodeAddress;
}

bool FolderItem::isEmpty() const {
    return nodeAddress == Globals::INVALID_VALUE;
}

void FolderItem::validateFolderName(const std::string& input) {
    if (input.empty()) {
        throw FSException("Given name is empty");
    }

    if (input.length() > Globals::MAX_FILE_STRING_LENGTH) {
        throw FSException("Folder name is too long");
    }
}

bool FolderItem::operator==(const FolderItem& other) const {
    return std::equal(itemName.begin(),itemName.end(), other.itemName.begin())  &&
           nodeAddress == other.nodeAddress;
}

bool FolderItem::operator!=(const FolderItem& other) const {
    return !(other == *this);
}
