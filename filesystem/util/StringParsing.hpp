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

      static std::pair<uint64_t, int> extractNumber(std::string& input) {
          int lastIndex;
          auto index = 0;
          auto digits = std::vector<char>();
          for (auto c : input) {
              if (isdigit(c)) {
                  index += 1;
                  digits.push_back(c);
              } else {
                  break;
              }
          }
          uint64_t value;
          auto iss = std::istringstream(std::string(digits.begin(), digits.end()));
          iss >> value;
          lastIndex = index;
          return std::pair(value, lastIndex);
      }

      static bool matchesFormatRegex(std::string string) {
          return std::regex_match(string.begin(), string.end(), DRIVE_FORMAT_REGEX);
      }


      static uint64_t parseDriveFormatSize(std::string input) {
          auto results = extractNumber(input);
          auto size = results.first;
          auto unit = std::string(input.begin() + results.second,
                                  input.end()); // parse ze zacatku + index konce cisla az do konce
          return convertToBytes(size, unit);
      }


      static uint64_t convertToBytes(uint64_t unitSize, std::string unit) {
          if (unit == "b") {
              return unitSize;
          } else if (unit == "kb" || unit == "k") {
              return ConversionUtils::kilobytesToBytes(unitSize);
          } else if (unit == "mb" || unit == "m") {
              return ConversionUtils::megabytesToBytes(unitSize);
          } else if (unit == "gb" || unit == "g") {
              return ConversionUtils::gigabyteToBytes(unitSize);
          }
          throw FSException("Error invalid input"); // nemelo by prejit pres regex
      }


      static std::vector<std::string> parsePath(const std::string& path) {
          auto pathString = std::string(path);
          auto delimiter = std::string("/");
          auto pos = 0;

          auto result = std::vector<std::string>();
          auto token = std::string();
          while ((pos = pathString.find(delimiter)) != std::string::npos) {
              token = pathString.substr(0, pos);
              result.push_back(token);
              pathString.erase(0, pos + delimiter.length());
          }

          if (!pathString.empty()) {
              result.push_back(pathString);
          }

          return result;
      }
};

#endif //STRINGPARSING_HPP
