#include <Geode/modify/LevelListCell.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

#define PREFERRED_HOOK_PRIO (-3999)

using namespace geode::prelude;

class $modify(MyLevelListCell, LevelListCell) {
	struct Fields {
		bool blendingApplied = false;
	};
	static void onModify(auto & self) {
		(void) self.setHookPriority("LevelListCell::draw", PREFERRED_HOOK_PRIO);
		(void) self.setHookPriority("LevelListCell::loadFromList", PREFERRED_HOOK_PRIO);
	}
	void applyBlendingText() {
		if (!Utils::modEnabled() || !Utils::getBool("applyToLists") || !Utils::getBool("blendingText") || !m_levelList || !m_mainLayer || m_fields->blendingApplied) return;
		for (const auto node : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
			if (node->getID() == "cvolton.level-id-api/level-id-label" && Manager::getSharedInstance()->username != utils::string::toLower(m_levelList->m_creatorName)) {
				// see https://discord.com/channels/911701438269386882/911702535373475870/1346625863130677370 for context
				node->setVisible(false);
				continue;
			}
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
	void hideLevel(const std::string_view reason) {
		const bool listIsRated = m_levelList->m_diamonds != 0;
		if (!Utils::modEnabled() || !Utils::getBool("applyToLists") || (Utils::getBool("dontHideIfRated") && listIsRated)) return;
		for (CCNode* node : CCArrayExt<CCNode*>(this->getChildren())) node->setVisible(false);
		CCMenu* menu = CCMenu::create();
		ButtonSprite* buttonSprite = ButtonSprite::create("Show", 56, 30, .75f, false, "bigFont.fnt", "GJ_button_04.png");
		CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(buttonSprite, this, menu_selector(MyLevelListCell::unhideLevel));
		CCLabelBMFont* label = CCLabelBMFont::create(fmt::format("Hidden Level List {}", reason).c_str(), "bigFont.fnt");

		menu->setScale(.9f);
		menu->addChild(button);
		menu->ignoreAnchorPointForPosition(false);
		menu->setContentSize(button->getContentSize());
		menu->setPosition({314.85f, this->m_height / 2.f});

		button->setPosition(button->getContentSize() / 2.f);

		label->limitLabelWidth(270.f, 1.f, .001f);
		if (listIsRated) label->setColor({50, 200, 255});
		label->setPosition({143.5f, menu->getPositionY()}); // why do i need a ternary for xpos? robtop sucks at ui!!!!

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
		if (!Utils::modEnabled() || !Utils::getBool("applyToLists") || !Utils::getBool("favoriteUsers")) return;
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
	void loadFromList(GJLevelList* list) {
		LevelListCell::loadFromList(list);
		Manager* manager = Manager::getSharedInstance();
		if (!Utils::modEnabled() || !Utils::getBool("applyToLists") || manager->username == utils::string::toLower(list->m_creatorName)) return;
		const std::string& levelName = list->m_listName;
		const int accountID = list->m_accountID;
		if (Utils::getBool("ignorePeople") && Utils::contains<int>(manager->ignoredUsers, accountID)) return MyLevelListCell::hideLevel("by Ignored User");
		if (Utils::getBool("favoriteUsers") && Utils::contains<int>(manager->favoriteUsers, accountID)) return MyLevelListCell::highlightLevel();
		if (Utils::getBool("personalFilter") && utils::string::containsAny(utils::string::toLower(levelName), manager->dislikedWords)) return MyLevelListCell::hideLevel("Name has Disliked Word(s)");
	}
	void draw() {
		LevelListCell::draw();
		if (!Utils::modEnabled() || !Utils::getBool("applyToLists") || !Utils::getBool("blendingText") || m_fields->blendingApplied) return;
		MyLevelListCell::applyBlendingText();
	}
};