#include "FStreamWrapper.hpp"


void FStreamWrapper::formatSpace(uint64_t bytes) {
    auto bufferFullWrites = bytes / FORMAT_BUFFER_BYTES;
    auto remainder = bytes % FORMAT_BUFFER_BYTES;
    auto buffer = std::vector<char>(FORMAT_BUFFER_BYTES, (int) 0);
    auto remainderBuffer = std::vector<char>(remainder, (int) 0);

    for (auto i = 0; i < bufferFullWrites; i++) { fstream.write(buffer.data(), buffer.size()); }
    fstream.write(remainderBuffer.data(), remainderBuffer.size());
}
