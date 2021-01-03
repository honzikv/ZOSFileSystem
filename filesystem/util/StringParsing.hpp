#ifndef STRINGPARSING_HPP
#define STRINGPARSING_HPP

#include "ConversionUtils.hpp"
#include "FSException.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <regex>

class StringParsing {
    public:

      static inline const std::regex DRIVE_FORMAT_REGEX = std::regex("\\d+(b|kb|mb|gb|k|m|g)");

      static std::pair<uint64_t, int> extractNumber(std::string& input);

      static bool matchesFormatRegex(std::string string);


      static uint64_t parseDriveFormatSize(std::string input);


      static uint64_t convertToBytes(uint64_t unitSize, std::string unit);


      static std::vector<std::string> parsePath(const std::string& path);
};

#endif //STRINGPARSING_HPP
