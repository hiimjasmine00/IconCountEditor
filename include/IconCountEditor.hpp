#include <Geode/Enums.hpp>
#include <Geode/Result.hpp>
#include <map>

/// A class to edit the counts of various icon types in the game.
class IconCountEditor {
private:
    static std::map<IconType, int> iconCounts;
public:
    /// Edits the count of an icon type.
    /// @param type The icon type to edit.
    /// @param count The new count for the icon type.
    static void edit(IconType type, int count) {
        #ifdef GEODE_IS_ANDROID64 // If only GameManager::countForType was hookable...
        count = std::min(count, 65535);
        #endif
        auto iconType = (int)type;
        if (iconType > -1 && (iconType < 9 || iconType > 97) && iconType != 100 && iconType < 102) iconCounts[type] = count;
    }

    /// Gets the count of an icon type.
    /// @param type The icon type to get the count for.
    /// @returns The edited count if it exists, otherwise the original count.
    static int getCount(IconType type) {
        return hasType(type) ? iconCounts[type] : originalCount(type);
    }

    /// Checks if an icon type has been edited.
    /// @param type The icon type to check.
    /// @returns True if the icon type has been edited, otherwise false.
    static bool hasType(IconType type) {
        return iconCounts.contains(type);
    }

    /// Gets the original count of an icon type.
    /// @param type The icon type to get the original count for.
    /// @returns The original count for the icon type.
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

    /// Patches the game to use the edited icon counts.
    /// @returns A Result indicating success or failure.
    static geode::Result<void, void> patch();
};
