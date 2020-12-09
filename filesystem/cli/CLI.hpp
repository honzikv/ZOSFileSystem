
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

      bool running = true;

      std::string filePath;

      std::unique_ptr<FileSystem> fileSystem;

      void executeScript(const std::vector<std::string>& tokens);

      bool startsWith(const std::string& input, const std::string& token);

      void execute(const std::vector<std::string>& tokens);

      static void printHelp();

      static std::vector<std::string> splitByWhitespace(const std::string& input);

    public:
      explicit CLI(std::string filePath);

      void run();

};


#endif //CLI_HPP
