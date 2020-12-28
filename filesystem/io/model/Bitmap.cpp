#include <bitset>
#include <utility>
#include "Bitmap.hpp"
#include "../../util/FSException.hpp"

Bitmap::Bitmap(uint64_t bitmapStartAddress, uint32_t count, uint64_t objectStartAddress, uint64_t objectSizeBytes,
               FileStream& fileStream, std::string name) : bitmapStartAddress(bitmapStartAddress), fstream(fileStream),
                                                           objectSizeBytes(objectSizeBytes),
                                                           objectStartAddress(objectStartAddress), name(std::move(name)) {
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


void Bitmap::updateBitmap(uint64_t bitmapIndex, uint8_t value) {
    fstream.moveTo(bitmapStartAddress + bitmapIndex);
    fstream.write(value);
}


uint64_t Bitmap::getFirstEmptyAddress() {
    uint32_t index = -1;
    for (auto i = 0; i < bitmap.size(); i += 1) {
        if (bitmap[i] < 0xff) {
            std::cout << name << "found empty byte on index: " << +i << std::endl;
            index = i;
            break;
        }
    }

    if (index != -1) {
        auto bit = getFirstEmptyBit(bitmap[index]);
        return (index * 8 + bit) * objectSizeBytes + objectStartAddress;
    } else {
        throw FSException("Error bitmap is full");
    }
}

void Bitmap::setAddress(uint64_t address, bool empty) {
    auto objectIndex = (address - objectStartAddress) / objectSizeBytes; // zjisteni indexu objektu
    auto bitmapIndex = objectIndex / 8; // zjisteni indexu v bitmape
    auto bit = objectIndex % 8; // zjisteni bitu v bitmape
    auto bitSet = std::bitset<8>(bitmap[bitmapIndex]);

    bitSet[bit] = empty ? 0 : 1;
    uint8_t byteValue = bitSet.to_ulong();
    std::cout << name <<   std::endl;
    bitmap[bitmapIndex] = byteValue;
    updateBitmap(bitmapIndex, byteValue);
}

uint8_t Bitmap::getFirstEmptyBit(uint8_t byte) {
    auto bitSet = std::bitset<8>(byte);
    for (auto i = 0; i < bitSet.size(); i += 1) {
        if (bitSet[i] == 0) {
            std::cout << name <<  " found empty bit: " << i << std::endl;
            return i;
        }
    }
    return -1;
}

uint64_t Bitmap::getIdFromAddress(uint64_t itemAddress) const {
    return (itemAddress - objectStartAddress) / objectSizeBytes;
}

void Bitmap::debug() {
    std::cout << "BITMAP start address: " << bitmapStartAddress << std::endl;
    for (auto i = 0; i < bitmap.size(); i += 1) {
        if (bitmap[i] > 0) {
            std::cout << "Non empty byte at (" << i << "): " << +bitmap[i] << std::endl;
        }
    }
}
