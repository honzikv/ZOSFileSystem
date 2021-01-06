#include <sstream>
#include "FolderItem.hpp"
#include "../../util/FSException.hpp"

void FolderItem::validateFileName(const std::string& input) {
    if (input.empty()) {
        throw FSException("Given name is empty");
    }

    if (input.length() > Globals::MAX_FILE_STRING_LENGTH) {
        throw FSException("File name with extension is too long");
    }

    if (input.find('.') == std::string::npos) { // pokud neni tecka a je soubor prilis dlouhy, vyhodime exception
        if (input.length() > Globals::FILE_NAME_CHAR_LIMIT) {
            throw FSException("Error file name is too long");
        }
    } else {

        auto stringStream = std::istringstream(input);
        auto token = std::string();
        auto tokens = std::vector<std::string>();

        while (std::getline(stringStream, token, '.')) {
            tokens.push_back(token);
        }

        // posledni z tecek je koncovka
        if (tokens[tokens.size() - 1].length() > Globals::FILE_EXTENSION_CHAR_LIMIT) {
            throw FSException(
                    "Extension is too long, make sure it is less or equal to " +
                    std::to_string(Globals::FILE_EXTENSION_CHAR_LIMIT)
            );
        }
    }
}

FolderItem::FolderItem(const std::string& itemName, uint64_t nodeAddress) : nodeAddress(nodeAddress) {
    this->itemName = std::vector<char>(itemName.begin(), itemName.end());
    auto paddingBytes = Globals::FILE_NAME_CHAR_ARRAY_LENGTH - itemName.size();
    for (auto i = 0; i < paddingBytes; i++) {
        this->itemName.push_back((char) 0);
    }
}

FolderItem::FolderItem() = default;


std::string FolderItem::getItemName() const {
    // iterujeme dokud nenarazime na '/0' a pote vratime string
    auto terminalIndex = -1;
    for (auto i = 0; i < itemName.size(); i++) {
        if (itemName[i] == (char) 0) {
            terminalIndex = i;
            break;
        }
    }

    if (terminalIndex == -1) {
        throw FSException("Error terminal symbol in the string not found"); // nemelo by se stat
    }

    return std::string(itemName.begin(), itemName.begin() + terminalIndex);
}

uint64_t FolderItem::getNodeAddress() const {
    return nodeAddress;
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
    return std::equal(itemName.begin(), itemName.end(), other.itemName.begin()) &&
           nodeAddress == other.nodeAddress;
}

bool FolderItem::operator!=(const FolderItem& other) const {
    return !(other == *this);
}
