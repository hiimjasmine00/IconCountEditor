#include "IconCountEditor.hpp"
#include <Geode/loader/ModSettingsManager.hpp>
#include <hiimjasmine00.optional_settings/include/OptionalIntSetting.hpp>

using namespace geode::prelude;
using namespace optional_settings;

CCSprite* IconCountEditor::createSprite(std::string_view prefix, int id, std::string_view suffix) {
    auto sprite = CCSprite::createWithSpriteFrameName(fmt::format("{}_{:02}{}_001.png", prefix, id, suffix).c_str());
    if ((!sprite || sprite->getUserObject("geode.texture-loader/fallback"))) {
        sprite = CCSprite::createWithSpriteFrameName(fmt::format("{}_01{}_001.png", prefix, suffix).c_str());
    }
    return sprite;
}

void loadCount(std::map<IconType, std::pair<int, bool>>& counts, ModSettingsManager* manager, IconType type, std::string_view id) {
    if (auto count = static_cast<OptionalIntSetting*>(manager->get(id).get())) {
        auto enabled = count->isEnabled();
        counts.emplace(type, std::make_pair(enabled ? count->getStoredValue() : count->getStoredDefaultValue(), enabled));
    }
}

std::map<IconType, std::pair<int, bool>>& IconCountEditor::getCounts() {
    static std::map<IconType, std::pair<int, bool>> iconCounts = [] {
        std::map<IconType, std::pair<int, bool>> counts;
        auto manager = ModSettingsManager::from(getMod());
        loadCount(counts, manager, IconType::Cube, "cubes");
        loadCount(counts, manager, IconType::Ship, "ships");
        loadCount(counts, manager, IconType::Ball, "balls");
        loadCount(counts, manager, IconType::Ufo, "ufos");
        loadCount(counts, manager, IconType::Wave, "waves");
        loadCount(counts, manager, IconType::Robot, "robots");
        loadCount(counts, manager, IconType::Spider, "spiders");
        loadCount(counts, manager, IconType::Swing, "swings");
        loadCount(counts, manager, IconType::Jetpack, "jetpacks");
        loadCount(counts, manager, IconType::DeathEffect, "death-effects");
        loadCount(counts, manager, IconType::Special, "trails");
        loadCount(counts, manager, IconType::ShipFire, "ship-fires");
        return counts;
    }();
    return iconCounts;
}

int IconCountEditor::getCount(IconType type) {
    auto& counts = getCounts();
    auto it = counts.find(type);
    return it != counts.end() ? it->second.first : 0;
}

double IconCountEditor::random() {
    return (double)rand() / (double)RAND_MAX;
}
