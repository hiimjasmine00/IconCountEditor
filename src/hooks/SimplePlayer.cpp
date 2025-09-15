#include "../IconCountEditor.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/modify/SimplePlayer.hpp>

using namespace geode::prelude;

class $modify(ICESimplePlayer, SimplePlayer) {
    static void onModify(ModifyBase<ModifyDerive<ICESimplePlayer, SimplePlayer>>& self) {
        auto& counts = IconCountEditor::getCounts();
        if (auto found = self.m_hooks.find("SimplePlayer::init"); found != self.m_hooks.end()) {
            auto& hook = found->second;
            hook->setAutoEnable(counts[IconType::Cube].second);
            hook->setPriority(Priority::Replace);
        }
        if (auto found = self.m_hooks.find("SimplePlayer::updatePlayerFrame"); found != self.m_hooks.end()) {
            auto& hook = found->second;
            hook->setAutoEnable(counts[IconType::Cube].second ||
                                counts[IconType::Ship].second ||
                                counts[IconType::Ball].second ||
                                counts[IconType::Ufo].second ||
                                counts[IconType::Wave].second ||
                                counts[IconType::Robot].second ||
                                counts[IconType::Spider].second ||
                                counts[IconType::Swing].second ||
                                counts[IconType::Jetpack].second);
            hook->setPriority(Priority::Replace);
        }
    }

    bool init(int id) {
        auto gm = GameManager::get();
        m_iconRequestID = gm->getIconRequestID();

        id = std::clamp(id, 1, IconCountEditor::getCount(IconType::Cube));

        if (!CCSprite::init()) return false;

        setTextureRect({ 0.0f, 0.0f, 0.0f, 0.0f });

        gm->loadIcon(id, 0, m_iconRequestID);

        m_firstLayer = IconCountEditor::createSprite(fmt::format("player_{:02}_001.png", id), "player_01_001.png");
        addChild(m_firstLayer, 1);

        m_secondLayer = IconCountEditor::createSprite(fmt::format("player_{:02}_2_001.png", id), "player_01_2_001.png");
        m_secondLayer->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_firstLayer->addChild(m_secondLayer, -1);

        m_birdDome = IconCountEditor::createSprite(fmt::format("player_{:02}_2_001.png", id), "player_01_2_001.png");
        m_birdDome->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_firstLayer->addChild(m_birdDome, -2);

        m_outlineSprite = IconCountEditor::createSprite(fmt::format("player_{:02}_glow_001.png", id), "player_01_glow_001.png");
        m_outlineSprite->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_firstLayer->addChild(m_outlineSprite, -3);

        m_detailSprite = IconCountEditor::createSprite(fmt::format("player_{:02}_2_001.png", id), "player_01_2_001.png");
        m_detailSprite->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_firstLayer->addChild(m_detailSprite, 1);

        SimplePlayer::updatePlayerFrame(id, IconType::Cube);

        return true;
    }

    void updatePlayerFrame(int id, IconType type) {
        constexpr std::array prefixes = {
            "player_", "ship_", "player_ball_", "bird_", "dart_", "robot_", "spider_", "swing_", "jetpack_"
        };

        if (m_robotSprite) m_robotSprite->setVisible(type == IconType::Robot);
        if (m_spiderSprite) m_spiderSprite->setVisible(type == IconType::Spider);

        auto notRobot = type != IconType::Robot && type != IconType::Spider;
        m_firstLayer->setVisible(notRobot);
        m_secondLayer->setVisible(notRobot);
        m_birdDome->setVisible(notRobot);

        id = std::clamp(id, 1, IconCountEditor::getCount(type));

        auto gm = GameManager::get();
        gm->loadIcon(id, (int)type, m_iconRequestID);

        if (type == IconType::Robot) {
            if (m_robotSprite) m_robotSprite->updateFrame(id);
            else createRobotSprite(id);
            return;
        }
        if (type == IconType::Spider) {
            if (m_spiderSprite) m_spiderSprite->updateFrame(id);
            else createSpiderSprite(id);
            return;
        }

        auto prefix = prefixes[(int)type];
        setFrames(
            fmt::format("{}{:02}_001.png", prefix, id).c_str(),
            fmt::format("{}{:02}_2_001.png", prefix, id).c_str(),
            type == IconType::Ufo ? fmt::format("{}{:02}_3_001.png", prefix, id).c_str() : nullptr,
            fmt::format("{}{:02}_glow_001.png", prefix, id).c_str(),
            fmt::format("{}{:02}_extra_001.png", prefix, id).c_str()
        );

        m_firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
        m_firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });
    }
};
