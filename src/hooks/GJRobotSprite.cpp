#include "../IconCountEditor.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/modify/GJRobotSprite.hpp>

using namespace geode::prelude;

class $modify(ICERobotSprite, GJRobotSprite) {
    static void onModify(ModifyBase<ModifyDerive<ICERobotSprite, GJRobotSprite>>& self) {
        auto& counts = IconCountEditor::getCounts();
        if (auto found = self.m_hooks.find("GJRobotSprite::updateFrame"); found != self.m_hooks.end()) {
            auto& hook = found->second;
            hook->setAutoEnable(counts[IconType::Robot].second || counts[IconType::Spider].second);
            hook->setPriority(Priority::Replace);
        }
    }

    void updateFrame(int id) {
        id = std::clamp(id, 1, IconCountEditor::getCount(m_iconType));

        auto gm = GameManager::get();
        setTexture(gm->loadIcon(id, (int)m_iconType, m_iconRequestID));

        auto prefix = m_iconType == IconType::Robot ? "robot_" : "spider_";
        auto sfc = CCSpriteFrameCache::get();
        auto spriteParts = m_paSprite->m_spriteParts;
        for (int i = 0; i < spriteParts->count(); i++) {
            auto sprite = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
            auto tag = sprite->getTag();
            auto frame = fmt::format("{}{:02}_{:02}", prefix, id, tag);

            sprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_001.png", frame).c_str()));

            if (auto secondSprite = static_cast<CCSprite*>(m_secondArray->objectAtIndex(i))) {
                secondSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_2_001.png", frame).c_str()));
                secondSprite->setPosition(sprite->getContentSize() / 2.0f);
            }

            if (auto glowSprite = m_glowSprite->getChildByIndex<CCSprite>(i)) {
                glowSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_glow_001.png", frame).c_str()));
            }

            if (sprite != m_headSprite) continue;

            if (auto extraFrame = sfc->spriteFrameByName(fmt::format("{}_extra_001.png", frame).c_str())) {
                if (m_extraSprite) m_extraSprite->setDisplayFrame(extraFrame);
                else {
                    m_extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                    m_headSprite->addChild(m_extraSprite, 2);
                }
                m_extraSprite->setPosition(sprite->getContentSize() / 2.0f);
                m_hasExtra = true;
            }
            else m_hasExtra = false;
            m_extraSprite->setVisible(m_hasExtra);
        }
    }
};
