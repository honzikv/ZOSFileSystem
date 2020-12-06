
#ifndef PATHINFO_HPP
#define PATHINFO_HPP


#include <string>
#include <map>
#include <vector>
#include "../io/model/INode.hpp"
#include "../util/FSException.hpp"

/**
 * Trida s informacemi o aktualni ceste - tzn. soubory, velikost souboru, slozky apod.
 * Slouzi take pri validaci uzivatelskeho vstupu
 */
class PathInfo {

      std::map<std::string, INode> itemMap; // mapping kdy string je nazev slozky / souboru pro danou inode

    public:
      void saveInfo(const std::pair<std::vector<INode>, std::vector<std::string>>& data); // ulozeni informaci

      int32_t getItemCount() {
          return 2;
      }

      std::vector<std::string> getItemNames();


      bool contains(const std::string& item);

      INode operator[](const std::string& itemName) {
          if (!contains(itemName)) {
              throw FSException("File does not exist in this folder");
          }

          return itemMap[itemName];
      }

};


#endif //PATHINFO_HPP
