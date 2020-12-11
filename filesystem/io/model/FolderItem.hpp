
#ifndef FOLDERITEM_HPP
#define FOLDERITEM_HPP


#include <vector>
#include <cstdint>
#include <string>
#include "../../global/Globals.hpp"
#include "INode.hpp"

class FileStream;

struct FolderItem {

      std::vector<char> itemName = std::vector<char>(Globals::FILE_NAME_CHAR_ARRAY_LENGTH, '\0'); // nazev souboru
      uint64_t nodeAddress = Globals::INVALID_VALUE; // reference na INode

    public:

      FolderItem(const std::string& itemName, uint64_t nodeAddress, bool isFolder);

      /**
       * Konstruktor pro invalid folder item
       */
      FolderItem();

      /**
       * Vrati, zda-li je dany FolderItem invalid. To lze pouzit jako ukonceni pri iterovani po bloku
       * @param folderItem reference na folder item
       * @return true, pokud je folder item nevalidni - tzn. adresa nepointuje na zadnou inode, jinak false
       */
      [[nodiscard]] bool isEmpty() const;

      [[nodiscard]] std::string getItemName() const;

      [[nodiscard]] uint64_t getNodeAddress() const;

      bool operator==(const FolderItem& other) const;

      bool operator!=(const FolderItem& other) const;

    private:
      void validateFileName(const std::string& input);

      static void validateFolderName(const std::string& input);
};


#endif //ZOSREWORK_FOLDERITEM_HPP
