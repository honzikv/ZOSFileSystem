
#ifndef FILESYSTEMCONTROLLER_HPP
#define FILESYSTEMCONTROLLER_HPP

#include <string>
#include "../util/FStreamWrapper.hpp"
#include "MemoryAllocator.hpp"

/**
 * Trida, ktera slouzi pro ovladani filesystemu
 * Obstarava jednotlive prikazy volane cli
 */
class FileSystemController {

      FStreamWrapper fstream;

      std::unique_ptr<MemoryAllocator> memoryAllocator;

    public:

      explicit FileSystemController(const std::string& fileName);
};


#endif //FILESYSTEMCONTROLLER_HPP
