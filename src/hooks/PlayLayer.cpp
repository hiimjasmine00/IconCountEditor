#include <Geode/platform/cplatform.h>
#ifdef GEODE_IS_WINDOWS // DO A FLIP
#include "../IconCountEditor.hpp"
#include <Geode/binding/AudioEffectsLayer.hpp>
#include <Geode/binding/ColorAction.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJEffectManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelTools.hpp>
#include <Geode/binding/LevelSettingsObject.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/binding/ObjectToolbox.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/StartPosObject.hpp>
#include <Geode/binding/UILayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(ICEPlayLayer, PlayLayer) {
    static void onModify(ModifyBase<ModifyDerive<ICEPlayLayer, PlayLayer>>& self) {
        IconCountEditor::modify(self.m_hooks, "PlayLayer::setupHasCompleted", { IconType::DeathEffect });
    }

    void setupHasCompleted() {
        createTextLayers();
        updateSpecialGroupData();
        optimizeColorGroups();
        optimizeOpacityGroups();
        scanDynamicSaveObjects();
        scanActiveSaveObjects();

        updateLayerCapacity(m_level->m_capacityString);
        increaseBatchNodeCapacity();

        if (!m_levelSettings) {
            m_levelSettings = LevelSettingsObject::create();
            m_levelSettings->m_level = m_level;
            m_levelSettings->retain();
        }
        m_player1->m_isPlatformer = m_isPlatformer;
        m_player2->m_isPlatformer = m_isPlatformer;

        auto playerColor1 = m_effectManager->getColorAction(1005);
        playerColor1->m_duration = 0.0f;
        playerColor1->m_fromColor = m_player1->m_playerColor1;
        playerColor1->m_blending = true;
        auto playerColor2 = m_effectManager->getColorAction(1006);
        playerColor2->m_duration = 0.0f;
        playerColor2->m_fromColor = m_player1->m_playerColor2;
        playerColor2->m_blending = true;

        auto gm = GameManager::get();
        auto deathEffect = gm->m_playerDeathEffect.value();
        if (deathEffect > 0) {
            deathEffect = IconCountEditor::clamp(deathEffect, IconType::DeathEffect);
            if (gm->m_loadedDeathEffect != deathEffect) {
                auto pngPath = fmt::format("PlayerExplosion_{:02}.png", deathEffect - 1);
                auto plistPath = fmt::format("PlayerExplosion_{:02}.plist", deathEffect - 1);

                auto fileUtils = CCFileUtils::get();
                if (!fileUtils->isFileExist(pngPath) || !fileUtils->isFileExist(plistPath)) deathEffect = 1;

                auto textureCache = CCTextureCache::get();
                if (gm->m_loadedDeathEffect > 1) {
                    textureCache->removeTextureForKey(fmt::format("PlayerExplosion_{:02}.png", gm->m_loadedDeathEffect - 1).c_str());
                }
                if (deathEffect > 1) {
                    CCSpriteFrameCache::get()->addSpriteFramesWithFile(plistPath.c_str(), textureCache->addImage(pngPath.c_str(), false));
                }
                gm->m_loadedDeathEffect = deathEffect;
            }
        }

        createBackground(m_levelSettings->m_backgroundIndex);
        if (m_levelSettings->m_middleGroundIndex > 0) createMiddleground(m_levelSettings->m_middleGroundIndex);
        createGroundLayer(m_levelSettings->m_groundIndex, m_levelSettings->m_groundLineIndex);

        m_uiLayer->togglePlatformerMode(m_isPlatformer);

        auto color = m_player1->m_playerColor1;
        m_glitterParticles->setStartColor({ color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f });
        m_glitterParticles->setEndColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        resetPlayer();

        auto fae = FMODAudioEngine::get();
        fae->loadMusic(m_level->getAudioFileName(), 1.0f, 0.0f, 1.0f, true, 0, 0, m_levelSettings->m_dontReset);
        auto songID = m_level->m_songID;
        if (songID == 0 && m_level->m_audioTrack < 20) {
            fae->disableMetering();
        }
        else {
            fae->enableMetering();
            m_skipAudioStep = true;
            MusicDownloadManager::sharedState()->incrementPriorityForSong(songID);
        }
        m_isSilent = fae->getBackgroundMusicVolume() <= 0.0f;
        m_audioEffectsLayer = AudioEffectsLayer::create(LevelTools::getAudioString(m_level->m_audioTrack));
        m_audioEffectsLayer->setVisible(false);
        m_objectLayer->addChild(m_audioEffectsLayer, 1);

        m_attempts = 0;
        m_attemptLabel = CCLabelBMFont::create("Attempt 1", gm->getFontFile(gm->m_loadedFont));
        if (m_doNot) m_attemptLabel->setScaleY(-1.0f);
        m_objectLayer->addChild(m_attemptLabel, 1400);

        auto winSize = CCDirector::get()->getWinSize();
        m_infoLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_infoLabel->setPosition({ 5.0f, winSize.height - 10.0f });
        m_infoLabel->setScale(0.5f);
        m_infoLabel->setAnchorPoint({ 0.0f, 1.0f });
        m_infoLabel->setOpacity(150);
        addChild(m_infoLabel, 1000);
        updateInfoLabel();
        toggleInfoLabel();

        auto spawnDelay = 1.0f;
        if (m_levelSettings->m_twoPlayerMode && !m_isTestMode && m_level->m_levelType != GJLevelType::Editor && !m_isPlatformer) {
            m_attemptLabel->setVisible(false);
            runAction(CCSequence::createWithTwoActions(
                CCDelayTime::create(0.5f),
                CCCallFunc::create(this, callfunc_selector(PlayLayer::showTwoPlayerGuide))
            ));
            spawnDelay = 2.0f;
        }

        m_freezeStartCamera = true;
        m_unk322a = true;
        m_skipArtReload = true;
        updateCamera(0.0f);

        m_progressBar = CCSprite::create("slidergroove2.png");
        m_progressBar->setPosition({ winSize.width / 2.0f, winSize.height - 8.0f });
        m_progressWidth = m_progressBar->getTextureRect().size.width - 4.0f;
        m_progressHeight = 8.0f;
        addChild(m_progressBar, 10);

        m_progressFill = CCSprite::create("sliderBar2.png");
        m_progressFill->setPosition({ 2.0f, 4.0f });
        m_progressFill->setAnchorPoint({ 0.0f, 0.0f });
        ccTexParams tp = { GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT };
        m_progressFill->getTexture()->setTexParameters(&tp);
        m_progressFill->setColor(m_player1->m_playerColor1);
        m_progressBar->addChild(m_progressFill, -1);

        m_percentageLabel = CCLabelBMFont::create("100%", "bigFont.fnt");
        m_percentageLabel->setPosition({ winSize.width / 2.0f + 110.0f, winSize.height - 8.0f });
        m_percentageLabel->setScale(0.5f);
        m_percentageLabel->setAnchorPoint({ 0.0f, 0.5f });
        addChild(m_percentageLabel, 15);

        updateProgressbar();
        toggleProgressbar();

        if (!m_isTestMode && !m_levelSettings->m_propertykA24 && m_levelSettings->m_spawnGroup < 1) {
            m_player1->setVisible(false);
            m_player2->setVisible(false);
        }
        else {
            m_player1->setVisible(true);
            m_player2->setVisible(true);
        }

        loadDefaultColors();
        updateLevelColors();

        setupLevelStart(m_startPosObject ? m_startPosObject->m_startSettings : m_levelSettings);

        m_objectsDeactivated = true;
        updateVisibility(0.0f);
        updateCamera(0.0f);
        toggleGlitter(false);
        m_disableGravityEffect = gm->getGameVariable("0072");

        auto spikeFrame = ObjectToolbox::sharedState()->intKeyToFrame(8);
        m_anticheatSpike = GameObject::createWithKey(8);
        m_anticheatSpike->customSetup();
        m_anticheatSpike->addColorSprite(spikeFrame);
        m_anticheatSpike->setupCustomSprites(spikeFrame);
        m_anticheatSpike->setStartPos(m_player1->getPosition());
        m_anticheatSpike->setDontDraw(true);
        addObject(m_anticheatSpike);
        createPlayerCollisionBlock();

        m_attemptLabel->setPosition(winSize / 2.0f + m_gameState.m_cameraPosition + CCPoint { 0.0f, 85.0f });
        if (m_isTestMode) m_attemptLabel->setOpacity(50);

        m_freezeStartCamera = true;
        resetLevel();
        m_started = false;
        prepareMusic(false);
        runAction(CCSequence::createWithTwoActions(
            CCDelayTime::create(spawnDelay),
            CCCallFunc::create(this, m_passedIntegrity ? callfunc_selector(PlayLayer::startGame) : callfunc_selector(PlayLayer::onQuit))
        ));
        scheduleUpdate();
        checkSpawnObjects();
        m_effectManager->updateEffects(0.0f);
        updateVisibility(0.0f);
        updateLevelColors();
        updateProximityVolumeEffects();
        updateTestModeLabel();

        if (shouldExitHackedLevel()) {
            removeAllObjects();
            onQuit();
        }
    }
};
#endif
