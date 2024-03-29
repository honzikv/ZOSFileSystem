
#ifndef FOLDERITEM_HPP
#define FOLDERITEM_HPP


#include <vector>
#include <cstdint>
#include <string>
#include "../../global/Globals.hpp"
#include "INode.hpp"

class FileStream;

/**
 * Reprezentuje predmet ve slozce - DirectoryItem
 */
struct FolderItem {

      std::vector<char> itemName = std::vector<char>(Globals::FILE_NAME_CHAR_ARRAY_LENGTH, (char) 0); // nazev souboru
      uint64_t nodeAddress = Globals::INVALID_VALUE; // reference na INode

      /**
       * Validace nazvu slozky
       * @param input vstup od uzivatele
       */
      static void validateFolderName(const std::string& input);

      /**
       * Validace nazvu souboru
       * @param input vstup od uzivatele
       */
      static void validateFileName(const std::string& input);

    public:

      FolderItem(const std::string& itemName, uint64_t nodeAddress);

      /**
       * Konstruktor pro invalid folder item
       */
      FolderItem();

      /**
       * Ziskani jmena predmetu
       * @return string se jmenem predmetu
       */
      [[nodiscard]] std::string getItemName() const;

      /**
       * Ziskani adresy INode
       * @return adresu Inode
       */
      [[nodiscard]] uint64_t getNodeAddress() const;

      bool operator==(const FolderItem& other) const;

      bool operator!=(const FolderItem& other) const;


};


#endif //ZOSREWORK_FOLDERITEM_HPP
