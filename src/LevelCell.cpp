#include <Geode/modify/LevelCell.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

#define PREFERRED_HOOK_PRIO (-2123456789)

using namespace geode::prelude;

class $modify(MyLevelCell, LevelCell) {
	struct Fields {
		Manager* manager = Manager::getSharedInstance();
	};
	static void onModify(auto & self)
	{
		(void) self.setHookPriority("LevelCell::onClick", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadFromLevel", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadLocalLevelCell", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadCustomLevelCell", PREFERRED_HOOK_PRIO);
	}
	void onShowLevelDesc(CCObject* sender) {
		const auto theLevel = this->m_level;
		std::string levelDesc = theLevel->getUnpackedLevelDescription();
		if (levelDesc.empty()) {
			if (Utils::doesNodeExistNoParent("provider-popup") || Utils::doesNodeExistNoParent("dogotrigger.level_history/provider-popup")) {
				levelDesc = "(No description available. You're probably viewing this level from a level history mod; there's not much more you can do from here.)";
			} else if (Utils::doesNodeExist("LevelBrowserLayer", "saved-menu") || Utils::isSceneRunning("LevelListLayer")) {
				levelDesc = "(No description visible. Try downloading the level, then exit and re-enter this menu to view this level's description again.)";
			} else {
				levelDesc = "(No description provided)";
			}
		}
		FLAlertLayer::create(
			nullptr,
			std::string(theLevel->m_levelName).c_str(),
			fmt::format("Pubished by {}\n\n{}", theLevel->m_creatorName, levelDesc),
			"Close",
			nullptr,
			420.f
		)->show();
	}

	void addColorToSequence(CCArray *arrayOfSequences, ccColor4B color) {
		arrayOfSequences->addObject(CCTintTo::create(static_cast<float>(Utils::getDouble("songCycleSpeed")), color.r, color.g, color.b));
	}

	void applySongRecoloring(cocos2d::CCLayer* mainLayer, GJGameLevel* level) {
		if (const auto songLabel = typeinfo_cast<CCLabelBMFont*>(mainLayer->getChildByIDRecursive("song-name"))) {
			std::string songIDs = level->m_songIDs;
			const int defaultSongID = level->m_songID;
			const bool ncs = mainLayer->getChildByIDRecursive("ncs-icon");
			const bool songIDsEmpty = songIDs.empty();
			const bool defaultSongIsNCSOrML = defaultSongID >= 10000000;
			const bool defaultSongIsNewgrounds = defaultSongID >= 1 && defaultSongID < 10000000;
			if (!Utils::getBool("cycleThroughColors") || songIDsEmpty || songIDs.find(',') == std::string::npos) {
				ccColor4B colorAlpha;
				if (defaultSongIsNCSOrML) {
					colorAlpha = Utils::getColorAlpha("musicLibraryColor");
					if (ncs && Utils::getBool("recolorNCS")) { colorAlpha = Utils::getColorAlpha("ncsColor"); }
				} else if (defaultSongIsNewgrounds) {
					colorAlpha = Utils::getColorAlpha("newgroundsColor");
				} else {
					colorAlpha = Utils::getColorAlpha("defaultSongColor");
				}
				songLabel->setColor({colorAlpha.r, colorAlpha.g, colorAlpha.b});
				songLabel->setOpacity(colorAlpha.a);
			} else {
				std::vector<std::string> songIDVector = utils::string::split(songIDs, ",");
				int ngSongs = 0;
				int defaultSongs = 0;
				int ncsSongs = 0;
				int musicLibrarySongs = 0;
				for (const auto& songIDString : songIDVector) {
					int songIDInt = utils::numFromString<int>(songIDString).unwrapOr(INT_MIN);
					if (songIDInt == INT_MIN) { continue; }
					ccColor4B colorToAdd;
					if (songIDInt >= 10000000) {
						auto songInfoObject = MusicDownloadManager::sharedState()->getSongInfoObject(songIDInt);
						if (!songInfoObject) { continue; }
						if ((songInfoObject->m_songUrl.find("NCS") != std::string::npos || songInfoObject->m_songUrl.find("ncs") != std::string::npos) && Utils::getBool("recolorNCS")) {
							ncsSongs++;
						} else {
							musicLibrarySongs++;
						}
					} else if (songIDInt >= 1) {
						ngSongs++;
					} else {
						defaultSongs++;
					}
				}
				auto arrayOfSequences = CCArray::create();
				if ((ncsSongs > 0 || ncs) && Utils::getBool("recolorNCS")) {
					addColorToSequence(arrayOfSequences, Utils::getColorAlpha("ncsColor"));
				}
				if (defaultSongs > 0 || defaultSongID < 1) {
					ccColor4B color = Utils::getColorAlpha("defaultSongColor");
					arrayOfSequences->addObject(CCTintTo::create(static_cast<float>(Utils::getDouble("songCycleSpeed")), color.r, color.g, color.b));
				}
				if (ngSongs > 0) {
					ccColor4B color = Utils::getColorAlpha("newgroundsColor");
					arrayOfSequences->addObject(CCTintTo::create(static_cast<float>(Utils::getDouble("songCycleSpeed")), color.r, color.g, color.b));
				}
				if (musicLibrarySongs > 0) {
					ccColor4B color = Utils::getColorAlpha("musicLibraryColor");
					arrayOfSequences->addObject(CCTintTo::create(static_cast<float>(Utils::getDouble("songCycleSpeed")), color.r, color.g, color.b));
				}
				CCActionInterval* sequence = CCSequence::create(arrayOfSequences);
				CCAction* repeat = CCRepeatForever::create(sequence);
				songLabel->runAction(repeat);
			}
		}
	}
	void onClick(CCObject* sender) {
		// hooking this function is necessary in order for the "view" button to work while compact mode is active in "my levels"
		if (this->m_level->m_levelType == GJLevelType::Editor && Utils::modEnabled() && Utils::getBool("compactEditorLevels")) {
			const auto scene = CCScene::create();
			scene->addChild(EditLevelLayer::create(m_level));
			CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));
		} else { LevelCell::onClick(sender); }
	}
	void loadLocalLevelCell() {
		LevelCell::loadLocalLevelCell();
		if (!(Utils::modEnabled() && Utils::getBool("compactEditorLevels"))) { return; }
		if (const auto localLevelname = typeinfo_cast<CCLabelBMFont*>(getChildByIDRecursive("level-name"))) { localLevelname->limitLabelWidth(200.f, .6f, .01f); }
		if (const auto mainLayer = typeinfo_cast<CCLayer*>(getChildByIDRecursive("main-layer"))) { mainLayer->setPositionY(-3.f); }
	}
	void loadCustomLevelCell() {
		LevelCell::loadCustomLevelCell();
		if (!Utils::modEnabled()) { return; }
		const auto mainLayer = this->m_mainLayer;
		if (!mainLayer || !m_level) { return; }
		if (Utils::getBool("levelDescriptions")) {
			if (const auto viewButton = mainLayer->getChildByIDRecursive("view-button")) {
				const auto infoButton = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
				infoButton->setScale(.6f);
				auto descButton = CCMenuItemSpriteExtra::create(infoButton, this, menu_selector(MyLevelCell::onShowLevelDesc));
				descButton->setID("level-desc-button"_spr);
				#ifndef GEODE_IS_MOBILE
				descButton->setPosition({
					viewButton->getPositionX() + (viewButton->getContentSize().width / 2.f),
					viewButton->getPositionY() - (viewButton->getContentSize().height / 2.f)
				});
				#else
				if (!Utils::isSceneRunningRecursive("DailyLevelPage")) {
					if (const auto mainLayer = getChildByID("main-layer")) {
						descButton->setPosition({
							mainLayer->getPositionX() - (mainLayer->getContentSize().width / 2.f) + 7.5f,
							mainLayer->getPositionY() - (mainLayer->getContentSize().height / 2.f) + 7.5f
						});
					}
				} else {
					descButton->setPosition({
						viewButton->getPositionX() + (viewButton->getContentSize().width / 2.f),
						viewButton->getPositionY() - (viewButton->getContentSize().height / 2.f)
					});
				}
				#endif
				getChildByIDRecursive("main-menu")->addChild(descButton);
			}
		}
	}
	void loadFromLevel(GJGameLevel* level) {
		LevelCell::loadFromLevel(level);
		if (!Utils::modEnabled()) { return; }
		const auto mainLayer = this->m_mainLayer;
		if (!mainLayer || !m_level) { return; }
		if (Utils::getBool("recolorSongLabels")) applySongRecoloring(mainLayer, m_level);
		if (Utils::getBool("recolorLevelNameFeaturedScore")) {
			const auto diffContainerNode = mainLayer->getChildByIDRecursive("difficulty-container");
			if (!diffContainerNode) { return; }
			const auto diffSpriteNode = diffContainerNode->getChildByIDRecursive("difficulty-sprite");
			if (!diffSpriteNode) { return; }
			const auto diffSprite = typeinfo_cast<GJDifficultySprite*>(diffSpriteNode);
			if (!diffSprite) { return; }
			const auto levelNameLabel = typeinfo_cast<CCLabelBMFont*>(mainLayer->getChildByIDRecursive("level-name"));
			const auto featureState = diffSprite->m_featureState;
			if (!levelNameLabel || featureState == GJFeatureState::None) { return; }
			const auto levelNameLabelColor = levelNameLabel->getColor();
			const auto defaultColor = CCTintTo::create(1.5f, levelNameLabelColor.r, levelNameLabelColor.g, levelNameLabelColor.b);
			auto color = ccColor3B(50, 200, 255);
			if (featureState == ::GJFeatureState::Featured) color = ccColor3B(255, 255, 0);
			else if (featureState == ::GJFeatureState::Epic) color = ccColor3B(255, 90, 75);
			else if (featureState == ::GJFeatureState::Legendary) color = ccColor3B(255, 0, 255);
			const auto featuredColor = CCTintTo::create(1.5f, color.r, color.g, color.b);
			CCActionInterval* sequence = CCSequence::create(defaultColor, featuredColor, nullptr);
			CCAction* repeat = CCRepeatForever::create(sequence);
			levelNameLabel->runAction(repeat);
		}
		if (Utils::getBool("blendingText")) {
			for (const auto node : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
				if (const auto label = typeinfo_cast<CCLabelBMFont*>(node)) {
					if (std::string(label->getFntFile()) == "chatFont.fnt") {
						label->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
						label->setColor({255, 255, 255});
						label->setOpacity(255);
					}
				}
			}
		}
	}
};