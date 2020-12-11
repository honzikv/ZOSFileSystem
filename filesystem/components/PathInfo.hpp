
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


struct Navigation {


};

class PathInfo {

      std::map<std::string, INode> itemMap; // mapping kdy string je nazev slozky / souboru pro danou inode

      uint32_t nodeId;

    public:

      void saveInfo(const std::pair<std::vector<INode>, std::vector<std::string>>& data, uint32_t nodeId); // ulozeni informaci

      int32_t getItemCount() {
          return itemMap.size();
      }

      std::vector<std::string> getItemNames();

      uint32_t getNodeId() const;

      bool contains(const std::string& item);

      INode operator[](const std::string& itemName) {
          if (!contains(itemName)) {
              throw FSException("File does not exist in this folder");
          }

          return itemMap[itemName];
      }


};


#endif //PATHINFO_HPP
