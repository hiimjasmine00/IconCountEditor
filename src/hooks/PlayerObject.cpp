#include "../IconCountEditor.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/GJActionManager.hpp>
#include <Geode/binding/HardStreak.hpp>
#include <Geode/binding/PlayerFireBoostSprite.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <jasmine/random.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_ANDROID
gd::string getFrameForStreak(ShipStreak streak, float dt);
void getSettingsForStreak(ShipStreak streak, float speed, float size, float& fade, float& stroke);
#else
#ifdef GEODE_IS_WINDOWS
void* wrapFunction(uintptr_t address, const tulip::hook::WrapperMetadata& metadata) {
    auto wrapped = hook::createWrapper(reinterpret_cast<void*>(address), metadata);
    if (wrapped.isErr()) {
        throw std::runtime_error(wrapped.unwrapErr());
    }
    return wrapped.unwrap();
}
#else
void* wrapFunction(uintptr_t address, const tulip::hook::WrapperMetadata& metadata);
#endif

std::string getFrameForStreak(ShipStreak streak, float dt) {
    using FunctionType = std::string(*)(ShipStreak, float);
    static auto func = wrapFunction(
        base::get() + GEODE_WINDOWS(0x3702f0) GEODE_ARM_MAC(0x36a004) GEODE_INTEL_MAC(0x3e7eb0) GEODE_IOS(0x217d98),
        {
            .m_convention = hook::createConvention(tulip::hook::TulipConvention::Default),
            .m_abstract = tulip::hook::AbstractFunction::from(FunctionType(nullptr))
        }
    );
    return reinterpret_cast<FunctionType>(func)(streak, dt);
}

void getSettingsForStreak(ShipStreak streak, float speed, float size, float& fade, float& stroke) {
    using FunctionType = void(*)(ShipStreak, float, float, float&, float&);
    static auto func = wrapFunction(
        base::get() + GEODE_WINDOWS(0x36fe00) GEODE_ARM_MAC(0x369910) GEODE_INTEL_MAC(0x3e7870) GEODE_IOS(0x217864),
        {
            .m_convention = hook::createConvention(tulip::hook::TulipConvention::Default),
            .m_abstract = tulip::hook::AbstractFunction::from(FunctionType(nullptr))
        }
    );
    return reinterpret_cast<FunctionType>(func)(streak, speed, size, fade, stroke);
}
#endif

class $modify(ICEPlayerObject, PlayerObject) {
    static void onModify(ModifyBase<ModifyDerive<ICEPlayerObject, PlayerObject>>& self) {
        IconCountEditor::modify(self.m_hooks, "PlayerObject::init", { IconType::Cube, IconType::Ship });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerFrame", { IconType::Cube });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerShipFrame", { IconType::Ship });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerRollFrame", { IconType::Ball });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerBirdFrame", { IconType::Ufo });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerDartFrame", { IconType::Wave });
        #ifndef GEODE_IS_WINDOWS
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerRobotFrame", { IconType::Robot });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerSpiderFrame", { IconType::Spider });
        #endif
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerSwingFrame", { IconType::Swing });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::updatePlayerJetpackFrame", { IconType::Jetpack });
        IconCountEditor::modify(self.m_hooks, "PlayerObject::setupStreak", { IconType::Special, IconType::ShipFire });
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool playLayer) {
        player = IconCountEditor::clamp(player, IconType::Cube);
        ship = IconCountEditor::clamp(ship, IconType::Ship);

        auto gm = GameManager::get();
        m_iconRequestID = gm->getIconRequestID();
        gm->loadIcon(player, 0, m_iconRequestID);
        gm->loadIcon(ship, 1, m_iconRequestID);

        auto frame = fmt::format("player_{:02}_001.png", player);
        auto spriteFrame = CCSpriteFrameCache::get()->spriteFrameByName(frame.c_str());
        if (!spriteFrame || spriteFrame->getTag() == 105871529) frame = "player_01_001.png";
        if (!GameObject::init(frame.c_str())) return false;

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
        m_playerSpeedAC = jasmine::random::get() * 10.0 + 5.0;
        m_gameLayer = gameLayer;
        m_parentLayer = layer;
        m_playEffects = playLayer;
        m_maybeCanRunIntoBlocks = !playLayer;
        m_touchingRings = CCArray::create();
        m_touchingRings->retain();
        setTextureRect({ 0.0f, 0.0f, 0.0f, 0.0f });

        m_iconSprite = IconCountEditor::createSprite("player", player);
        m_mainLayer->addChild(m_iconSprite, 1);
        m_iconSpriteSecondary = IconCountEditor::createSprite("player", player, "_2");
        m_iconSpriteSecondary->setPosition(m_iconSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_iconSprite->addChild(m_iconSpriteSecondary, -1);
        m_iconSpriteWhitener = IconCountEditor::createSprite("player", player, "_2");
        m_iconSpriteWhitener->setPosition(m_iconSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_iconSprite->addChild(m_iconSpriteWhitener, 2);
        updatePlayerSpriteExtra(fmt::format("player_{:02}_extra_001.png", player));

        m_vehicleSprite = IconCountEditor::createSprite("ship", ship);
        m_vehicleSprite->setVisible(false);
        m_mainLayer->addChild(m_vehicleSprite, 2);
        m_vehicleSpriteSecondary = IconCountEditor::createSprite("ship", ship, "_2");
        m_vehicleSpriteSecondary->setPosition(m_vehicleSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_vehicleSprite->addChild(m_vehicleSpriteSecondary, -1);
        m_birdVehicle = IconCountEditor::createSprite("ship", ship, "_2");
        m_birdVehicle->setPosition(m_vehicleSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_birdVehicle->setVisible(false);
        m_vehicleSprite->addChild(m_birdVehicle, -2);
        m_vehicleSpriteWhitener = IconCountEditor::createSprite("ship", ship, "_2");
        m_vehicleSpriteWhitener->setPosition(m_vehicleSprite->convertToNodeSpace({ 0.0f, 0.0f }));
        m_vehicleSprite->addChild(m_vehicleSpriteWhitener, 1);
        updateShipSpriteExtra(fmt::format("ship_{:02}_extra_001.png", ship));

        createRobot(gm->m_playerRobot.value());
        m_robotFire = PlayerFireBoostSprite::create();
        m_robotFire->setVisible(false);
        m_mainLayer->addChild(m_robotFire, -1);
        createSpider(gm->m_playerSpider.value());

        m_swingFireMiddle = PlayerFireBoostSprite::create();
        m_swingFireMiddle->setVisible(false);
        m_swingFireMiddle->setRotation(90.0f);
        m_swingFireMiddle->setPosition({ -14.0f, 0.0f });
        m_swingFireMiddle->m_size = 0.96f;
        m_mainLayer->addChild(m_swingFireMiddle, -1);

        m_swingFireBottom = PlayerFireBoostSprite::create();
        m_swingFireBottom->setVisible(false);
        m_swingFireBottom->setRotation(45.0f);
        m_swingFireBottom->setPosition({ -9.5f, -10.0f });
        m_swingFireBottom->m_size = 0.72f;
        m_mainLayer->addChild(m_swingFireBottom, -1);

        m_swingFireTop = PlayerFireBoostSprite::create();
        m_swingFireTop->setVisible(false);
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

        PlayerObject::setupStreak();

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

        m_iconGlow = IconCountEditor::createSprite("player", player, "_glow");
        m_iconGlow->setVisible(false);
        m_dashSpritesContainer->addChild(m_iconGlow, 2);
        m_vehicleGlow = IconCountEditor::createSprite("ship", ship, "_glow");
        m_vehicleGlow->setVisible(false);
        m_dashSpritesContainer->addChild(m_vehicleGlow, -3);

        m_width = 30.0f;
        m_height = 30.0f;
        m_unkAngle1 = 30.0f;
        m_defaultMiniIcon = gm->getGameVariable("0060");
        m_swapColors = gm->getGameVariable("0061");
        m_switchDashFireColor = gm->getGameVariable("0062");
        m_playerFollowFloats.resize(200, 0.0f);

        updateCheckpointMode(false);

        return true;
    }

    void updatePlayerFrame(int id) {
        id = IconCountEditor::clamp(id, IconType::Cube);
        if (id > 0) m_maybeSavedPlayerFrame = id;

        GameManager::get()->loadIcon(id, 0, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("player_{:02}_extra_001.png", id));
    }

    void updatePlayerShipFrame(int id) {
        id = IconCountEditor::clamp(id, IconType::Ship);

        GameManager::get()->loadIcon(id, 1, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_vehicleSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("ship_{:02}_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("ship_{:02}_2_001.png", id).c_str()));
        m_vehicleGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("ship_{:02}_glow_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setPosition(m_vehicleSprite->getContentSize() / 2.0f);
        updateShipSpriteExtra(fmt::format("ship_{:02}_extra_001.png", id));
    }

    void updatePlayerRollFrame(int id) {
        id = IconCountEditor::clamp(id, IconType::Ball);

        GameManager::get()->loadIcon(id, 2, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_ball_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_ball_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("player_ball_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("player_ball_{:02}_extra_001.png", id));
    }

    void updatePlayerBirdFrame(int id) {
        id = IconCountEditor::clamp(id, IconType::Ufo);

        GameManager::get()->loadIcon(id, 3, m_iconRequestID);

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
        id = IconCountEditor::clamp(id, IconType::Wave);

        GameManager::get()->loadIcon(id, 4, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("dart_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("dart_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("dart_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("dart_{:02}_extra_001.png", id));
    }

    #ifndef GEODE_IS_WINDOWS // Inlined into MenuGameLayer::resetPlayer on Windows
    void updatePlayerRobotFrame(int id) {
        createRobot(IconCountEditor::clamp(id, IconType::Robot));
    }

    void updatePlayerSpiderFrame(int id) {
        createSpider(IconCountEditor::clamp(id, IconType::Spider));
    }
    #endif

    void updatePlayerSwingFrame(int id) {
        id = IconCountEditor::clamp(id, IconType::Swing);

        GameManager::get()->loadIcon(id, 7, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_iconSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("swing_{:02}_001.png", id).c_str()));
        m_iconSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("swing_{:02}_2_001.png", id).c_str()));
        m_iconGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("swing_{:02}_glow_001.png", id).c_str()));
        m_iconSpriteSecondary->setPosition(m_iconSprite->getContentSize() / 2.0f);
        updatePlayerSpriteExtra(fmt::format("swing_{:02}_extra_001.png", id));
    }

    void updatePlayerJetpackFrame(int id) {
        id = IconCountEditor::clamp(id, IconType::Jetpack);

        GameManager::get()->loadIcon(id, 8, m_iconRequestID);

        auto sfc = CCSpriteFrameCache::get();
        m_vehicleSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("jetpack_{:02}_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setDisplayFrame(sfc->spriteFrameByName(fmt::format("jetpack_{:02}_2_001.png", id).c_str()));
        m_vehicleGlow->setDisplayFrame(sfc->spriteFrameByName(fmt::format("jetpack_{:02}_glow_001.png", id).c_str()));
        m_vehicleSpriteSecondary->setPosition(m_vehicleSprite->getContentSize() / 2.0f);
        updateShipSpriteExtra(fmt::format("jetpack_{:02}_extra_001.png", id));
    }

    void setupStreak() {
        auto gm = GameManager::get();
        auto textureCache = CCTextureCache::get();

        m_playerStreak = IconCountEditor::clamp(gm->m_playerStreak.value(), IconType::Special);
        m_hasGlow = gm->m_playerGlow;
        m_streakStrokeWidth = 10.0f;
        auto fade = 0.3f;
        auto stroke = 10.0f;
        switch (m_playerStreak) {
            case 2:
            case 7:
                stroke = 14.0f;
                m_disableStreakTint = true;
                m_streakStrokeWidth = 14.0f;
                break;
            case 3:
                m_streakStrokeWidth = 8.5f;
                stroke = 8.5f;
                break;
            case 4:
                fade = 0.4f;
                stroke = 10.0f;
                break;
            case 5:
                m_streakStrokeWidth = 5.0f;
                fade = 0.6f;
                m_alwaysShowStreak = true;
                stroke = 5.0f;
                break;
            case 6:
                fade = 1.0f;
                m_alwaysShowStreak = true;
                m_streakStrokeWidth = 3.0f;
                stroke = 3.0f;
                break;
        }
        auto texture = textureCache->addImage(fmt::format("streak_{:02}_001.png", m_playerStreak).c_str(), false);
        if (!texture) texture = textureCache->addImage("streak_01_001.png", false);
        m_regularTrail = CCMotionStreak::create(fade, 5.0f, stroke, { 255, 255, 255 }, texture);
        m_regularTrail->setM_fMaxSeg(50.0f);
        m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
        m_parentLayer->addChild(m_regularTrail, -2);
        queueInMainThread([selfref = WeakRef(this)] {
            if (auto self = selfref.lock()) {
                if (!self->m_regularTrail->getTexture()) {
                    self->m_regularTrail->setTexture(CCTextureCache::get()->addImage("streak_01_001.png", false));
                }
            }
        });

        auto shipFire = gm->m_playerShipFire.value();
        auto shipStreak = (ShipStreak)shipFire;
        m_shipStreakType = shipStreak;
        shipFire = IconCountEditor::clamp(shipFire, IconType::ShipFire);
        if (shipFire > 1) {
            fade = 0.0f;
            stroke = 0.0f;
            getSettingsForStreak(shipStreak, 1.6f, 1.0f, fade, stroke);
            texture = textureCache->addImage(getFrameForStreak(shipStreak, 0.0f).c_str(), false);
            if (texture) {
                m_shipStreak = CCMotionStreak::create(fade, 1.0f, stroke, { 255, 255, 255 }, texture);
                m_shipStreak->setM_fMaxSeg(50.0f);
                m_shipStreak->setDontOpacityFade(true);
                m_shipStreak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
                m_parentLayer->addChild(m_shipStreak, -3);
            }
        }

        m_waveTrail = HardStreak::create();
        if (gm->m_playerColor.value() == 15 && !m_switchWaveTrailColor) m_waveTrail->m_isSolid = true;
        else m_waveTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        m_parentLayer->addChild(m_waveTrail, -3);
        deactivateStreak(true);
    }
};
