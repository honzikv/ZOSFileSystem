#ifndef INODE_HPP
#define INODE_HPP


#include <cstdint>
#include <vector>
#include "../../global/Globals.hpp"

class FolderItem;

class FileStream;

/**
 * Reprezentuje INode v FS
 */
class INode {
      friend class FileStream; // aby FileStream mohl pristupovat k privatnim polim bez getteru a setteru
      friend class INodeIO; // aby INodeIO mohlo vyuzivat funkcionality bez zbytecnych getteru

      bool folder = false;
      uint32_t id = (uint32_t) Globals::INVALID_VALUE; // Id. Bude bud vypocteno z adresy INode nebo INVALID_VALUE
      uint32_t size = 0; // Velikost souboru nebo pocet prvku ve slozce
      uint64_t timestamp = Globals::INVALID_VALUE; // Timestamp pro posledni pristup k souboru
      uint64_t refCount = 0; // kolik extra referenci ukazuje na inode - data lze smazat pouze, pokud je refCount = 0

      std::vector<uint64_t> t0AddressList = std::vector(Globals::T0_ADDRESS_LIST_SIZE,
                                                        Globals::INVALID_VALUE); // Seznam vsech primych adres

      uint64_t t1Address = Globals::INVALID_VALUE; // Odkaz na datovy blok s neprimymi adresami
      uint64_t t2Address = Globals::INVALID_VALUE; // Odkaz na blok s odkazy na neprime adresy

    public:
      /**
       * Getter pro id
       * @return id
       */
      [[nodiscard]] uint32_t getId() const;

      /**
       * Nastavi id INode (v Memory Allocator)
       * @param id dane id
       */
      void setId(uint32_t id);

      /**
       * Nastavi, zda-li je INode slozkou
       * @param folder true, pokud je slozka, jinak false
       */
      void setFolder(bool folder);

      [[nodiscard]] uint64_t getRefCount() const;

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
       * alias pro logictejsi kod pro INode, ktera reprezentuje slozku
       * @return vrati pocet predmetu ve slozce
       */
      [[nodiscard]] uint32_t getFolderSize() const;

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

      /**
       * Zvyseni poctu predmetu ve slozce
       */
      void incrFolderItemCount();

      /**
       * Zvyseni poctu referenci
       */
      void incrRefCount();

      /**
       * Operator pro rovnost
       * @param rhs right hand side
       * @return zda-li se dve INodes rovnaji
       */
      bool operator==(const INode& rhs) const;

      /**
       * Operator pro nerovnost
       * @param rhs right hand side
       * @return zda-li se dve INodes nerovnaji
       */
      bool operator!=(const INode& rhs) const;

      /**
       * Vrati, zda-li je slozka plna
       * @return
       */
      [[nodiscard]] bool isFolderFull() const;

      /**
       * Factory metoda pro vytvoreni rootu
       * @return
       */
      static INode createRoot();

      /**
       * Nastavi timestamp (nakonec nevyuzito)
       * @param timestamp
       */
      void setTimestamp(uint64_t timestamp);

      /**
       * Snizi pocet referenci na INode
       */
      void decrRefCount();
};


#endif //ZOSREWORK_INODE_HPP
