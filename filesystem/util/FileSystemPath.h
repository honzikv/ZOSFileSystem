
#ifndef ZOSREWORK_RELATIVEPATH_H
#define ZOSREWORK_RELATIVEPATH_H

#include <vector>
#include <cstring>
#include "StringParsing.hpp"
#include <string>
#include <optional>

enum class PathType {
      Empty,
      Relative,
      Absolute
};

class FileSystemPath {

      int currentIndex = 0; // aktualni index pro iteraci

      std::vector<std::string> path; // cesta kdy index 0 je na zacatku a index n je konecna slozka

      PathType pathType;

    public:
      [[nodiscard]] PathType getPathType() const {
          return pathType;
      }

      bool hasNext() {
          return currentIndex < path.size();
      }

      uint32_t size() {
          return path.size();
      }

      bool isComplete() {
          return currentIndex == path.size();
      }

      bool hasLastElement() {
          return currentIndex != path.size() - 1;
      }

      std::string get() {
          return path[currentIndex];
      }

      void next() {
          currentIndex += 1;
      }

      explicit FileSystemPath(const std::string& path) {
          if (path.empty()) {
              pathType = PathType::Empty;
              return;
          }
          // prepise se na local pokud je relativni a ma velikost 1
          pathType = path.starts_with('/') ? PathType::Absolute
                                           : PathType::Relative;
          auto result = StringParsing::parsePath(path);
          this->path = result;
      }
};

#endif //ZOSREWORK_RELATIVEPATH_H
