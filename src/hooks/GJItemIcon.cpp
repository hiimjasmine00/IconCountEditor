#include "../IconCountEditor.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJPathSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJItemIcon.hpp>

using namespace geode::prelude;

class $modify(ICEItemIcon, GJItemIcon) {
    static void onModify(ModifyBase<ModifyDerive<ICEItemIcon, GJItemIcon>>& self) {
        auto& counts = IconCountEditor::getCounts();
        if (auto found = self.m_hooks.find("GJItemIcon::init"); found != self.m_hooks.end()) {
            auto& hook = found->second;
            hook->setAutoEnable(counts[IconType::DeathEffect].second || counts[IconType::Special].second || counts[IconType::ShipFire].second);
            hook->setPriority(Priority::Replace);
        }
    }

    bool init(UnlockType type, int id, ccColor3B color1, ccColor3B color2, bool dark, bool p5, bool noLabel, ccColor3B) {
        if (!CCSprite::init()) return false;

        m_unlockID = id;
        m_unlockType = type;

        auto gm = GameManager::get();
        m_iconRequestID = gm->getIconRequestID();
        m_playerSize.width = 30.0f;
        m_playerSize.height = 30.0f;
        setContentSize(m_playerSize);

        constexpr std::array prefixes = {
            "player", "", "", "ship", "player_ball", "bird", "dart",
            "robot", "spider", "player_special", "explosionIcon",
            "gjItem", "swing", "jetpack", "shipfireIcon"
        };
        auto prefix = prefixes[(int)type - 1];
        switch (type) {
            case UnlockType::Cube:
            case UnlockType::Ship:
            case UnlockType::Ball:
            case UnlockType::Bird:
            case UnlockType::Dart:
            case UnlockType::Robot:
            case UnlockType::Spider:
            case UnlockType::Swing:
            case UnlockType::Jetpack: {
                m_isIcon = true;
                auto player = SimplePlayer::create(1);
                player->updatePlayerFrame(id, gm->unlockTypeToIconType((int)type));
                player->setColors(color1, color2);
                m_playerSize = player->getContentSize();
                m_player = player;
                break;
            }
            case UnlockType::Col1:
            case UnlockType::Col2: {
                auto colorIcon = CCSprite::createWithSpriteFrameName("player_special_01_001.png");
                if (dark) {
                    colorIcon->setColor({ 0, 0, 0 });
                    colorIcon->setOpacity(120);
                }
                else {
                    colorIcon->setColor(gm->colorForIdx(id));
                    if (!noLabel) {
                        auto label = CCLabelBMFont::create(type == UnlockType::Col1 ? "1" : "2", "bigFont.fnt");
                        label->setPosition(colorIcon->getContentSize() / 2.0f);
                        label->setScale(0.5f);
                        colorIcon->addChild(label, 1);
                    }
                }
                m_player = colorIcon;
                break;
            }
            default: {
                if (type == UnlockType::GJItem && id > 5 && id < 16) m_player = GJPathSprite::create(id - 5);
                else m_player = IconCountEditor::createSprite(fmt::format("{}_{:02}_001.png", prefix, id), fmt::format("{}_01_001.png", prefix));
            }
        }

        m_player->setPosition({ 15.0f, 15.0f });
        addChild(m_player);

        return true;
    }
};
