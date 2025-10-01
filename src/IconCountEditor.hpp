#include <cocos2d.h>
#include <Geode/Enums.hpp>

class IconCountEditor {
public:
    static cocos2d::CCSprite* createSprite(std::string_view prefix, int id, std::string_view suffix = "");
    static std::map<IconType, std::pair<int, bool>>& getCounts();
    static int getCount(IconType type);
    static double random();
};
