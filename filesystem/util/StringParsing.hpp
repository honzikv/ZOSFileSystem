#ifndef STRINGPARSING_HPP
#define STRINGPARSING_HPP

#include "ConversionUtils.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

namespace StringParsing {


    bool is_number(const std::string& s) {
        return !s.empty() && std::find_if(s.begin(),
                                          s.end(), [](auto c) { return !std::isdigit(c); }) == s.end();
    }

    UnitType getUnits(std::string units);

    uint64_t parseNumber(std::string& input, int& lastIndex);

    uint64_t getSizeBytes(std::vector<std::string>& args) {
        if (args.size() == 1) {
            if (is_number(args[0])) {
                uint64_t value;
                auto iss = std::istringstream(args[0]);
                iss >> value;
                return value;
            } else {
                auto input = args[0];
                auto index = -1;
                auto size = parseNumber(input, index);
                auto units = std::vector<char>();
                for (auto i = index; i < args[0].size(); i++) {
                    units.push_back(args[0][i]);
                }
                auto unitType = getUnits(std::string(units.begin(), units.end()));

                return ConversionUtils::convert(unitType, size);
            }
        }

        if (args.size() == 2) {
            auto lastIndex = args[0].size() - 1;
            auto parsedLastIndex = -1;
            auto size = parseNumber(args[0], parsedLastIndex);
            if (parsedLastIndex != lastIndex) {
                throw FSException("Error incorrect input for format");
            }
            return ConversionUtils::convert(getUnits(args[1]), size);
        }

        throw FSException("Error incorrect input for format");
    }

    uint64_t parseNumber(std::string& input, int& lastIndex) {
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
        return value;
    }

    UnitType getUnits(std::string units) {
        std::transform(units.begin(), units.end(), units.begin(),
                       [](auto c) { return std::tolower(c); });
        if (units == "b") {
            return UnitType::Byte;
        } else if (units == "kb" || units == "k") {
            return UnitType::Kilobyte;
        } else if (units == "mb" || units == "m") {
            return UnitType::Megabyte;
        } else if (units == "gb" || units == "g") {
            return UnitType::Gigabyte;
        }

        throw FSException("Error, incorrect unit");
    }

}

#endif //STRINGPARSING_HPP
