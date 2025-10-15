#include "../IconCountEditor.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(ICEGameManager, GameManager) {
    static void onModify(ModifyBase<ModifyDerive<ICEGameManager, GameManager>>& self) {
        if (auto found = self.m_hooks.find("GameManager::countForType"); found != self.m_hooks.end()) {
            IconCountEditor::configureHook(found->second.get(), {
                IconType::Cube, IconType::Ship, IconType::Ball, IconType::Ufo,
                IconType::Wave, IconType::Robot, IconType::Spider, IconType::Swing,
                IconType::Jetpack, IconType::DeathEffect, IconType::Special, IconType::ShipFire
            });
        }
        #ifdef GEODE_IS_WINDOWS
        if (auto found = self.m_hooks.find("GameManager::init"); found != self.m_hooks.end()) {
            IconCountEditor::configureHook(found->second.get(), {
                IconType::Cube, IconType::Ship, IconType::Ball,
                IconType::Ufo, IconType::Wave, IconType::Robot,
                IconType::Spider, IconType::Swing, IconType::Jetpack
            });
        }
        #else
        if (auto found = self.m_hooks.find("GameManager::calculateBaseKeyForIcons"); found != self.m_hooks.end()) {
            IconCountEditor::configureHook(found->second.get(), {
                IconType::Cube, IconType::Ship, IconType::Ball,
                IconType::Ufo, IconType::Wave, IconType::Robot,
                IconType::Spider, IconType::Swing, IconType::Jetpack
            });
        }
        if (auto found = self.m_hooks.find("GameManager::loadDeathEffect"); found != self.m_hooks.end()) {
            IconCountEditor::configureHook(found->second.get(), { IconType::DeathEffect });
        }
        #endif
    }

    int countForType(IconType type) {
        return IconCountEditor::getCount(type);
    }

    #ifdef GEODE_IS_WINDOWS // Inlined into GameManager::init on Windows
    bool init() {
        m_fileName = "CCGameManager.dat";
        m_unkBool2 = false;
        m_vsyncEnabled = false;
        m_adTimer = 0.0;
        m_unkDouble2 = 0.0;
        m_googlePlaySignedIn = false;
        m_unkArray = CCArray::create();
        m_unkArray->retain();
        setup();
        m_keyStartForIcon.resize(9);
        auto count = 0;
        for (int i = 0; i < 9; i++) {
            m_keyStartForIcon[i] = count;
            count += IconCountEditor::getCount((IconType)i);
        }
        for (int i = 0; i < count; i++) {
            m_iconLoadCounts[i] = 0;
        }
        setupGameAnimations();
        m_chk = IconCountEditor::random() * 1000000.0 + floor(m_playerUserID.value() / 10000.0);
        return true;
    }
    #else
    void calculateBaseKeyForIcons() {
        m_keyStartForIcon.resize(9);
        auto count = 0;
        for (int i = 0; i < 9; i++) {
            m_keyStartForIcon[i] = count;
            count += IconCountEditor::getCount((IconType)i);
        }
        for (int i = 0; i < count; i++) {
            m_iconLoadCounts[i] = 0;
        }
    }

    void loadDeathEffect(int id) {
        id = std::clamp(id, 1, IconCountEditor::getCount(IconType::DeathEffect));
        if (m_loadedDeathEffect == id) return;

        auto pngPath = fmt::format("PlayerExplosion_{:02}.png", id - 1);
        auto plistPath = fmt::format("PlayerExplosion_{:02}.plist", id - 1);

        auto fileUtils = CCFileUtils::get();
        if (!fileUtils->isFileExist(pngPath) || !fileUtils->isFileExist(plistPath)) id = 1;

        auto textureCache = CCTextureCache::get();
        if (m_loadedDeathEffect > 1) {
            textureCache->removeTextureForKey(fmt::format("PlayerExplosion_{:02}.png", m_loadedDeathEffect - 1).c_str());
        }
        if (id > 1) {
            CCSpriteFrameCache::get()->addSpriteFramesWithFile(plistPath.c_str(), textureCache->addImage(pngPath.c_str(), false));
        }
        m_loadedDeathEffect = id;
    }
    #endif
};
