#include <Geode/Enums.hpp>
#include <map>

class IconCountEditor {
public:
    static std::map<IconType, std::pair<int, bool>>& getCounts();
    static int getCount(IconType type);
    static double random();
};
