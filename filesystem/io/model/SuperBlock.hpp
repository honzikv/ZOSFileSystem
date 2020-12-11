
#ifndef SUPERBLOCK_HPP
#define SUPERBLOCK_HPP


#include <cstdint>
#include <string>
#include <ostream>
#include "../../global/Globals.hpp"
#include "../../util/FileStream.hpp"

/**
*                               Filesystem
* 0 ......... | ............ | .................... | ..... | ....... EOF
* Super Block | Block Bitmap |       Node Bitmap    | Nodes | Data Blocks
*/
struct SuperBlock {

    uint16_t magicNumber = (uint16_t) Globals::INVALID_VALUE; // cislo pro kontrolu
    uint64_t totalSize = Globals::INVALID_VALUE; // celkova velikost

    uint64_t blockSize = Globals::INVALID_VALUE; // velikost jednoho bloku
    uint64_t blockCount = Globals::INVALID_VALUE; // Number of data blocks
    uint64_t blockBitmapAddress = Globals::INVALID_VALUE; // Beginning of bitmap with taken blocks

    uint64_t nodeCount = Globals::INVALID_VALUE; // Total number of nodes
    uint64_t nodeBitmapAddress = Globals::INVALID_VALUE; // Address of the first node object

    uint64_t nodeAddress = Globals::INVALID_VALUE;
    uint64_t dataAddress = Globals::INVALID_VALUE; // Start of data blocks

    uint64_t freeNodes = Globals::INVALID_VALUE; // Pocet volnych inodes, aby se pro prikaz nemuselo pocitat s O(N) slozitosti
    uint64_t freeBlocks = Globals::INVALID_VALUE; // Pocet volnych datovych bloku, aby se nemuselo pocitat s O(N)

    explicit SuperBlock(uint64_t totalSize);

    SuperBlock() = default;

    friend FileStream& operator>>(FileStream& fileStream, SuperBlock& superBlock);

    [[nodiscard]] bool isValid() const;

    static uint64_t getBlockCount(uint64_t sizeBytes);

    static uint64_t getNodeCount(uint64_t blockCount);

    static uint64_t getBitmapSize(uint64_t objectCount);

    friend std::ostream& operator<<(std::ostream& os, const SuperBlock& block);

    void printInfo() const;
};


#endif //SUPERBLOCK_HPP
