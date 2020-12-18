#ifndef INODE_HPP
#define INODE_HPP


#include <cstdint>
#include <vector>
#include "../../global/Globals.hpp"

class FolderItem;

class FileStream;

class INode {
      friend class FileStream;

      friend class INodeIO;

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
      uint32_t getId() const;

      void setId(uint32_t id);

      void setFolder(bool folder);

      uint64_t getRefCount() const;

      /**
       * @return zda-li je INode volna
       */
      [[nodiscard]] bool isFree() const;

      /**
       * @return zda-li je INode slozka
       */
      [[nodiscard]] bool isFolder() const;

      /**
       * @return seznam primych adres
       */
      [[nodiscard]] const std::vector<uint64_t>& getT0AddressList() const;

      /**
       * @return vrati odkaz na 1. neprimy odkaz
       */
      [[nodiscard]] uint64_t getT1Address() const;

      /**
       * @return vrati odkaz na 2. neprimy odkaz
       */
      [[nodiscard]] uint64_t getT2Address() const;

      /**
       * @return vrati velikost souboru
       */
      uint32_t getSize() const;

      /**
       * alias pro logictejsi kod pro INode, ktera reprezentuje slozku
       * @return vrati pocet predmetu ve slozce
       */
      uint32_t getFolderSize() const;

      /**
       * nastavi INode velikost
       * @param size velikost, ktera se nastavi
       */
      void setSize(uint32_t size);

      /**
       * Nastavi danou primou adresu na jinou
       * @param index index prime adresy
       * @param address adresa, ktera se nastavi
       */
      void setDirectAddress(uint32_t index, uint64_t address);

      /**
       * Setter pro 1. neprimy odkaz
       * @param t1Address adresa, na kterou se nastavi
       */
      void setT1Address(uint64_t t1Address);

      /**
       * Setter pro 2. neprimy odkaz
       * @param t2Address adresa, na kterou se nastavi
       */
      void setT2Address(uint64_t t2Address);

      /**
       * Konstruktor pro ulozeni do souboru
       * @param isFolder zda-li se jedna o slozku
       * @param id id INode
       */
      INode(bool isFolder, uint32_t id);

      /**
       * Empty konstruktor pro naplneni pri cteni ze souboru
       */
      INode();

      void incrFolderItemCount();

      void incrRefCount();

      bool operator==(const INode& rhs) const;

      bool operator!=(const INode& rhs) const;

      bool isFolderFull() const;

      static INode createRoot();

      void setTimestamp(uint64_t timestamp);

      void printInfo();
};


#endif //ZOSREWORK_INODE_HPP
