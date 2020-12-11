
#include "PathInfo.hpp"

void PathInfo::saveInfo(const std::pair<std::vector<INode>, std::vector<std::string>>& data, uint32_t nodeId) {
    itemMap.clear();
    for (auto i = 0; i < data.first.size(); i++) {
        itemMap.insert(std::pair(data.second[i], data.first[i]));
    }
    this->nodeId = nodeId;
}

std::vector<std::string> PathInfo::getItemNames() {
    auto itemNames = std::vector<std::string>();
    for (auto& it : itemMap) {
        itemNames.push_back(it.first);
    }
    return itemNames;
}

bool PathInfo::contains(const std::string& item) {
    return itemMap.contains(item);
}

uint32_t PathInfo::getNodeId() const {
    return nodeId;
}
