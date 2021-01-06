
#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP


#include <cstdint>
#include <memory>
#include <cmath>
#include <unordered_set>
#include "FileSystemController.hpp"
#include "../util/FileStream.hpp"

#define INCORRECT_NUM_PARAMS std::string("Error, incorrect number of parameters for command ")

class SuperBlock;

class FileSystem {

      FileStream& fileStream; // reference na file stream pro zapis na "disk"

      std::unordered_set<std::string> commandList{ // seznam vsech pouzitelnych prikazu
              "format", "mount", "cp", "mv", "rm", "mkdir",
              "rmdir", "ls", "cat", "cd", "pwd", "info",
              "incp", "outcp", "load", "ln",

              // debugovaci prikazy
              "diskinfo", "debugnodebitmap", "debugblockbitmap"
      };

      std::shared_ptr<FileSystemController> fileSystemController = nullptr; // file system controller instance

      bool isMounted = false; // zda-li je disk mounted - tzn. lze pouzit

    public:

      /**
       * Konstruktor vytvori filesystem a ulozi si referenci na filestream
       * @param fstream injektovany filestream
       */
      explicit FileSystem(FileStream& fstream);

      /**
       * Provede format disku
       * @param args argumenty - velikost a jednotky
       */
      void format(std::vector<std::string>& args);

      /**
       * Provede dany prikaz
       * @param commandWithArguments vektor prikazu s argumenty, prvni prvek je prikaz zbytek argumenty
       */
      void execute(const std::vector<std::string>& commandWithArguments);

};

#endif //FILESYSTEM_HPP
