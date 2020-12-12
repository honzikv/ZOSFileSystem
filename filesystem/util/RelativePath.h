
#ifndef ZOSREWORK_RELATIVEPATH_H
#define ZOSREWORK_RELATIVEPATH_H

#include <vector>
#include <cstring>
#include "StringParsing.hpp"
#include <string>
#include <optional>

class RelativePath {

        int currentIndex = 0; // aktualni index pro iteraci

        std::vector<std::string> path; // cesta kdy index 0 je na zacatku a index n je konecna slozka

        std::optional<std::string> fileName = std::nullopt; // jmeno souboru
    public:
        bool hasNext() {
            return currentIndex < path.size();
        }

        std::string next() {
            auto result = path[currentIndex];
            currentIndex += 1;
            return result;
        }

        RelativePath(const std::string& path, bool parseFileName = false) {
            auto result = StringParsing::parsePath(path);
            if (!parseFileName) {
                this->path = result;
            } else {
                auto last = result.back();
                result.pop_back();
                this->path = result;
                this->fileName = last;
            }
        }
};

#endif //ZOSREWORK_RELATIVEPATH_H
