#include <bitset>
#include "Bitmap.hpp"

Bitmap::Bitmap(uint64_t bitmapStartAddress, uint32_t count, uint64_t objectStartAddress, uint64_t objectSizeBytes,
               FileStream& fileStream, std::string name) : bitmapStartAddress(bitmapStartAddress), fstream(fileStream),
                                                           objectSizeBytes(objectSizeBytes),
                                                           objectStartAddress(objectStartAddress),
                                                           name(std::move(name)) {
    fileStream.moveTo(bitmapStartAddress);
    auto bytes = count % 8 > 0 ? (count / 8) + 1 : (count / 8);

    bitmap = std::vector<uint8_t>(bytes, 0);
    fileStream.readVector(bitmap);
}


void Bitmap::updateBitmap(uint64_t bitmapIndex, uint8_t value) {
    fstream.moveTo(bitmapStartAddress + bitmapIndex);
    fstream.write(value);
}


uint64_t Bitmap::getFirstEmptyAddress() {
    uint32_t index = -1; // index adresy v bitmape
    for (auto i = 0; i < bitmap.size(); i += 1) {
        if (bitmap[i] < 0xff) {
            index = i;
            break;
        }
    }

    if (index !=
        -1) { // pokud neni index -1 (resp. preteceni uint32_t) najdeme prvni prazdny bit a vratime danou adresu
        auto bit = getFirstEmptyBit(bitmap[index]);
        return (index * 8 + bit) * objectSizeBytes + objectStartAddress;
    } else {
        throw FSException("Error, bitmap is full");
    }
}

void Bitmap::setAddress(uint64_t address, bool empty) {
    auto objectIndex = (address - objectStartAddress) / objectSizeBytes; // zjisteni indexu objektu
    auto bitmapIndex = objectIndex / 8; // zjisteni indexu v bitmape
    auto bit = objectIndex % 8; // zjisteni bitu v bitmape
    auto bitSet = std::bitset<8>(bitmap[bitmapIndex]);

    bitSet[bit] = empty ? 0 : 1;
    uint8_t byteValue = bitSet.to_ulong();
    bitmap[bitmapIndex] = byteValue;
    updateBitmap(bitmapIndex, byteValue);
}

uint8_t Bitmap::getFirstEmptyBit(uint8_t byte) {
    auto bitSet = std::bitset<8>(byte); // prevedeme na bitset a najdeme bit
    for (auto i = 0; i < bitSet.size(); i += 1) {
        if (bitSet[i] == 0) {
            return i;
        }
    }
    return -1;
}

uint64_t Bitmap::getIdFromAddress(uint64_t itemAddress) const {
    return (itemAddress - objectStartAddress) / objectSizeBytes;
}

void Bitmap::debug() { // vytiskne debug
    std::cout << "BITMAP start address: " << bitmapStartAddress << std::endl;
    for (auto i = 0; i < bitmap.size(); i += 1) {
        if (bitmap[i] > 0) {
            std::cout << "Non empty byte at (" << i << "): " << +bitmap[i] << std::endl;
        }
    }
}
