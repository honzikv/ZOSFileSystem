
#ifndef CLI_HPP
#define CLI_HPP


#include <memory>
#include <unordered_set>
#include <vector>
#include "../components/FileSystem.hpp"

/**
 * Commandline interface pro ovladani aplikace
 */
class CLI {

      bool running = true; // pokud running, neukonci se while loop

      std::string filePath; // cesta k souboru

      std::unique_ptr<FileSystem> fileSystem; // pointer na file system

      /**
       * Provede dany skript
       * @param tokens - radky z precteneho souboru
       */
      void executeScript(const std::vector<std::string>& tokens);

      /**
       * Provede dany prikaz
       * @param tokens tokeny - slova rozdelene podle bilych znaku
       */
      void execute(const std::vector<std::string>& tokens);

      /**
       * Tisk napovedy do konzole
       */
      static void printHelp();

      /**
       * Rozdeli vstup na slova podle bilych znaku
       * @param input vstup od uzivatele
       * @return vektor s rozdelenymi slovy podle bilych znaku
       */
      static std::vector<std::string> splitByWhitespace(const std::string& input);

    public:
      explicit CLI(std::string filePath);

      /**
       * Spusti samotne CLI
       */
      void run();

};


#endif //CLI_HPP
