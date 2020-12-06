#ifndef INODE_HPP
#define INODE_HPP


#include <cstdint>
#include <vector>
#include "../../global/Globals.hpp"

class FolderItem;
class FStreamWrapper;
class INode {

      bool folder = false;
      uint32_t id = (uint32_t) Globals::INVALID_VALUE; // Id. Bude bud vypocteno z adresy INode nebo INVALID_VALUE
      uint32_t size = 0; // Velikost souboru nebo pocet prvku ve slozce
      uint64_t timestamp = Globals::INVALID_VALUE; // Timestamp pro posledni pristup k souboru
      uint64_t refCount = 0; // kolik referenci ukazuje na inode - lze smazat pouze, pokud je refCount = 0 (resp. 1)

      std::vector<uint64_t> t0AddressList =
              std::vector(Globals::T0_ADDRESS_LIST_SIZE,
                          Globals::INVALID_VALUE); // Seznam vsech primych adres

      uint64_t t1Address = Globals::INVALID_VALUE; // Odkaz na datovy blok s neprimymi adresami
      uint64_t t2Address = Globals::INVALID_VALUE; // Odkaz na blok s odkazy na neprime adresy

    public:
      /**
       * @return if inode is linked to any data
       */
      [[nodiscard]] bool isFree() const;

      [[nodiscard]] bool isFolder() const;

      [[nodiscard]] const std::vector<uint64_t>& getT0AddressList() const;

      [[nodiscard]] uint64_t getT1Address() const;

      [[nodiscard]] uint64_t getT2Address() const;

      uint32_t getSize() const;

      uint32_t getFolderSize(); // alias pro logictejsi kod pro INode, ktera reprezentuje slozku

      void setSize(uint32_t size);

      void setDirectAddress(uint32_t index, uint64_t address);

      void setT1Address(uint64_t t1IndirectAddress);

      void setT2Address(uint64_t t2IndirectAddress);

      INode(bool isFolder, uint32_t id);

      friend FStreamWrapper& operator<<(FStreamWrapper& fs, INode& iNode);

      friend FStreamWrapper& operator>>(FStreamWrapper& fs, INode& iNode);

      /**
       * Nastavi hodnoty pro root folder dane referenci na iNode
       * @param iNode reference na I-Node objekt
       */
      static void makeRoot(INode& iNode);

      /**
       * Nacte ze souboru polozky dane slozky prostrednictvim fstream
       * @param fstream fstream wrapper pro precteni dat ze souboru
       * @param folderItemBuffer reference na vektor, kam se zapisou namapovane objekty
       */
      const void loadValidFolderItems(FStreamWrapper& fstream, std::vector<FolderItem>& folderItemBuffer);

      INode();

      void setDirectAddressList(std::vector<uint64_t>& vector);
};


#endif //ZOSREWORK_INODE_HPP
