
#ifndef ZOSREWORK_RELATIVEPATH_H
#define ZOSREWORK_RELATIVEPATH_H

#include <utility>
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

      uint32_t size() {
          return path.size();
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

      std::string operator[](int i) {
          return path[i];
      }

      std::string back() {
          return path.back();
      }

      std::string releaseBack() {
          auto back = path.back();
          path.pop_back();
          return back;
      }

      void popBack() {
          path.pop_back();
      }
};

#endif //ZOSREWORK_RELATIVEPATH_H
