#include <Geode/Enums.hpp>
#include <Geode/Result.hpp>
#include <map>

class IconCountEditor {
private:
    inline static std::map<IconType, int> iconCounts;
public:
    static void edit(IconType type, int count) {
        auto iconType = (int)type;
        if (iconType > -1 && (iconType < 9 || iconType > 97) && iconType != 100 && iconType < 102) iconCounts[type] = count;
    }

    static int getCount(IconType type) {
        return hasType(type) ? iconCounts[type] : originalCount(type);
    }

    static bool hasType(IconType type) {
        return iconCounts.contains(type);
    }

    static int originalCount(IconType type) {
        switch(type) {
            case IconType::Cube: return 485;
            case IconType::Ship: return 169;
            case IconType::Ball: return 118;
            case IconType::Ufo: return 149;
            case IconType::Wave: return 96;
            case IconType::Robot: return 68;
            case IconType::Spider: return 69;
            case IconType::Swing: return 43;
            case IconType::Jetpack: return 8;
            case IconType::DeathEffect: return 20;
            case IconType::Special: return 7;
            case IconType::ShipFire: return 6;
            default: return 0;
        }
    }

    static geode::Result<> patch();
};
