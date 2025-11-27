#include "../IconCountEditor.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/MenuGameLayer.hpp>
#include <jasmine/random.hpp>

using namespace geode::prelude;

class $modify(ICEMenuGameLayer, MenuGameLayer) {
    static void onModify(ModifyBase<ModifyDerive<ICEMenuGameLayer, MenuGameLayer>>& self) {
        IconCountEditor::modify(self.m_hooks, "MenuGameLayer::resetPlayer", {
            IconType::Cube, IconType::Ship, IconType::Ball, IconType::Ufo,
            IconType::Wave, IconType::Robot, IconType::Spider, IconType::Swing
        });
    }

    int randomIcon(IconType type) {
        return jasmine::random::get(0.0, IconCountEditor::getCount(type));
    }

    void resetPlayer() {
        m_playerObject->deactivateStreak(true);
        m_playerObject->deactivateParticle();
        m_playerObject->setPosition({ (float)jasmine::random::get(-100.0, -600.0), m_playerObject->getPosition().y });
        m_playerObject->resetAllParticles();
        m_playerObject->togglePlayerScale(false, false);
        m_playerObject->m_hasGlow = jasmine::random::get() > 0.8;
        auto gm = GameManager::get();
        m_playerObject->setColor(gm->colorForIdx(jasmine::random::get(0.0, 108.0)));
        m_playerObject->setSecondColor(gm->colorForIdx(jasmine::random::get(0.0, 108.0)));
        m_playerObject->flipGravity(false, false);
        m_playerObject->update(0.0f);
        if (!m_videoOptionsOpen) {
            auto typeRand = jasmine::random::get();
            if (typeRand < 0.12 && !m_playerObject->m_isShip) {
                m_playerObject->toggleFlyMode(true, false);
                m_playerObject->updatePlayerShipFrame(randomIcon(IconType::Ship));
                m_playerObject->updatePlayerFrame(randomIcon(IconType::Cube));
            }
            else if (typeRand < 0.24 && !m_playerObject->m_isBall) {
                m_playerObject->releaseButton(PlayerButton::Jump);
                m_playerObject->toggleRollMode(true, false);
                m_playerObject->updatePlayerRollFrame(randomIcon(IconType::Ball));
            }
            else if (typeRand < 0.36 && !m_playerObject->m_isBird) {
                m_playerObject->toggleBirdMode(true, false);
                m_playerObject->updatePlayerBirdFrame(randomIcon(IconType::Ufo));
                m_playerObject->updatePlayerFrame(randomIcon(IconType::Cube));
            }
            else if (typeRand < 0.48 && !m_playerObject->m_isDart) {
                m_playerObject->toggleDartMode(true, false);
                m_playerObject->updatePlayerDartFrame(randomIcon(IconType::Wave));
            }
            else if (typeRand < 0.6 && !m_playerObject->m_isRobot) {
                m_playerObject->toggleRobotMode(true, false);
                auto id = randomIcon(IconType::Robot);
                #ifdef GEODE_IS_WINDOWS
                m_playerObject->createRobot(IconCountEditor::clamp(id, IconType::Robot));
                #else
                m_playerObject->updatePlayerRobotFrame(id);
                #endif
            }
            else if (typeRand < 0.7 && !m_playerObject->m_isSpider) {
                m_playerObject->releaseButton(PlayerButton::Jump);
                m_playerObject->toggleSpiderMode(true, false);
                auto id = randomIcon(IconType::Spider);
                #ifdef GEODE_IS_WINDOWS
                m_playerObject->createSpider(IconCountEditor::clamp(id, IconType::Spider));
                #else
                m_playerObject->updatePlayerSpiderFrame(id);
                #endif
            }
            else if (typeRand < 0.8 && !m_playerObject->m_isSwing) {
                m_playerObject->releaseButton(PlayerButton::Jump);
                m_playerObject->toggleSwingMode(true, false);
                m_playerObject->updatePlayerSwingFrame(randomIcon(IconType::Swing));
            }
            else {
                m_playerObject->switchedToMode(GameObjectType::CubePortal);
                m_playerObject->updatePlayerFrame(randomIcon(IconType::Cube));
            }
            m_playerObject->togglePlayerScale(jasmine::random::get() <= 0.1, false);
        }
        m_playerObject->updateGlowColor();
        auto speedRand = jasmine::random::get();
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
