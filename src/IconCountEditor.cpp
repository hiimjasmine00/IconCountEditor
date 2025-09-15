#include "IconCountEditor.hpp"
#include <Geode/loader/Mod.hpp>
#include <hiimjasmine00.optional_settings/include/OptionalIntSetting.hpp>

using namespace geode::prelude;
using namespace optional_settings;

CCSprite* IconCountEditor::createSprite(std::string_view frameName, std::string_view fallbackFrameName) {
    auto sprite = CCSprite::createWithSpriteFrameName(frameName.data());
    if ((!sprite || sprite->getUserObject("geode.texture-loader/fallback")) && !fallbackFrameName.empty()) {
        sprite = CCSprite::createWithSpriteFrameName(fallbackFrameName.data());
    }
    return sprite;
}

void loadCount(std::map<IconType, std::pair<int, bool>>& counts, Mod* mod, IconType type, std::string_view id) {
    if (auto count = static_cast<OptionalIntSetting*>(mod->getSetting(id).get())) {
        auto enabled = count->isEnabled();
        counts.emplace(type, std::make_pair(enabled ? count->getStoredValue() : count->getStoredDefaultValue(), enabled));
    }
}

std::map<IconType, std::pair<int, bool>>& IconCountEditor::getCounts() {
    static std::map<IconType, std::pair<int, bool>> iconCounts = [] {
        std::map<IconType, std::pair<int, bool>> counts;
        auto mod = Mod::get();
        if (auto res = mod->registerCustomSettingType("dummy", [](const std::string& key, const std::string& id, const matjson::Value& json) {
            return Err("Dummy setting type - should never be parsed");
        }); res.isErr()) {
            log::logImpl(Severity::Error, mod, "Failed to register dummy setting type: {}", res.unwrapErr());
        }
        loadCount(counts, mod, IconType::Cube, "cubes");
        loadCount(counts, mod, IconType::Ship, "ships");
        loadCount(counts, mod, IconType::Ball, "balls");
        loadCount(counts, mod, IconType::Ufo, "ufos");
        loadCount(counts, mod, IconType::Wave, "waves");
        loadCount(counts, mod, IconType::Robot, "robots");
        loadCount(counts, mod, IconType::Spider, "spiders");
        loadCount(counts, mod, IconType::Swing, "swings");
        loadCount(counts, mod, IconType::Jetpack, "jetpacks");
        loadCount(counts, mod, IconType::DeathEffect, "death-effects");
        loadCount(counts, mod, IconType::Special, "trails");
        loadCount(counts, mod, IconType::ShipFire, "ship-fires");
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
