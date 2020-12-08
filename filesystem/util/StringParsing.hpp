#ifndef STRINGPARSING_HPP
#define STRINGPARSING_HPP

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

    uint64_t getSizeBytes(std::vector<std::string>& args) {
        if (args.size() == 1) {
            if (is_number(args[0])) {
                uint64_t value;
                auto iss = std::istringstream(args[0]);
                iss >> value;
                std::cout << value <<std::endl;
                return value;
            }
        }
    }

}

#endif //STRINGPARSING_HPP
