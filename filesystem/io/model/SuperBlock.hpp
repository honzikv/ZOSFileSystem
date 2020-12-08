
#ifndef SUPERBLOCK_HPP
#define SUPERBLOCK_HPP



#include <cstdint>
#include <string>
#include "../../global/Globals.hpp"
#include "../../util/FStreamWrapper.hpp"

/**
*                               Filesystem
* 0 ......... | ............ | .................... | ..... | ....... EOF
* Super Block | Block Bitmap |       Node Bitmap    | Nodes | Data Blocks
*/
struct SuperBlock {

      uint16_t magicNumber{ }; // cislo pro kontrolu
      uint64_t totalSize{ }; // celkova velikost

      uint64_t blockSize{ }; // velikost jednoho bloku
      uint64_t blockCount{ }; // Number of data blocks
      uint64_t blockBitmapAddress{ }; // Beginning of bitmap with taken blocks

      uint64_t nodeCount{ }; // Total number of nodes
      uint64_t nodeBitmapAddress{ }; // Address of the first node object

      uint64_t nodeAddress{ };
      uint64_t dataAddress{ }; // Start of data blocks

      uint64_t freeNodes{ }; // Pocet volnych inodes, aby se pro prikaz nemuselo pocitat s O(N) slozitosti
      uint64_t freeBlocks{ }; // Pocet volnych datovych bloku, aby se nemuselo pocitat s O(N)

      explicit SuperBlock(uint64_t totalSize = -1);

      friend FStreamWrapper& operator<<(FStreamWrapper& wrapper, SuperBlock& superBlock);

      friend FStreamWrapper& operator>>(FStreamWrapper& wrapper, SuperBlock& superBlock);

      bool isValid() const;

      static uint64_t getBlockCount(uint64_t sizeBytes);

      static uint64_t getNodeCount(uint64_t blockCount);

      static uint64_t getBitmapSize(uint64_t objectCount);

    private:
};


#endif //SUPERBLOCK_HPP
