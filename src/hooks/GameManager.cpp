#include "../IconCountEditor.hpp"
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(ICEGameManager, GameManager) {
    static void onModify(ModifyBase<ModifyDerive<ICEGameManager, GameManager>>& self) {
        auto& counts = IconCountEditor::getCounts();
        if (auto found = self.m_hooks.find("GameManager::countForType"); found != self.m_hooks.end()) {
            auto& hook = found->second;
            hook->setAutoEnable(counts[IconType::Cube].second ||
                                counts[IconType::Ship].second ||
                                counts[IconType::Ball].second ||
                                counts[IconType::Ufo].second ||
                                counts[IconType::Wave].second ||
                                counts[IconType::Robot].second ||
                                counts[IconType::Spider].second ||
                                counts[IconType::Swing].second ||
                                counts[IconType::Jetpack].second ||
                                counts[IconType::DeathEffect].second ||
                                counts[IconType::Special].second ||
                                counts[IconType::ShipFire].second);
            hook->setPriority(Priority::Replace);
        }
        #ifdef GEODE_IS_WINDOWS
        if (auto found = self.m_hooks.find("GameManager::init"); found != self.m_hooks.end()) {
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
        #else
        if (auto found = self.m_hooks.find("GameManager::calculateBaseKeyForIcons"); found != self.m_hooks.end()) {
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
        if (auto found = self.m_hooks.find("GameManager::loadDeathEffect"); found != self.m_hooks.end()) {
            auto& hook = found->second;
            hook->setAutoEnable(counts[IconType::DeathEffect].second);
            hook->setPriority(Priority::Replace);
        }
        #endif
    }

    int countForType(IconType type) {
        return IconCountEditor::getCount(type);
    }

    #ifdef GEODE_IS_WINDOWS // Inlined into GameManager::init on Windows
    bool init() {
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
        auto& counts = IconCountEditor::getCounts();
        for (int i = 0; i < 9; i++) {
            m_keyStartForIcon[i] = count;
            count += counts[(IconType)i].first;
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
        auto& counts = IconCountEditor::getCounts();
        for (int i = 0; i < 9; i++) {
            m_keyStartForIcon[i] = count;
            count += counts[(IconType)i].first;
        }
        for (int i = 0; i < count; i++) {
            m_iconLoadCounts[i] = 0;
        }
    }

    void loadDeathEffect(int id) {
        id = std::clamp(id, 1, IconCountEditor::getCount(IconType::DeathEffect));
        if (m_loadedDeathEffect == id) return;

        auto pngPath = fmt::format("PlayerExplosion_{:02}.png", id);
        auto plistPath = fmt::format("PlayerExplosion_{:02}.plist", id);

        auto fileUtils = CCFileUtils::get();
        if (!fileUtils->isFileExist(pngPath) || !fileUtils->isFileExist(plistPath)) return;

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
