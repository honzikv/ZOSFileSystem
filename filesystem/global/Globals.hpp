#ifndef GLOBALS_HPP
#define GLOBALS_HPP


#include <cstdint>
#include <limits>
#include <string>

/**
Globalni hodnoty pro snazsi pristup
 */
struct Globals {

    static constexpr float BLOCK_RATIO = 0.9; // bloky : inodes
    static constexpr float INODE_RATIO = 0.1; // inodes : blokum

    static constexpr uint64_t MIN_DRIVE_SIZE = 1024 * 10 * 1024; // minimalni velikost disku (10 MB)
    static constexpr uint16_t SUPER_BLOCK_MAGIC_NUMBER = 0x53ef; // magicke cislo pro kontrolu
    static constexpr uint64_t SUPERBLOCK_SIZE_BYTES = 10 * sizeof(uint64_t) + sizeof(uint16_t); // velikost super bloku

    static constexpr uint64_t INVALID_VALUE = std::numeric_limits<uint64_t>::max(); // invalid value pro kontrolu
    static inline uint32_t BLOCK_SIZE_BYTES = 4096;
    static inline uint32_t BLOCKS_PER_INODE = 4;

    // limity pro soubory
    static constexpr uint8_t FILE_NAME_CHAR_LIMIT = 8; // jmeno souboru napr. a1
    static constexpr uint8_t FILE_EXTENSION_CHAR_LIMIT = 3; // koncovka - .exe, .bin ...
    static constexpr uint8_t MAX_FILE_STRING_LENGTH =
            FILE_NAME_CHAR_LIMIT + FILE_EXTENSION_CHAR_LIMIT; // jmeno + koncovka
    static constexpr uint8_t FILE_NAME_CHAR_ARRAY_LENGTH = MAX_FILE_STRING_LENGTH + 1; // char array jmena + '\0'

    static constexpr uint8_t FOLDER_ITEM_SIZE_BYTES =
            sizeof(uint64_t) + FILE_NAME_CHAR_ARRAY_LENGTH * sizeof(char); //17



    // konstanty pro inode
    static constexpr uint8_t T0_ADDRESS_LIST_SIZE = 12;

    static uint64_t INODE_SIZE_BYTES() {
        return sizeof(bool) //folder
               + 2 * sizeof(uint32_t) // id, size
               + 2 * sizeof(uint64_t) // timestamp, refcount
               + Globals::T0_ADDRESS_LIST_SIZE * sizeof(uint64_t) // prime adresy
               + 2 * sizeof(uint64_t); // t1 a t2 adresy

    }

    static uint64_t FOLDER_ITEMS_PER_BLOCK() { return BLOCK_SIZE_BYTES % FOLDER_ITEM_SIZE_BYTES; }

    static constexpr uint64_t POINTER_SIZE_BYTES = sizeof(uint64_t); // alias

    static uint64_t POINTERS_PER_BLOCK() {
        return BLOCK_SIZE_BYTES / sizeof(uint64_t);
    } // kolik pointeru muze byt v jednom bloku

    static uint64_t T0_CAPACITY() { return FOLDER_ITEMS_PER_BLOCK(); } // kapacita jednoho T0 odkazu - tzn. primeho

    [[maybe_unused]] static uint64_t T0_ADDRESS_LIST_CAPACITY() { return T0_CAPACITY() * T0_ADDRESS_LIST_SIZE; }

    static uint64_t T0_ADDRESS_LIST_CAPACITY_BYTES() { return BLOCK_SIZE_BYTES * T0_ADDRESS_LIST_SIZE; }

    static uint64_t T1_POINTER_CAPACITY_BYTES() { return POINTERS_PER_BLOCK() * BLOCK_SIZE_BYTES; };

    static uint64_t T2_POINTER_CAPACITY_BYTES() { return POINTERS_PER_BLOCK() * T1_POINTER_CAPACITY_BYTES(); };

    static uint64_t T1_CAPACITY() {
        return T0_CAPACITY() * POINTERS_PER_BLOCK();
    }// kapacita jednoho T1 odkazu - tzn 1. neprimeho

    [[maybe_unused]] static uint64_t T2_CAPACITY() {
        return T1_CAPACITY() * POINTERS_PER_BLOCK();
    }

    static uint32_t maxAddressableBlocksPerINode() { // max pocet adresovatelnych bloku jednou INode
        return T0_ADDRESS_LIST_SIZE + POINTERS_PER_BLOCK() + POINTERS_PER_BLOCK() * POINTERS_PER_BLOCK();
    }

    [[maybe_unused]] static bool isFileTooLarge(uint64_t bytes) { // zda-li je soubor moc velky pro filesystem (nevyuzito)
        return bytes > (T0_ADDRESS_LIST_CAPACITY_BYTES() + T1_POINTER_CAPACITY_BYTES() + T2_POINTER_CAPACITY_BYTES());
    }

    static uint32_t getBlockCount(uint64_t bytes) { // pocet bloku z bytu
        return bytes % BLOCK_SIZE_BYTES == 0 ? bytes / BLOCK_SIZE_BYTES : bytes / BLOCK_SIZE_BYTES + 1;
    }

    static uint32_t MAX_FOLDER_ITEMS() { // max pocet predmetu ve slozce
        return T0_ADDRESS_LIST_SIZE * FOLDER_ITEMS_PER_BLOCK() + POINTERS_PER_BLOCK() * FOLDER_ITEMS_PER_BLOCK() +
               POINTERS_PER_BLOCK() * POINTERS_PER_BLOCK() * FOLDER_ITEMS_PER_BLOCK();
    }

    static inline const std::string CURRENT_FOLDER_SYMBOL = "."; // symbol pro aktualni slozku
    static inline const std::string PARENT_FOLDER_SYMBOL = ".."; // symbol pro nadrazenou slozku


    static uint64_t MAX_FILE_SIZE_BYTES() {
        return (T0_ADDRESS_LIST_SIZE +// prime linky
                POINTERS_PER_BLOCK() + // 1. neprimy odkaz
                POINTERS_PER_BLOCK() * POINTERS_PER_BLOCK()  // 2. neprimy odkaz
               ) * BLOCK_SIZE_BYTES; // x velikost v bytech
    }
};

#endif //GLOBALS_HPP
