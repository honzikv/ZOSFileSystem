
#ifndef ADDRESSTYPE_H
#define ADDRESSTYPE_H

/**
 * Typ adresy, pouzito pro formatovani
 */
enum class AddressType {
      Pointer, // pointer - nastavime na invalid 64 bit
      FolderItem, // folder item - nastavime na prazdny
      RawData // data
};

#endif //ZOSREWORK_ADDRESSTYPE_H
