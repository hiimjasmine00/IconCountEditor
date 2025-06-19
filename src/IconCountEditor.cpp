#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/GJActionManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/PlayerFireBoostSprite.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GJRobotSprite.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/SimplePlayer.hpp>
#include <IconCountEditor.hpp>

using namespace geode::prelude;

std::map<IconType, int> IconCountEditor::iconCounts = {};

#define ICE_MODIFY \
    static void onModify(auto& self) { \
        for (auto& [name, hook] : self.m_hooks) { \
            hook->setPriority(999999999); \
        } \
    } \

class $modify(ICEGameManager, GameManager) {
    ICE_MODIFY

    #ifndef GEODE_IS_ANDROID64 // Crashes on 64-bit Android due to patch overflow
    int countForType(IconType type) {
        return IconCountEditor::getCount(type);
    }
    #endif

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
        for (int i = 0; i < 9; i++) {
            m_keyStartForIcon[i] = count;
            count += IconCountEditor::getCount((IconType)i);
        }
        for (int i = 0; i < count; i++) {
            m_iconLoadCounts[i] = 0;
        }
        setupGameAnimations();
        m_chk = ((float)rand() / (float)RAND_MAX) * 1000000.0f + floorf(m_playerUserID.value() / 10000.0f);
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
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::DeathEffect)), 1);
        if (m_loadedDeathEffect == id) return;

        auto textureCache = CCTextureCache::get();
        if (m_loadedDeathEffect > 1) {
            textureCache->removeTextureForKey(fmt::format("PlayerExplosion_{:02}.png", m_loadedDeathEffect - 1).c_str());
        }
        if (id > 1) {
            textureCache->addImage(fmt::format("PlayerExplosion_{:02}.png", id).c_str(), false);
            CCSpriteFrameCache::get()->addSpriteFramesWithFile(fmt::format("PlayerExplosion_{:02}.plist", id).c_str());
        }
        m_loadedDeathEffect = id;
    }
    #endif
};

class $modify(ICERobotSprite, GJRobotSprite) {
    ICE_MODIFY

    void updateFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(m_iconType)), 1);

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

            if (auto glowSprite = static_cast<CCSprite*>(m_glowSprite->getChildren()->objectAtIndex(i))) {
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

class $modify(ICEPlayerObject, PlayerObject) {
    ICE_MODIFY

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, cocos2d::CCLayer* layer, bool playLayer) {
        player = std::max(std::min(player, IconCountEditor::getCount(IconType::Cube)), 1);
        ship = std::max(std::min(ship, IconCountEditor::getCount(IconType::Ship)), 1);

        auto gm = GameManager::get();
        m_iconRequestID = gm->getIconRequestID();
        gm->loadIcon(player, 0, m_iconRequestID);
        gm->loadIcon(ship, 1, m_iconRequestID);

        if (!GameObject::init(fmt::format("player_{:02}_001.png", player).c_str())) return false;

        m_bUnkBool2 = false;
        m_bDontDraw = true;
        m_mainLayer = CCNode::create();
        m_mainLayer->setContentSize({ 0.0f, 0.0f });
        addChild(m_mainLayer);

        m_currentRobotAnimation = "run";
        m_switchWaveTrailColor = gm->getGameVariable("0096");
        m_practiceDeathEffect = gm->getGameVariable("0100");
        m_gv0123 = gm->getGameVariable("0123");

        auto gsm = GameStatsManager::get();
        m_robotAnimation1Enabled = gsm->isItemEnabled(UnlockType::GJItem, 18);
        m_robotAnimation2Enabled = gsm->isItemEnabled(UnlockType::GJItem, 19);
        m_spiderAnimationEnabled = gsm->isItemEnabled(UnlockType::GJItem, 20);
        m_maybeSavedPlayerFrame = player;
        m_ghostType = GhostType::Disabled;
        m_playerSpeed = 0.9f;
        m_playerSpeedAC = ((float)rand() / (float)RAND_MAX) * 10.0f + 5.0f;
        m_gameLayer = gameLayer;
        m_parentLayer = layer;
        m_playEffects = playLayer;
        m_maybeCanRunIntoBlocks = !playLayer;
        m_touchingRings = CCArray::create();
        m_touchingRings->retain();

        m_iconSprite = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_001.png", player).c_str());
        m_mainLayer->addChild(m_iconSprite, 1);
        m_iconSpriteSecondary = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_2_001.png", player).c_str());
        m_iconSpriteSecondary->setPosition(m_iconSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_iconSprite->addChild(m_iconSpriteSecondary, -1);
        m_iconSpriteWhitener = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_2_001.png", player).c_str());
        m_iconSpriteWhitener->setPosition(m_iconSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_iconSprite->addChild(m_iconSpriteWhitener, 2);
        updatePlayerSpriteExtra(fmt::format("player_{:02}_extra_001.png", player));

        m_vehicleSprite = CCSprite::createWithSpriteFrameName(fmt::format("ship_{:02}_001.png", ship).c_str());
        m_mainLayer->addChild(m_vehicleSprite, 2);
        m_vehicleSpriteSecondary = CCSprite::createWithSpriteFrameName(fmt::format("ship_{:02}_2_001.png", ship).c_str());
        m_vehicleSpriteSecondary->setPosition(m_vehicleSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_vehicleSprite->addChild(m_vehicleSpriteSecondary, -1);
        m_birdVehicle = CCSprite::createWithSpriteFrameName(fmt::format("ship_{:02}_2_001.png", ship).c_str());
        m_birdVehicle->setPosition(m_vehicleSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_vehicleSprite->addChild(m_birdVehicle, -2);
        m_vehicleSpriteWhitener = CCSprite::createWithSpriteFrameName(fmt::format("ship_{:02}_2_001.png", ship).c_str());
        m_vehicleSpriteWhitener->setPosition(m_vehicleSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_vehicleSprite->addChild(m_vehicleSpriteWhitener, 1);
        updateShipSpriteExtra(fmt::format("ship_{:02}_extra_001.png", ship));

        createRobot(gm->m_playerRobot.value());
        m_robotFire = PlayerFireBoostSprite::create();
        m_robotFire->setVisible(false);
        m_mainLayer->addChild(m_robotFire, -1);
        createSpider(gm->m_playerSpider.value());

        m_swingFireMiddle = PlayerFireBoostSprite::create();
        m_swingFireMiddle->setRotation(90.0f);
        m_swingFireMiddle->setPosition({ -14.0f, 0.0f });
        m_swingFireMiddle->m_size = 0.96f;
        m_mainLayer->addChild(m_swingFireMiddle, -1);
    
        m_swingFireBottom = PlayerFireBoostSprite::create();
        m_swingFireBottom->setRotation(45.0f);
        m_swingFireBottom->setPosition({ -9.5f, -10.0f });
        m_swingFireBottom->m_size = 0.72f;
        m_mainLayer->addChild(m_swingFireBottom, -1);
    
        m_swingFireTop = PlayerFireBoostSprite::create();
        m_swingFireTop->setRotation(135.0f);
        m_swingFireTop->setPosition({ -9.5f, 10.0f });
        m_swingFireTop->m_size = 0.72f;
        m_mainLayer->addChild(m_swingFireTop, -1);

        setYVelocity(0.0, 51);
        m_maybeIsBoosted = false;
        m_isDead = false;
        m_isOnGround = false;
        m_isOnGround2 = false;
        m_vehicleSize = 1.0f;
        m_unkAngle1 = 30.0f;
        updateTimeMod(0.9f, false);

        m_pendingCheckpoint = nullptr;
        m_maybeLastGroundObject = CCNode::create();
        m_collisionLogTop = CCDictionary::create();
        m_collisionLogTop->retain();
        m_collisionLogBottom = CCDictionary::create();
        m_collisionLogBottom->retain();
        m_collisionLogLeft = CCDictionary::create();
        m_collisionLogLeft->retain();
        m_collisionLogRight = CCDictionary::create();
        m_collisionLogRight->retain();
        m_particleSystems = CCArray::create();
        m_particleSystems->retain();
        m_actionManager = GJActionManager::create();
        m_actionManager->retain();
        m_playerGroundParticles = CCParticleSystemQuad::create("dragEffect.plist", false);
        m_playerGroundParticles->setPositionType(kCCPositionTypeRelative);
        m_playerGroundParticles->stopSystem();
        m_particleSystems->addObject(m_playerGroundParticles);
        m_hasGroundParticles = false;
        m_dashParticles = CCParticleSystemQuad::create("dashEffect.plist", false);
        m_dashParticles->setPositionType(kCCPositionTypeRelative);
        m_dashParticles->stopSystem();
        m_particleSystems->addObject(m_dashParticles);
        m_ufoClickParticles = CCParticleSystemQuad::create("burstEffect.plist", false);
        m_ufoClickParticles->setPositionType(kCCPositionTypeRelative);
        m_ufoClickParticles->stopSystem();
        m_particleSystems->addObject(m_ufoClickParticles);
        m_robotBurstParticles = CCParticleSystemQuad::create("burstEffect2.plist", false);
        m_robotBurstParticles->setPositionType(kCCPositionTypeRelative);
        m_robotBurstParticles->stopSystem();
        m_particleSystems->addObject(m_robotBurstParticles);
        m_trailingParticles = CCParticleSystemQuad::create("dragEffect.plist", false);
        m_trailingParticles->setPosVar({ 0.0f, 2.0f });
        m_trailingParticles->setSpeed(m_trailingParticles->getSpeed() * 0.2f);
        m_trailingParticles->setSpeedVar(m_trailingParticles->getSpeedVar() * 0.2f);
        m_trailingParticles->setStartColor({ 1.0f, 100.0f / 255.0f, 0.0f, 1.0f });
        m_trailingParticles->setEndColor({ 1.0f, 0.0f, 0.0f, 1.0f });
        m_trailingParticles->setPositionType(kCCPositionTypeRelative);
        m_trailingParticles->stopSystem();
        m_particleSystems->addObject(m_trailingParticles);
        m_trailingParticleLife = m_trailingParticles->getLife();
        m_shipClickParticles = CCParticleSystemQuad::create("dragEffect.plist", false);
        m_shipClickParticles->setPosVar({ 0.0f, 2.0f });
        m_shipClickParticles->setAngleVar(m_shipClickParticles->getAngleVar() * 2.0f);
        m_shipClickParticles->setSpeed(m_shipClickParticles->getSpeed() * 2.0f);
        m_shipClickParticles->setSpeedVar(m_shipClickParticles->getSpeedVar() * 2.0f);
        m_shipClickParticles->setStartSize(m_shipClickParticles->getStartSize() * 1.5f);
        m_shipClickParticles->setStartSizeVar(m_shipClickParticles->getStartSizeVar() * 1.5f);
        m_shipClickParticles->setStartColor({ 1.0f, 190.0f / 255.0f, 0.0f, 1.0f });
        m_shipClickParticles->setEndColor({ 1.0f, 0.0f, 0.0f, 1.0f });
        m_shipClickParticles->setPositionType(kCCPositionTypeRelative);
        m_shipClickParticles->stopSystem();
        m_particleSystems->addObject(m_shipClickParticles);
        m_hasShipParticles = false;
        m_swingBurstParticles1 = CCParticleSystemQuad::create("swingBurstEffect.plist", false);
        m_swingBurstParticles1->setStartColor({ 1.0f, 100.0f / 255.0f, 0.0f, 1.0f });
        m_swingBurstParticles1->setEndColor({ 1.0f, 0.0f, 0.0f, 1.0f });
        m_swingBurstParticles1->setPositionType(kCCPositionTypeGrouped);
        m_swingBurstParticles1->stopSystem();
        m_particleSystems->addObject(m_swingBurstParticles1);
        m_swingBurstParticles2 = CCParticleSystemQuad::create("swingBurstEffect.plist", false);
        m_swingBurstParticles2->setStartColor({ 1.0f, 100.0f / 255.0f, 0.0f, 1.0f });
        m_swingBurstParticles2->setEndColor({ 1.0f, 0.0f, 0.0f, 1.0f });
        m_swingBurstParticles2->setPositionType(kCCPositionTypeGrouped);
        m_swingBurstParticles2->stopSystem();
        m_particleSystems->addObject(m_swingBurstParticles2);
        m_vehicleGroundParticles = CCParticleSystemQuad::create("shipDragEffect.plist", false);
        m_vehicleGroundParticles->setPositionType(kCCPositionTypeGrouped);
        m_vehicleGroundParticles->stopSystem();
        m_particleSystems->addObject(m_vehicleGroundParticles);
        m_landParticles0 = CCParticleSystemQuad::create("landEffect.plist", false);
        m_landParticles0->setPositionType(kCCPositionTypeGrouped);
        m_landParticles0->stopSystem();
        m_particleSystems->addObject(m_landParticles0);
        m_landParticlesAngle = m_landParticles0->getAngle();
        m_landParticleRelatedY = m_landParticles0->getGravity().y;
        m_landParticles1 = CCParticleSystemQuad::create("landEffect.plist", false);
        m_landParticles1->setPositionType(kCCPositionTypeGrouped);
        m_landParticles1->stopSystem();
        m_particleSystems->addObject(m_landParticles1);

        setupStreak();

        m_dashSpritesContainer = CCSprite::create();
        m_dashSpritesContainer->setTextureRect({ 0.0f, 0.0f, 0.0f, 0.0f });
        m_dashSpritesContainer->setDontDraw(true);
        m_mainLayer->addChild(m_dashSpritesContainer, -1);
        m_dashFireSprite = CCSprite::createWithSpriteFrameName("playerDash2_001.png");
        m_dashFireSprite->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        m_dashFireSprite->setPosition(m_dashSpritesContainer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_dashFireSprite->setVisible(false);
        m_dashSpritesContainer->addChild(m_dashFireSprite);
        auto dashOutlineSprite = CCSprite::createWithSpriteFrameName("playerDash2_outline_001.png");
        dashOutlineSprite->setPosition(m_dashFireSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        dashOutlineSprite->setOpacity(150);
        m_dashFireSprite->addChild(dashOutlineSprite, 1);

        m_iconGlow = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_glow_001.png", player).c_str());
        m_iconGlow->setVisible(false);
        m_dashSpritesContainer->addChild(m_iconGlow, 2);
        m_vehicleGlow = CCSprite::createWithSpriteFrameName(fmt::format("ship_{:02}_glow_001.png", ship).c_str());
        m_vehicleGlow->setVisible(false);
        m_dashSpritesContainer->addChild(m_vehicleGlow, -3);

        m_width = 30.0f;
        m_height = 30.0f;
        m_unkAngle1 = 30.0f;
        m_defaultMiniIcon = gm->getGameVariable("0060");
        m_swapColors = gm->getGameVariable("0061");
        m_switchDashFireColor = gm->getGameVariable("0062");
        #ifndef GEODE_IS_ANDROID // I simply cannot believe this
        m_playerFollowFloats.resize(200, 0.0f);
        #else
        auto oldSize = m_playerFollowFloats.size();
        m_playerFollowFloats.resize(200);
        if (oldSize < 200) std::fill(m_playerFollowFloats.begin() + oldSize, m_playerFollowFloats.end(), 0.0f);
        #endif

        updateCheckpointMode(false);

        return true;
    }

    void updatePlayerFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Cube)), 1);
        if (id > 0) m_maybeSavedPlayerFrame = id;

        auto gm = GameManager::get();
        gm->loadIcon(id, 0, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("player_{:02}_extra_001.png", id));
    }

    void updatePlayerShipFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Ship)), 1);

        auto gm = GameManager::get();
        gm->loadIcon(id, 1, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_vehicleSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("ship_{:02}_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("ship_{:02}_2_001.png", id).c_str()));
        m_vehicleGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("ship_{:02}_glow_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setPosition(m_vehicleSprite->getContentSize() / 2.0f);
        updateShipSpriteExtra(fmt::format("ship_{:02}_extra_001.png", id));
    }

    void updatePlayerRollFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Ball)), 1);

        auto gm = GameManager::get();
        gm->loadIcon(id, 2, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_ball_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_ball_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_ball_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("player_ball_{:02}_extra_001.png", id));
    }

    void updatePlayerBirdFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Ufo)), 1);

        auto gm = GameManager::get();
        gm->loadIcon(id, 3, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_vehicleSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("bird_{:02}_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("bird_{:02}_2_001.png", id).c_str()));
        m_birdVehicle->setDisplayFrame(sfc->spriteFrameByName(fmt::format("bird_{:02}_3_001.png", id).c_str()));
        m_vehicleGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("bird_{:02}_glow_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setPosition(m_vehicleSprite->getContentSize() / 2.0f);
        m_birdVehicle->setPosition(m_vehicleSpriteSecondary->getPosition());
        updateShipSpriteExtra(fmt::format("bird_{:02}_extra_001.png", id));
    }

    void updatePlayerDartFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Wave)), 1);

        auto gm = GameManager::get();
        gm->loadIcon(id, 4, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("dart_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("dart_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("dart_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("dart_{:02}_extra_001.png", id));
    }

    #ifndef GEODE_IS_WINDOWS // Inlined into MenuGameLayer::resetPlayer on Windows
    void updatePlayerRobotFrame(int id) {
        createRobot(std::max(std::min(id, IconCountEditor::getCount(IconType::Robot)), 1));
    }

    void updatePlayerSpiderFrame(int id) {
        createSpider(std::max(std::min(id, IconCountEditor::getCount(IconType::Spider)), 1));
    }
    #endif

    void updatePlayerSwingFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Swing)), 1);

        auto gm = GameManager::get();
        gm->loadIcon(id, 6, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("swing_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("swing_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("swing_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("swing_{:02}_extra_001.png", id));
    }

    void updatePlayerJetpackFrame(int id) {
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Jetpack)), 1);

        auto gm = GameManager::get();
        gm->loadIcon(id, 7, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_vehicleSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("jetpack_{:02}_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("jetpack_{:02}_2_001.png", id).c_str()));
        m_vehicleGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("jetpack_{:02}_glow_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setPosition(m_vehicleSprite->getContentSize() / 2.0f);
        updateShipSpriteExtra(fmt::format("jetpack_{:02}_extra_001.png", id));
    }
};

class $modify(ICESimplePlayer, SimplePlayer) {
    ICE_MODIFY
    
    bool init(int id) {
        auto gm = GameManager::get();
        m_iconRequestID = gm->getIconRequestID();
    
        id = std::max(std::min(id, IconCountEditor::getCount(IconType::Cube)), 1);
    
        if (!CCSprite::init()) return false;

        setTextureRect({ 0.0f, 0.0f, 0.0f, 0.0f });

        gm->loadIcon(id, 0, m_iconRequestID);

        m_firstLayer = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_001.png", id).c_str());
        addChild(m_firstLayer, 1);

        m_secondLayer = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_2_001.png", id).c_str());
        m_firstLayer->addChild(m_secondLayer, -1);

        m_secondLayer->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_birdDome = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_2_001.png", id).c_str());
        m_firstLayer->addChild(m_birdDome, -2);

        m_birdDome->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_outlineSprite = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_glow_001.png", id).c_str());
        m_firstLayer->addChild(m_outlineSprite, -3);

        m_outlineSprite->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));
        m_detailSprite = CCSprite::createWithSpriteFrameName(fmt::format("player_{:02}_2_001.png", id).c_str());

        m_firstLayer->addChild(m_detailSprite, 1);
        m_detailSprite->setPosition(m_firstLayer->convertToNodeSpace({ 0.0f, 0.0f }));

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

        id = std::max(std::min(id, IconCountEditor::getCount(type)), 1);

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

#if defined(GEODE_IS_WINDOWS)
#define PATCH(offset) \
    auto count = it->second; \
    auto b1 = (uint8_t)((count >> 24) & 0xff); \
    auto b2 = (uint8_t)((count >> 16) & 0xff); \
    auto b3 = (uint8_t)((count >> 8) & 0xff); \
    auto b4 = (uint8_t)(count & 0xff); \
    if (auto res = mod->patch(reinterpret_cast<void*>(dash + offset), { b1, b2, b3, b4 }); res.isErr()) { \
        failCount++; \
        log::error("Failed to patch GeometryDash.exe + " #offset ": {}", res.unwrapErr()); \
    } \

Result<void, void> IconCountEditor::patch() {
    auto dash = base::get();
    auto failCount = 0;
    auto mod = Mod::get();
    if (auto it = iconCounts.find(IconType::Robot); it != iconCounts.end()) {
        PATCH(0x31dfa1);
    }
    if (auto it = iconCounts.find(IconType::Spider); it != iconCounts.end()) {
        PATCH(0x31e0cf);
    }
    if (failCount > 0) return Err();
    else return Ok();
}
#elif defined(GEODE_IS_ANDROID64)
#define PATCH_MOV(name, symbol, offset) \
    auto GEODE_CONCAT(sym, __LINE__) = reinterpret_cast<uintptr_t>(dlsym(dash, symbol)) + offset; \
    auto GEODE_CONCAT(value, __LINE__) = *reinterpret_cast<uint32_t*>(GEODE_CONCAT(sym, __LINE__)) & 0xffe0001f; \
    GEODE_CONCAT(value, __LINE__) |= (it->second & 0xffff) << 5; \
    if (auto res = mod->patch(reinterpret_cast<void*>(GEODE_CONCAT(sym, __LINE__)), { \
        (uint8_t)((GEODE_CONCAT(value, __LINE__) >> 24) & 0xff), \
        (uint8_t)((GEODE_CONCAT(value, __LINE__) >> 16) & 0xff), \
        (uint8_t)((GEODE_CONCAT(value, __LINE__) >> 8) & 0xff), \
        (uint8_t)(GEODE_CONCAT(value, __LINE__) & 0xff) \
    }); res.isErr()) { \
        failCount++; \
        log::error("Failed to patch " name " + " #offset ": {}", res.unwrapErr()); \
    } \

Result<void, void> IconCountEditor::patch() {
    auto dash = dlopen("libcocos2dcpp.so", RTLD_LAZY | RTLD_NOLOAD);
    auto failCount = 0;
    auto mod = Mod::get();
    if (auto it = iconCounts.find(IconType::Cube); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x28);
    }
    if (auto it = iconCounts.find(IconType::Ship); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x30);
    }
    if (auto it = iconCounts.find(IconType::Ball); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x38);
    }
    if (auto it = iconCounts.find(IconType::Ufo); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x40);
    }
    if (auto it = iconCounts.find(IconType::Wave); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x48);
    }
    if (auto it = iconCounts.find(IconType::Robot); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x50);
    }
    if (auto it = iconCounts.find(IconType::Spider); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x58);
    }
    if (auto it = iconCounts.find(IconType::Swing); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x60);
    }
    if (auto it = iconCounts.find(IconType::Jetpack); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x68);
    }
    if (auto it = iconCounts.find(IconType::DeathEffect); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x70);
    }
    if (auto it = iconCounts.find(IconType::Special); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x78);
    }
    if (auto it = iconCounts.find(IconType::ShipFire); it != iconCounts.end()) {
        PATCH_MOV("GameManager::countForType(IconType)", "_ZN11GameManager12countForTypeE8IconType", 0x80);
    }
    if (failCount > 0) return Err();
    else return Ok();
}
#else
Result<void, void> IconCountEditor::patch() {
    return Ok();
}
#endif
