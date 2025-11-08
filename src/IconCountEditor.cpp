#include "IconCountEditor.hpp"
#include <Geode/loader/Hook.hpp>
#include <hiimjasmine00.optional_settings/include/OptionalIntSetting.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/setting.hpp>

using namespace geode::prelude;
using namespace optional_settings;

std::map<IconType, std::pair<int, bool>>* counts = [] {
    auto counts = new std::map<IconType, std::pair<int, bool>>();
    constexpr std::array types = {
        std::make_pair(IconType::Cube, "cubes"),
        std::make_pair(IconType::Ship, "ships"),
        std::make_pair(IconType::Ball, "balls"),
        std::make_pair(IconType::Ufo, "ufos"),
        std::make_pair(IconType::Wave, "waves"),
        std::make_pair(IconType::Robot, "robots"),
        std::make_pair(IconType::Spider, "spiders"),
        std::make_pair(IconType::Swing, "swings"),
        std::make_pair(IconType::Jetpack, "jetpacks"),
        std::make_pair(IconType::DeathEffect, "death-effects"),
        std::make_pair(IconType::Special, "trails"),
        std::make_pair(IconType::ShipFire, "ship-fires"),
    };
    for (auto [type, key] : types) {
        if (auto count = jasmine::setting::get<std::optional<int>>(key)) {
            auto enabled = count->isEnabled();
            counts->emplace(type, std::make_pair(enabled ? count->getStoredValue() : count->getStoredDefaultValue(), enabled));
        }
    }
    return counts;
}();

int IconCountEditor::clamp(int id, IconType type) {
    return std::clamp(id, 1, getCount(type));
}

CCSprite* IconCountEditor::createSprite(std::string_view prefix, int id, std::string_view suffix) {
    auto sprite = CCSprite::createWithSpriteFrameName(fmt::format("{}_{:02}{}_001.png", prefix, id, suffix).c_str());
    if ((!sprite || sprite->getUserObject("geode.texture-loader/fallback"))) {
        sprite = CCSprite::createWithSpriteFrameName(fmt::format("{}_01{}_001.png", prefix, suffix).c_str());
    }
    return sprite;
}

void IconCountEditor::modify(
    std::map<std::string, std::shared_ptr<geode::Hook>>& hooks, std::string_view name, std::initializer_list<IconType> types
) {
    auto enabled = false;
    for (auto type : types) {
        auto it = counts->find(type);
        if (it != counts->end() && it->second.second) {
            enabled = true;
            break;
        }
    }
    if (auto hook = jasmine::hook::get(hooks, name, enabled)) hook->setPriority(4000);
}

int IconCountEditor::getCount(IconType type) {
    auto it = counts->find(type);
    return it != counts->end() ? it->second.first : 0;
}
