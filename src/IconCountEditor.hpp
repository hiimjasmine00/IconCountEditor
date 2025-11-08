#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

class IconCountEditor {
public:
    static int clamp(int id, IconType type);
    static cocos2d::CCSprite* createSprite(std::string_view prefix, int id, std::string_view suffix = "");
    static void modify(std::map<std::string, std::shared_ptr<geode::Hook>>& hooks, std::string_view name, std::initializer_list<IconType> types);
    static int getCount(IconType type);
};
