
#ifndef ZOSREWORK_RELATIVEPATH_H
#define ZOSREWORK_RELATIVEPATH_H

#include <utility>
#include <vector>
#include <cstring>
#include "StringParsing.hpp"
#include <string>
#include <optional>

enum class PathType {
      Empty, // prazdna (velikost 0)
      Relative, // relativni - nezacina /
      Absolute // absolutni - zacina /
};

class FileSystemPath {

      std::vector<std::string> path; // cesta kdy index 0 je na zacatku a index n je konecna slozka

      PathType pathType; // typ cesty

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
          auto parseString = pathType == PathType::Absolute ? path.substr(1) : path;

          auto result = StringParsing::parsePath(parseString);
          this->path = result;
      }

      /**
       * [] operátor pro snažší přístup než přes get metody
       * @param i index prvku
       * @return prvek na i-tém indexu
       */
      std::string operator[](int i) {
          return path[i];
      }

      /**
       * Vrátí poslední prvek, stejně jako vektor
       * @return poslední prvek v cestě
       */
      std::string back() {
          return path.back();
      }

      /**
       * Uvolní poslední prvek z cesty a vrátí ho
       * @return poslední prvek cesty
       */
      std::string releaseBack() {
          auto back = path.back();
          path.pop_back();
          return back;
      }

      /**
       * Uvolní poslední prvek cesty
       */
      void popBack() {
          path.pop_back();
      }
};

#endif //ZOSREWORK_RELATIVEPATH_H
