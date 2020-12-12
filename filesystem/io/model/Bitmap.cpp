#include <bitset>
#include "Bitmap.hpp"
#include "../../util/FSException.hpp"

Bitmap::Bitmap(uint64_t bitmapStartAddress, uint32_t count, uint64_t objectStartAddress, uint64_t sizeOfObject,
               FileStream& fileStream) :
        bitmapStartAddress(bitmapStartAddress), fstream(fileStream), sizeOfObject(sizeOfObject),
        objectStartAddress(objectStartAddress) {
    fileStream.moveTo(bitmapStartAddress);
    auto bytes = count % 8 > 0 ? (count / 8) + 1 : (count / 8);

    bitmap = std::vector<uint8_t>(bytes, 0);
    fileStream.readVector(bitmap);
}

bool Bitmap::setPosition(uint8_t value, uint32_t pos, FileStream& fileStream) {
    auto index = pos / 8;
    auto bit = pos % 8;
    auto bitSet = std::bitset<8>(bitmap[index]);
    bitSet[bit] = value;
    auto byteValue = (uint8_t) bitSet.to_ulong();
    bitmap[index] = byteValue;
    fileStream.moveTo(bitmapStartAddress + pos / 8);
    fileStream.write(byteValue);
    return true;
}

uint32_t Bitmap::findFirstEmpty() {
    auto full = uint8_t(0xff);
    for (auto i : bitmap) {
        if (i != full) {
            return i;
        }
    }
    return -1;
}

void Bitmap::getEmptyPositions(std::vector<uint8_t>& positions, uint32_t index) {
    auto bitSet = std::bitset<8>(bitmap[index]);
    for (auto i = 0; i < bitSet.size(); i++) {
        if (bitSet[i] == 0) { positions.push_back(i); }
    }
}

void Bitmap::updateBitmap(uint64_t bitmapIndex, uint8_t value) {
    fstream.moveTo(bitmapStartAddress + bitmapIndex);
    fstream.write(value);
}

bool Bitmap::isAddressEmpty(uint64_t address) {
    auto bitmapAddress = (address - bitmapStartAddress) / sizeOfObject;
    auto bitmapIndex = bitmapAddress / 8;
    auto bit = bitmapAddress % 8;

    auto bitSet = std::bitset<8>(bitmap[bitmapIndex]);
    return bitSet[bit] == 0;
}

uint64_t Bitmap::getFirstEmptyAddress() {
    auto full = uint8_t(0xff);
    uint32_t index = -1;
    for (auto i = 0; i < bitmap.size(); i++) {
        if (bitmap[i] != full) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        auto bit = getFirstEmptyBit(bitmap[index]);
        return (index * 8 + bit) * sizeOfObject + objectStartAddress;
    } else {
        throw FSException("Error bitmap is full");
    }
}

void Bitmap::setAddress(uint64_t address, bool empty) {
    auto bitmapAddress = (address - objectStartAddress) / sizeOfObject;
    auto bitmapIndex = bitmapAddress / 8;
    auto bit = bitmapAddress % 8;

    auto bitSet = std::bitset<8>(bitmap[bitmapIndex]);
    bitSet[bit] = !empty;
    uint8_t byteValue = bitSet.to_ulong();
    bitmap[bitmapIndex] = byteValue;
    updateBitmap(bitmapIndex, byteValue);
}

uint8_t Bitmap::getFirstEmptyBit(uint8_t byte) {
    auto bitSet = std::bitset<8>(byte);
    for (auto i = 0; i < bitSet.size(); i++) {
        if (bitSet[i] == 0) {
            return i;
        }
    }
    return -1;
}
