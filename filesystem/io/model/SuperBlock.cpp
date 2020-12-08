
#include <iostream>
#include "SuperBlock.hpp"
#include "../../util/ConversionUtils.hpp"

bool SuperBlock::isValid() const {
    return magicNumber == Globals::SUPER_BLOCK_MAGIC_NUMBER;
}

uint64_t SuperBlock::getBitmapSize(uint64_t objectCount) {
    auto count = objectCount / 8;
    return objectCount % 8 == 0 ? count : count + 1;
}

uint64_t SuperBlock::getNodeCount(uint64_t blockCount) {
    auto count = blockCount / 4;
    return blockCount % Globals::BLOCKS_PER_INODE == 0 ? count : count + 1;
}

uint64_t SuperBlock::getBlockCount(uint64_t sizeBytes) {
    auto count = sizeBytes / Globals::BLOCK_SIZE_BYTES;
    return sizeBytes % Globals::BLOCK_SIZE_BYTES == 0 ? count : count + 1;
}

FStreamWrapper& operator<<(FStreamWrapper& fstream, SuperBlock& superBlock) {
//    fstream.write(superBlock.magicNumber, superBlock.totalSize, superBlock.blockBitmapAddress,
//                  superBlock.blockSize, superBlock.blockCount, superBlock.blockBitmapAddress,
//                  superBlock.nodeCount, superBlock.nodeBitmapAddress,
//                  superBlock.nodeAddress, superBlock.dataAddress);
    fstream.write(superBlock.magicNumber);
    fstream.write(superBlock.totalSize);
    fstream.write(superBlock.blockBitmapAddress);
    fstream.write(superBlock.blockSize);
    fstream.write(superBlock.blockCount);
    fstream.write(superBlock.blockBitmapAddress);
    fstream.write(superBlock.nodeCount);
    fstream.write(superBlock.nodeBitmapAddress);
    fstream.write(superBlock.nodeAddress);
    fstream.write(superBlock.dataAddress);
    return fstream;
}

FStreamWrapper& operator>>(FStreamWrapper& fstream, SuperBlock& superBlock) {
//    fstream.read(superBlock.magicNumber, superBlock.totalSize, superBlock.blockBitmapAddress,
//                 superBlock.blockSize, superBlock.blockCount, superBlock.blockBitmapAddress,
//                 superBlock.nodeCount, superBlock.nodeBitmapAddress,
//                 superBlock.nodeAddress, superBlock.dataAddress);

    fstream.read(superBlock.magicNumber);
    fstream.read(superBlock.totalSize);
    fstream.read(superBlock.blockBitmapAddress);
    fstream.read(superBlock.blockSize);
    fstream.read(superBlock.blockCount);
    fstream.read(superBlock.blockBitmapAddress);
    fstream.read(superBlock.nodeCount);
    fstream.read(superBlock.nodeBitmapAddress);
    fstream.read(superBlock.nodeAddress);
    fstream.read(superBlock.dataAddress);

    return fstream;
}

SuperBlock::SuperBlock(uint64_t size) {
    std::cout << size << std::endl;
    blockCount = getBlockCount(size);
    nodeCount = getNodeCount(blockCount);
    auto nodeBitmapSize = getBitmapSize(nodeCount);
    auto blockBitmapSize = getBitmapSize(blockCount);

    totalSize = (uint64_t) (Globals::INODE_SIZE_BYTES() * nodeCount + nodeBitmapSize + blockBitmapSize +
                            Globals::SUPERBLOCK_SIZE_BYTES + blockCount * Globals::BLOCK_SIZE_BYTES);

    blockSize = Globals::BLOCK_SIZE_BYTES;
    nodeAddress = Globals::SUPERBLOCK_SIZE_BYTES;
    nodeBitmapAddress = nodeAddress + Globals::INODE_SIZE_BYTES() * nodeCount;
    blockBitmapAddress = nodeBitmapAddress + nodeBitmapSize;
    dataAddress = blockBitmapAddress + blockBitmapSize;
    magicNumber = Globals::SUPER_BLOCK_MAGIC_NUMBER;

    //debug
    std::cout << "blocks: " << blockCount << " -> "
              << ConversionUtils::bytesToMegabytes(blockCount * Globals::BLOCK_SIZE_BYTES) << " MB" << std::endl
              << "nodes: " << nodeCount << " -> "
              << ConversionUtils::bytesToMegabytes(nodeCount * Globals::INODE_SIZE_BYTES()) << " MB" << std::endl
              << "node bitmap: " << ConversionUtils::bytesToKilobytes(nodeBitmapSize) << " kB " << std::endl
              << "block bitmap: " << ConversionUtils::bytesToKilobytes(blockBitmapSize) << " kB " << std::endl
              << "valid = " << isValid() << std::endl;
}
