#include <Geode/modify/LevelListLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include "Manager.hpp"
#include "Utils.hpp"
#include <regex>

#define PREFERRED_HOOK_PRIO (-3999)
#define LEVEL_PLACEMENT_OFFSET 20.f

using namespace geode::prelude;

class $modify(MyLevelCell, LevelCell) {
	struct Fields {
		bool blendingApplied = false;
	};
	static void onModify(auto & self) {
		(void) self.setHookPriority("LevelCell::draw", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::onClick", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadFromLevel", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadLocalLevelCell", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelCell::loadCustomLevelCell", PREFERRED_HOOK_PRIO);
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
	void onShowLevelDesc(CCObject*) {
		if (!Utils::modEnabled()) return;
		GJGameLevel* theLevel = this->m_level;
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
			} else levelDesc = "(No description provided)";
		}
		std::string levelInfo = "(Level info unavailable)";
		std::string songInfo = "(Song info unavailable)";
		if (SongInfoObject* songInfoObject = MusicDownloadManager::sharedState()->getSongInfoObject(theLevel->m_songID)) songInfo = fmt::format("{} by {} [ID: {}]", songInfoObject->m_songName, songInfoObject->m_artistName, theLevel->m_songID);
		levelInfo = fmt::format("Level ID: <cy>{}</c>\nSong: <cy>{}</c>", theLevel->m_levelID.value(), songInfo);
		if (!std::string(theLevel->m_songIDs).empty()) levelInfo = utils::string::replace(levelInfo, "Song", "Primary Song");
		if (!std::string(theLevel->m_sfxIDs).empty()) levelInfo = levelInfo.append(fmt::format("\nSFX IDs: <cy>{}</c>", std::regex_replace(std::string(theLevel->m_sfxIDs), std::regex(","), ", ")));
		FLAlertLayer::create(
			nullptr,
			theLevel->m_levelName.c_str(),
			fmt::format("Published by {}\n\n{}\n\n{}", theLevel->m_creatorName, levelDesc, levelInfo),
			"Close", nullptr, 420.f,
			levelInfo.length() > 300,
			320.f, 1.0f
		)->show();
	}
	static void addColorToSequence(CCArray *arrayOfSequences, const ccColor4B color) {
		if (!Utils::modEnabled()) return;
		arrayOfSequences->addObject(CCTintTo::create(static_cast<float>(Utils::getDouble("songCycleSpeed")), color.r, color.g, color.b));
	}
	static void applyFeatureStateRecoloring(CCLayer* mainLayer) {
		if (!Utils::modEnabled() || !Utils::getBool("recolorLevelNameFeaturedScore")) return;
		CCNode* diffContainerNode = mainLayer->getChildByIDRecursive("difficulty-container");
		if (!diffContainerNode) return;
		CCNode* diffSpriteNode = diffContainerNode->getChildByIDRecursive("difficulty-sprite");
		if (!diffSpriteNode) return;
		const GJDifficultySprite* diffSprite = typeinfo_cast<GJDifficultySprite*>(diffSpriteNode);
		if (!diffSprite) return;
		const auto levelNameLabel = typeinfo_cast<CCLabelBMFont*>(mainLayer->getChildByIDRecursive("level-name"));
		if (!levelNameLabel) return log::info("no levelNameLabel node");
		const GJFeatureState featureState = diffSprite->m_featureState;
		if (featureState == GJFeatureState::None) return;
		const auto [r, g, b] = levelNameLabel->getColor();
		CCTintTo* defaultColor = CCTintTo::create(Utils::getDouble("pulsingSpeed"), r, g, b);
		if (!defaultColor) return log::info("unable to create defaultColor for levelNameLabel");
		auto color = ccColor3B{255, 255, 255};
		if (featureState == GJFeatureState::Featured) { if (Utils::getBool("recolorFeatured")) { color = ccColor3B{255, 255, 0}; } else return; }
		else if (featureState == GJFeatureState::Epic) { if (Utils::getBool("recolorEpic")) { color = ccColor3B{255, 90, 75}; } else return; }
		else if (featureState == GJFeatureState::Legendary) { if (Utils::getBool("recolorLegendary")) { color = ccColor3B{255, 0, 255}; } else return; }
		else if (featureState == GJFeatureState::Mythic) { if (Utils::getBool("recolorMythic")) { color = ccColor3B{50, 200, 255}; } else return; }
		if (color.r == 255 && color.g == 255 && color.b == 255) return;
		CCTintTo* featuredColor = CCTintTo::create(Utils::getDouble("pulsingSpeed"), color.r, color.g, color.b);
		if (!featuredColor) return log::info("unable to create featuredColor for levelNameLabel");
		CCActionInterval* sequence = CCSequence::create(defaultColor, featuredColor, nullptr);
		if (!sequence) return log::info("could not create sequence for levelNameLabel");
		CCAction* repeat = CCRepeatForever::create(sequence);
		if (!repeat) return log::info("unable to create repeat for levelNameLabel node");
		levelNameLabel->runAction(repeat);
	}
	static void applySongRecoloring(cocos2d::CCLayer* mainLayer, const GJGameLevel* level) {
		if (!Utils::modEnabled() || !Utils::getBool("recolorSongLabels")) return;
		CCNode* songName = mainLayer->getChildByIDRecursive("song-name");
		if (!songName) return;
		const auto songLabel = typeinfo_cast<CCLabelBMFont*>(songName);
		if (!songLabel) return;
		const std::string& songIDs = level->m_songIDs;
		const int defaultSongID = level->m_songID;
		const bool ncs = mainLayer->getChildByIDRecursive("ncs-icon");
		const bool songIDsEmpty = songIDs.empty();
		const bool defaultSongIsNCSOrML = defaultSongID >= 10000000;
		const bool defaultSongIsNewgrounds = defaultSongID >= 1 && defaultSongID < 10000000;
		if (!Utils::getBool("cycleThroughColors") || songIDsEmpty || !utils::string::contains(songIDs, ',')) {
			ccColor4B colorAlpha;
			if (defaultSongIsNCSOrML) {
				colorAlpha = Utils::getColorAlpha("musicLibraryColor");
				if (ncs && Utils::getBool("recolorNCS")) colorAlpha = Utils::getColorAlpha("ncsColor");
			} else if (defaultSongIsNewgrounds) colorAlpha = Utils::getColorAlpha("newgroundsColor");
			else colorAlpha = Utils::getColorAlpha("defaultSongColor");
			songLabel->setColor({colorAlpha.r, colorAlpha.g, colorAlpha.b});
			songLabel->setOpacity(colorAlpha.a);
		} else {
			const std::vector<std::string> songIDVector = utils::string::split(songIDs, ",");
			int ngSongs = 0;
			int defaultSongs = 0;
			int ncsSongs = 0;
			int musicLibrarySongs = 0;
			for (const std::string& songIDString : songIDVector) {
				const int songIDInt = utils::numFromString<int>(songIDString).unwrapOr(INT_MIN);
				if (songIDInt == INT_MIN) continue;
				if (songIDInt >= 10000000) {
					const SongInfoObject* songInfoObject = MusicDownloadManager::sharedState()->getSongInfoObject(songIDInt);
					if (!songInfoObject) continue;
					if (utils::string::contains(utils::string::toLower(static_cast<std::string>(songInfoObject->m_songUrl)), "ncs") && Utils::getBool("recolorNCS")) {
						ncsSongs++;
					} else musicLibrarySongs++;
				} else if (songIDInt >= 1) ngSongs++;
				else defaultSongs++;
			}
			CCArray* arrayOfSequences = CCArray::create();
			if (!arrayOfSequences) return log::info("unable to create arrayOfSequences for songLabel node");
			if ((ncsSongs > 0 || ncs) && Utils::getBool("recolorNCS")) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("ncsColor"));
			if (defaultSongs > 0 || defaultSongID < 1) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("defaultSongColor"));
			if (ngSongs > 0) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("newgroundsColor"));
			if (musicLibrarySongs > 0) addColorToSequence(arrayOfSequences, Utils::getColorAlpha("musicLibraryColor"));
			CCActionInterval* sequence = CCSequence::create(arrayOfSequences);
			if (!sequence) return log::info("could not create sequence for songLabel");
			CCAction* repeat = CCRepeatForever::create(sequence);
			if (!repeat) return log::info("unable to create repeat for songLabel node");
			songLabel->runAction(repeat);
		}
	}
	void applyLevelDescriptions(CCLayer* mainLayer) {
		if (!Utils::modEnabled() || !Utils::getBool("levelDescriptions") || m_level->m_levelType == GJLevelType::Editor) return;
		CCNode* viewButton = mainLayer->getChildByIDRecursive("view-button");
		if (!viewButton) return;
		CCSprite* infoButton = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
		infoButton->setScale(getInfoButtonScale());
		if (Utils::getDouble("levelDescriptionScale") > .6f) infoButton->setScale(.6f);
		CCMenuItemSpriteExtra* descButton = CCMenuItemSpriteExtra::create(infoButton, this, menu_selector(MyLevelCell::onShowLevelDesc));
		descButton->setID("level-desc-button"_spr);
		const std::string& buttonPosSetting = getInfoButtonLocation();
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
		if (!utils::string::endsWith(buttonPosSetting, " Level Cell") && !utils::string::endsWith(buttonPosSetting, " Button")) return;
		m_mainMenu->addChild(descButton);
	}
	void applyBlendingText() {
		if (!Utils::modEnabled() || !Utils::getBool("blendingText") || !m_mainLayer || m_fields->blendingApplied) return;
		for (CCNode* node : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
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
		if (CCLabelBMFont* localLevelName = typeinfo_cast<CCLabelBMFont*>(getChildByIDRecursive("level-name"))) localLevelName->limitLabelWidth(200.f, .6f, .01f);
		if (CCNode* mainLayer = getChildByIDRecursive("main-layer")) mainLayer->setPositionY(-3.f);
	}
	void determineLevelVisibility(GJGameLevel *level) {
		Manager* manager = Manager::getSharedInstance();
		if (!Utils::modEnabled() || level->m_userID.value() == manager->userID) return;
		const std::string& levelName = level->m_levelName;
		const int accountID = level->m_accountID.value();
		if (Utils::getBool("ignorePeople") && Utils::contains<int>(manager->ignoredUsers, accountID)) return MyLevelCell::hideLevel("by Ignored User");
		if (Utils::getBool("favoriteUsers") && Utils::contains<int>(manager->favoriteUsers, accountID)) return MyLevelCell::highlightLevel();
		if (Utils::getBool("personalFilter") && utils::string::containsAny(utils::string::toLower(levelName), manager->dislikedWords)) return MyLevelCell::hideLevel("Name has Disliked Word(s)");
	}
	void hideLevel(const std::string_view reason) {
		const bool levelIsRated = m_level->m_stars.value() != 0;
		if (!Utils::modEnabled() || (Utils::getBool("dontHideIfRated") && levelIsRated)) return;
		for (CCNode* node : CCArrayExt<CCNode*>(this->getChildren())) node->setVisible(false);
		CCMenu* menu = CCMenu::create();
		ButtonSprite* buttonSprite = ButtonSprite::create("Show", 56, 30, .75f, false, "bigFont.fnt", "GJ_button_04.png");
		CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(buttonSprite, this, menu_selector(MyLevelCell::unhideLevel));
		CCLabelBMFont* label = CCLabelBMFont::create(fmt::format("Hidden Level {}", reason).c_str(), "bigFont.fnt");

		menu->setScale(this->m_compactView ? .8f : .9f);
		menu->addChild(button);
		menu->ignoreAnchorPointForPosition(false);
		menu->setContentSize(button->getContentSize());
		menu->setPosition({this->m_compactView ? 318.85f : 314.85f, this->m_height / 2.f});

		button->setPosition(button->getContentSize() / 2.f);

		label->limitLabelWidth(270.f, 1.f, .001f);
		if (levelIsRated) {
			if (!m_level->isPlatformer()) label->setColor({240, 211, 42});
			else label->setColor({50, 200, 255});
		}
		label->setPosition({this->m_compactView ? 147.5f : 143.5f, menu->getPositionY()}); // why do i need a ternary for xpos? robtop sucks at ui!!!!

		menu->setID("unhide-level"_spr);
		label->setID("hidden-reason"_spr);
		button->setID("unhide-level-button"_spr);
		buttonSprite->setID("unhide-level-sprite"_spr);

		this->addChild(menu);
		this->addChild(label);
	}
	void unhideLevel(CCObject*) {
		this->removeChildByID("unhide-level"_spr);
		this->removeChildByID("hidden-reason"_spr);
		for (CCNode* node : CCArrayExt<CCNode*>(this->getChildren())) node->setVisible(true);
	}
	void highlightLevel() {
		if (!Utils::modEnabled() || !Utils::getBool("favoriteUsers")) return;
		const auto [r, g, b, a] = Utils::getColorAlpha("favoriteUserColor");
		CCLayerGradient* gradientHighlight = CCLayerGradient::create({r, g, b, a}, {r, g, b, 0});
		gradientHighlight->setContentSize(m_backgroundLayer->getContentSize());
		gradientHighlight->setPosition(m_backgroundLayer->getPosition());
		m_backgroundLayer->setZOrder(m_backgroundLayer->getZOrder() - 1);
		gradientHighlight->setZOrder(m_backgroundLayer->getZOrder() + 1);
		gradientHighlight->ignoreAnchorPointForPosition(true);
		gradientHighlight->setID("highlight"_spr);
		this->addChild(gradientHighlight);
	}
	void loadCustomLevelCell() {
		LevelCell::loadCustomLevelCell();
		GJGameLevel* level = m_level;
		if (!Utils::modEnabled() || !m_mainMenu || !level) return;
		MyLevelCell::determineLevelVisibility(level);
	}
	void loadFromLevel(GJGameLevel* level) {
		LevelCell::loadFromLevel(level);
		if (!Utils::modEnabled()) return;
		CCLayer* mainLayer = this->m_mainLayer;
		if (!mainLayer || !m_level) return;
		if (Utils::getBool("recolorSongLabels")) MyLevelCell::applySongRecoloring(mainLayer, m_level);
		if (Utils::getBool("recolorLevelNameFeaturedScore")) MyLevelCell::applyFeatureStateRecoloring(mainLayer);
		if (Utils::getBool("levelDescriptions") && level->m_levelType != GJLevelType::Editor) MyLevelCell::applyLevelDescriptions(mainLayer);
	}
	void draw() {
		LevelCell::draw();
		if (!Utils::modEnabled() || !Utils::getBool("blendingText") || m_fields->blendingApplied) return;
		MyLevelCell::applyBlendingText();
	}
};

class $modify(MyLevelListLayer, LevelListLayer) {
	struct Fields {
		bool alreadyMoved = false;
	};
	static void removePlacement(const LevelCell* levelCell) {
		if (!Utils::modEnabled() || !levelCell->m_mainMenu || !levelCell->m_mainLayer || levelCell->m_level->m_listPosition == 0) return;
		if (CCNode* descButton = levelCell->m_mainMenu->getChildByID("level-desc-button"_spr)) descButton->setPositionY(descButton->getPositionY() - 40.f);
		if (!Utils::getBool("removePlacement")) return;
		// consent to reuse code found here: https://discord.com/channels/911701438269386882/911702535373475870/1333235345365532784
		if (CCNode* placementLabel = levelCell->m_mainLayer->getChildByID("level-place")) placementLabel->setVisible(false);
		for (CCNode* child : CCArrayExt<CCNode*>(levelCell->m_mainLayer->getChildren())) {
			if (const std::string& childID = child->getID(); childID == "main-menu" || utils::string::startsWith(childID, ""_spr)) continue;
			if (const auto label = typeinfo_cast<CCLabelBMFont*>(child); label && static_cast<std::string>(label->getFntFile()) == "chatFont.fnt") continue;
			child->setPositionX(child->getPositionX() - LEVEL_PLACEMENT_OFFSET);
		}
		for (CCNode* child : CCArrayExt<CCNode*>(levelCell->m_mainMenu->getChildren())) {
			if (const std::string& childID = child->getID(); childID == "view-button" || utils::string::startsWith(childID, ""_spr)) continue;
			child->setPositionX(child->getPositionX() - LEVEL_PLACEMENT_OFFSET);
		}
		if (levelCell->m_mainMenu->getChildByID("view-button")) {
			if (CCNode* node = levelCell->m_mainLayer->getChildByID("completed-icon")) node->setPosition({276.f, 25.f});
			if (CCNode* node = levelCell->m_mainLayer->getChildByID("percentage-label")) node->setPosition({276.f, 25.f});
		}
	}
	// need to hook this for when downloading a list for the first time
	void loadLevelsFinished(cocos2d::CCArray* p0, char const* p1, int p2) {
		LevelListLayer::loadLevelsFinished(p0, p1, p2);
		if (!Utils::modEnabled() || m_levelList->m_listType == GJLevelType::Editor) return;
		if (!m_list || !m_list->m_listView || !m_list->m_listView->m_tableView || !m_list->m_listView->m_tableView->m_cellArray || !typeinfo_cast<LevelCell*>(m_list->m_listView->m_tableView->m_cellArray->objectAtIndex(0))) return log::info("could not find the place where level cell entries are stored");
		for (const LevelCell* levelCell : CCArrayExt<LevelCell*>(m_list->m_listView->m_tableView->m_cellArray)) MyLevelListLayer::removePlacement(levelCell);
	}
	// need to hook this when exiting the LevelInfoLayer. also the m_fields access prevents moving the nodes more than once between level entry/exiting
	void onEnter() {
		LevelListLayer::onEnter();
		if (!Utils::modEnabled() || m_levelList->m_listType == GJLevelType::Editor || m_fields->alreadyMoved) return;
		if (!m_list || !m_list->m_listView || !m_list->m_listView->m_tableView || !m_list->m_listView->m_tableView->m_cellArray || !typeinfo_cast<LevelCell*>(m_list->m_listView->m_tableView->m_cellArray->objectAtIndex(0))) return log::info("could not find the place where level cell entries are stored");
		m_fields->alreadyMoved = true;
		for (const LevelCell* levelCell : CCArrayExt<LevelCell*>(m_list->m_listView->m_tableView->m_cellArray)) MyLevelListLayer::removePlacement(levelCell);
	}
};