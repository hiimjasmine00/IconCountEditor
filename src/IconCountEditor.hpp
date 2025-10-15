#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

class IconCountEditor {
public:
    static cocos2d::CCSprite* createSprite(std::string_view prefix, int id, std::string_view suffix = "");
    static void configureHook(geode::Hook* hook, std::initializer_list<IconType> types);
    static int getCount(IconType type);
    static double random();
};
