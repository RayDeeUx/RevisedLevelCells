#include <Geode/modify/LevelCell.hpp>
#include "Manager.hpp"
#include "Utils.hpp"
#include <regex>

#define PREFERRED_HOOK_PRIO (-3999)
#define LEVEL_PLACEMENT_OFFSET 20.f

using namespace geode::prelude;

class $modify(MyLevelCell, LevelCell) {
	struct Fields {
		Manager* manager = Manager::getSharedInstance();
		bool blendingApplied = false;
	};
	static void onModify(auto & self) {
		(void) self.setHookPriority("LevelCell::draw", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::onClick", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadFromLevel", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadLocalLevelCell", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadCustomLevelCell", -PREFERRED_HOOK_PRIO);
	}
	static double getInfoButtonScale() {
		#ifdef GEODE_IS_MOBILE
		return 0.6f;
		#else
		return Utils::getDouble("levelDescriptionScale");
		#endif
	}
	static std::string getInfoButtonLocation() {
		#ifdef GEODE_IS_MOBILE
		return "Top Left of Level Cell";
		#else
		return Utils::getString("levelDescriptionsPosition");
		#endif
	}
	void onShowLevelDesc(CCObject* sender) {
		if (!Utils::modEnabled()) return;
		const auto theLevel = this->m_level;
		if (theLevel->m_levelType == GJLevelType::Local || theLevel->m_levelType == GJLevelType::Local) return;
		std::string levelDesc = theLevel->getUnpackedLevelDescription();
		if (levelDesc.empty()) {
			if (Utils::doesNodeExistNoParent("provider-popup") || Utils::doesNodeExistNoParent("dogotrigger.level_history/provider-popup")) {
				levelDesc = "(No description available. You're probably viewing this level from a level history mod; there's not much more you can do from here.)";
			} else if (Utils::doesNodeExist("LevelBrowserLayer", "saved-menu")) {
				if (std::string(theLevel->m_levelString).empty()) levelDesc = "(No description visible. Try downloading the level again.)";
				else levelDesc = "(No description provided)";
			} else if (Utils::isSceneRunning("LevelListLayer")) {
				levelDesc = "(No description visible. Try downloading the level, then exit and re-enter this level list to view this level's description again.)";
			} else {
				levelDesc = "(No description provided)";
			}
		}
		std::string levelInfo = "(Level info unavailable)";
		std::string songInfo = "(Song info unavailable)";
		if (const auto songInfoObject = MusicDownloadManager::sharedState()->getSongInfoObject(theLevel->m_songID)) songInfo = fmt::format("{} by {} [ID: {}]", songInfoObject->m_songName, songInfoObject->m_artistName, theLevel->m_songID);
		levelInfo = fmt::format("Level ID: <cy>{}</c>\nSong: <cy>{}</c>", theLevel->m_levelID.value(), songInfo);
		if (!std::string(theLevel->m_songIDs).empty()) levelInfo = utils::string::replace(levelInfo, "Song", "Primary Song");
		if (!std::string(theLevel->m_sfxIDs).empty()) levelInfo = levelInfo.append(fmt::format("\nSFX IDs: <cy>{}</c>", std::regex_replace(std::string(theLevel->m_sfxIDs), std::regex(","), ", ")));
		FLAlertLayer::create(
			nullptr,
			theLevel->m_levelName.c_str(),
			fmt::format("Published by {}\n\n{}\n\n{}", theLevel->m_creatorName, levelDesc, levelInfo),
			"Close",
			nullptr,
			420.f,
			levelInfo.length() > 300,
			320.f,
			1.0f
		)->show();
	}
	void addColorToSequence(CCArray *arrayOfSequences, ccColor4B color) {
		if (!Utils::modEnabled()) return;
		arrayOfSequences->addObject(CCTintTo::create(static_cast<float>(Utils::getDouble("songCycleSpeed")), color.r, color.g, color.b));
	}
	static void applyFeatureStateRecoloring(CCLayer* mainLayer) {
		if (!Utils::modEnabled() || !Utils::getBool("recolorLevelNameFeaturedScore")) return;
		const auto diffContainerNode = mainLayer->getChildByIDRecursive("difficulty-container");
		if (!diffContainerNode) return;
		const auto diffSpriteNode = diffContainerNode->getChildByIDRecursive("difficulty-sprite");
		if (!diffSpriteNode) return;
		const auto diffSprite = typeinfo_cast<GJDifficultySprite*>(diffSpriteNode);
		if (!diffSprite) return;
		const auto levelNameLabel = typeinfo_cast<CCLabelBMFont*>(mainLayer->getChildByIDRecursive("level-name"));
		const auto featureState = diffSprite->m_featureState;
		if (!levelNameLabel || featureState == GJFeatureState::None) return;
		const auto levelNameLabelColor = levelNameLabel->getColor();
		const auto defaultColor = CCTintTo::create(Utils::getDouble("pulsingSpeed"), levelNameLabelColor.r, levelNameLabelColor.g, levelNameLabelColor.b);
		auto color = ccColor3B{255, 255, 255};
		if (featureState == GJFeatureState::Featured) if (Utils::getBool("recolorFeatured")) color = ccColor3B{255, 255, 0}; else return;
		else if (featureState == GJFeatureState::Epic) if (Utils::getBool("recolorEpic")) color = ccColor3B{255, 90, 75}; else return;
		else if (featureState == GJFeatureState::Legendary) if (Utils::getBool("recolorLegendary")) color = ccColor3B{255, 0, 255}; else return;
		else if (featureState == GJFeatureState::Mythic) if (Utils::getBool("recolorMythic")) color = ccColor3B{50, 200, 255}; else return;
		if (color.r == 255 && color.g == 255 && color.b == 255) return;
		const auto featuredColor = CCTintTo::create(Utils::getDouble("pulsingSpeed"), color.r, color.g, color.b);
		CCActionInterval* sequence = CCSequence::create(defaultColor, featuredColor, nullptr);
		CCAction* repeat = CCRepeatForever::create(sequence);
		levelNameLabel->runAction(repeat);
	}
	void applySongRecoloring(cocos2d::CCLayer* mainLayer, const GJGameLevel* level) {
		if (!Utils::modEnabled() || !Utils::getBool("recolorSongLabels")) return;
		const auto songName = mainLayer->getChildByIDRecursive("song-name");
		if (!songName) return;
		const auto songLabel = typeinfo_cast<CCLabelBMFont*>(songName);
		if (!songLabel) return;
		const std::string songIDs = level->m_songIDs;
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
					if (utils::string::toLower(std::string(songInfoObject->m_songUrl)).find("ncs") != std::string::npos && Utils::getBool("recolorNCS")) {
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
			if ((ncsSongs > 0 || ncs) && Utils::getBool("recolorNCS")) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("ncsColor"));
			if (defaultSongs > 0 || defaultSongID < 1) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("defaultSongColor"));
			if (ngSongs > 0) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("newgroundsColor"));
			if (musicLibrarySongs > 0) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("musicLibraryColor"));
			CCActionInterval* sequence = CCSequence::create(arrayOfSequences);
			CCAction* repeat = CCRepeatForever::create(sequence);
			songLabel->runAction(repeat);
		}
	}
	void applyLevelDescriptions(CCLayer* mainLayer) {
		if (!Utils::modEnabled() || !Utils::getBool("levelDescriptions") || m_level->m_levelType == GJLevelType::Editor) return;
		const auto viewButton = mainLayer->getChildByIDRecursive("view-button");
		if (!viewButton) return;
		const auto infoButton = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
		infoButton->setScale(getInfoButtonScale());
		if (Utils::getDouble("levelDescriptionScale") > .6f) infoButton->setScale(.6f);
		auto descButton = CCMenuItemSpriteExtra::create(infoButton, this, menu_selector(MyLevelCell::onShowLevelDesc));
		descButton->setID("level-desc-button"_spr);
		auto buttonPosSetting = getInfoButtonLocation();
		if (buttonPosSetting == "Bottom Left of Level Cell") {
			descButton->setPosition({
				mainLayer->getPositionX() - (mainLayer->getContentSize().width / 2.f) + 7.5f,
				mainLayer->getPositionY() - (mainLayer->getContentSize().height / 2.f) + 7.5f
			});
		} else if (buttonPosSetting == "Top Left of Level Cell") {
			float yOffset = 81.5f;
			if (m_listType == BoomListType::Level4) yOffset = 41.5f;
			descButton->setPosition({
				mainLayer->getPositionX() - (mainLayer->getContentSize().width / 2.f) + 7.5f,
				mainLayer->getPositionY() - (mainLayer->getContentSize().height / 2.f) + yOffset
			});
		} else {
			descButton->setPosition({
				viewButton->getPositionX() + (viewButton->getContentSize().width / 2.f),
				viewButton->getPositionY() - (viewButton->getContentSize().height / 2.f)
			});
		}
		if (buttonPosSetting.find_first_of("Level Cell") != std::string::npos || buttonPosSetting.find_first_of(" Button") != std::string::npos) getChildByIDRecursive("main-menu")->addChild(descButton);
	}
	void removePlacement() const {
		if (!Utils::modEnabled() || !Utils::isModLoaded("cvolton.compact_lists") || !m_mainLayer) return;
		if (!Utils::getMod("cvolton.compact_lists")->getSettingValue<bool>("enable-compact-lists")) return;
		if (m_level->m_listPosition == 0) return; // those cases should be handed by cvolton by now
		const auto label = m_mainLayer->getChildByID("level-place");
		if (!label) return;
		label->setVisible(false);
		for (auto child : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
			if (child->getID() == "main-menu") continue;
			child->setPositionX(child->getPositionX() - LEVEL_PLACEMENT_OFFSET);
		}
		const auto menu = m_mainLayer->getChildByID("main-menu");
		if (!menu) return;
		for (auto child : CCArrayExt<CCNode*>(menu->getChildren())) {
			if (child->getID() == "view-button") continue;
			child->setPositionX(child->getPositionX() - LEVEL_PLACEMENT_OFFSET);
		}
		const auto viewButton = m_mainMenu->getChildByID("view-button");
		if (!viewButton) return;
		m_mainLayer->getChildByID("completed-icon")->setPosition({276.f, 25.f});
		m_mainLayer->getChildByID("percentage-label")->setPosition({276.f, 25.f});
	}
	void applyBlendingText() {
		if (!Utils::modEnabled() || !Utils::getBool("blendingText") || !m_mainLayer || m_fields->blendingApplied) return;
		for (const auto node : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
			if (const auto label = typeinfo_cast<CCLabelBMFont*>(node)) {
				if (std::string(label->getFntFile()) == "chatFont.fnt") {
					label->setBlendFunc({GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
					label->setColor({255, 255, 255});
					label->setOpacity(255);
				}
			}
		}
		m_fields->blendingApplied = true;
	}
	void onClick(CCObject* sender) {
		// hooking this function is necessary in order for the "view" button to work while compact mode is active in "my levels"
		if (this->m_level->m_levelType == GJLevelType::Editor && Utils::modEnabled() && Utils::getBool("compactEditorLevels"))
			CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, EditLevelLayer::scene(m_level)));
		else LevelCell::onClick(sender);
	}
	void loadLocalLevelCell() {
		LevelCell::loadLocalLevelCell();
		if (!(Utils::modEnabled() && Utils::getBool("compactEditorLevels"))) return;
		if (const auto localLevelName = typeinfo_cast<CCLabelBMFont*>(getChildByIDRecursive("level-name"))) localLevelName->limitLabelWidth(200.f, .6f, .01f);
		if (const auto mainLayer = typeinfo_cast<CCLayer*>(getChildByIDRecursive("main-layer"))) mainLayer->setPositionY(-3.f);
	}
	void loadCustomLevelCell() {
		LevelCell::loadCustomLevelCell();
		if (!Utils::modEnabled() || !m_mainMenu || !m_level) return;
		if (Utils::getBool("removePlacement")) removePlacement();
	}
	void loadFromLevel(GJGameLevel* level) {
		LevelCell::loadFromLevel(level);
		if (!Utils::modEnabled()) return;
		const auto mainLayer = this->m_mainLayer;
		if (!mainLayer || !m_level) return;
		if (Utils::getBool("recolorSongLabels")) applySongRecoloring(mainLayer, m_level);
		if (Utils::getBool("recolorLevelNameFeaturedScore")) applyFeatureStateRecoloring(mainLayer);
		if (Utils::getBool("levelDescriptions") && level->m_levelType != GJLevelType::Editor) applyLevelDescriptions(mainLayer);
	}
	void draw() {
		LevelCell::draw();
		if (!Utils::modEnabled()) return;
		if (Utils::getBool("blendingText")) applyBlendingText();
	}
};