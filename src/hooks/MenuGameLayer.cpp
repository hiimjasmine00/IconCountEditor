#include "../IconCountEditor.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/MenuGameLayer.hpp>

using namespace geode::prelude;

class $modify(ICEMenuGameLayer, MenuGameLayer) {
    static void onModify(ModifyBase<ModifyDerive<ICEMenuGameLayer, MenuGameLayer>>& self) {
        if (auto found = self.m_hooks.find("MenuGameLayer::resetPlayer"); found != self.m_hooks.end()) {
            IconCountEditor::configureHook(found->second.get(), {
                IconType::Cube, IconType::Ship, IconType::Ball, IconType::Ufo,
                IconType::Wave, IconType::Robot, IconType::Spider, IconType::Swing
            });
        }
    }

    void resetPlayer() {
        m_playerObject->deactivateStreak(true);
        m_playerObject->deactivateParticle();
        m_playerObject->setPosition({ (float)(IconCountEditor::random() * -500.0 - 100.0), m_playerObject->getPosition().y });
        m_playerObject->resetAllParticles();
        m_playerObject->togglePlayerScale(false, false);
        m_playerObject->m_hasGlow = IconCountEditor::random() > 0.8;
        auto gm = GameManager::get();
        m_playerObject->setColor(gm->colorForIdx(IconCountEditor::random() * 108.0));
        m_playerObject->setSecondColor(gm->colorForIdx(IconCountEditor::random() * 108.0));
        m_playerObject->flipGravity(false, false);
        m_playerObject->update(0.0f);
        if (!m_videoOptionsOpen) {
            auto typeRand = IconCountEditor::random();
            if (typeRand < 0.12 && !m_playerObject->m_isShip) {
                m_playerObject->toggleFlyMode(true, false);
                m_playerObject->updatePlayerShipFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Ship));
                m_playerObject->updatePlayerFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Cube));
            }
            else if (typeRand < 0.24 && !m_playerObject->m_isBall) {
                m_playerObject->releaseButton(PlayerButton::Jump);
                m_playerObject->toggleRollMode(true, false);
                m_playerObject->updatePlayerRollFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Ball));
            }
            else if (typeRand < 0.36 && !m_playerObject->m_isBird) {
                m_playerObject->toggleBirdMode(true, false);
                m_playerObject->updatePlayerBirdFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Ufo));
                m_playerObject->updatePlayerFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Cube));
            }
            else if (typeRand < 0.48 && !m_playerObject->m_isDart) {
                m_playerObject->toggleDartMode(true, false);
                m_playerObject->updatePlayerDartFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Wave));
            }
            else if (typeRand < 0.6 && !m_playerObject->m_isRobot) {
                m_playerObject->toggleRobotMode(true, false);
                auto count = IconCountEditor::getCount(IconType::Robot);
                #ifdef GEODE_IS_WINDOWS
                m_playerObject->createRobot(std::clamp<int>(IconCountEditor::random() * count, 1, count));
                #else
                m_playerObject->updatePlayerRobotFrame(IconCountEditor::random() * count);
                #endif
            }
            else if (typeRand < 0.7 && !m_playerObject->m_isSpider) {
                m_playerObject->releaseButton(PlayerButton::Jump);
                m_playerObject->toggleSpiderMode(true, false);
                auto count = IconCountEditor::getCount(IconType::Spider);
                #ifdef GEODE_IS_WINDOWS
                m_playerObject->createSpider(std::clamp<int>(IconCountEditor::random() * count, 1, count));
                #else
                m_playerObject->updatePlayerSpiderFrame(IconCountEditor::random() * count);
                #endif
            }
            else if (typeRand < 0.8 && !m_playerObject->m_isSwing) {
                m_playerObject->releaseButton(PlayerButton::Jump);
                m_playerObject->toggleSwingMode(true, false);
                m_playerObject->updatePlayerSwingFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Swing));
            }
            else {
                m_playerObject->switchedToMode(GameObjectType::CubePortal);
                m_playerObject->updatePlayerFrame(IconCountEditor::random() * IconCountEditor::getCount(IconType::Cube));
            }
            m_playerObject->togglePlayerScale(IconCountEditor::random() <= 0.1, false);
        }
        m_playerObject->updateGlowColor();
        auto speedRand = IconCountEditor::random();
        if (speedRand < 0.2) m_playerObject->updateTimeMod(1.3f, false);
        else if (speedRand < 0.4) m_playerObject->updateTimeMod(1.1f, false);
        else if (speedRand < 0.6) m_playerObject->updateTimeMod(0.7f, false);
        else if (speedRand < 0.65) m_playerObject->updateTimeMod(1.6f, false);
        else m_playerObject->updateTimeMod(0.9f, false);
        if (m_playerObject->m_isDart) {
            m_playerObject->resetStreak();
            m_playerObject->placeStreakPoint();
        }
        m_playerObject->updateEffects(0.0f);
    }
};
