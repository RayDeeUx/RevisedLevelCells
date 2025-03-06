#include <Geode/modify/InfoLayer.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

#define PREFERRED_HOOK_PRIO (-3999)

using namespace geode::prelude;

class $modify(MyInfoLayer, InfoLayer) {
	bool init(GJGameLevel* level, GJUserScore* profile, GJLevelList* list) {
		if (!InfoLayer::init(level, profile, list)) return false;
		if (profile || !Utils::modEnabled() || (!Utils::getBool("applyToLists") && list) || !m_mainLayer || !m_buttonMenu || (!Utils::getBool("favoriteUsers") && !Utils::getBool("ignorePeople"))) return true;
		CCNode* infoButton = m_buttonMenu->getChildByID("info-button");
		CCNode* creatorButton = m_buttonMenu->getChildByID("creator-button");
		if (!infoButton || !creatorButton) return true;
		const bool favoriteUsers = Utils::getBool("favoriteUsers");
		const bool ignorePeople = Utils::getBool("ignorePeople");
		const float infoButtonY = infoButton->getPositionY();
		float xPosition = creatorButton->getPositionX() + (creatorButton->getContentWidth() / 1.5f) + 2.5f;
		if (favoriteUsers)
			Utils::createButton("GJ_starBtn_001.png", this, menu_selector(MyInfoLayer::onFavoriteUser), "favorite", m_buttonMenu, {xPosition, infoButtonY}, .5f);
		if (ignorePeople) {
			if (favoriteUsers) xPosition += infoButton->getContentWidth();
			Utils::createButton("accountBtn_blocked_001.png", this, menu_selector(MyInfoLayer::onIgnoreUser), "ignore", m_buttonMenu, {xPosition, infoButtonY}, .5f);
		}
		return true;
	}
	void onFavoriteUser(CCObject*) const {
		MyInfoLayer::handleAction(true);
	}
	void onIgnoreUser(CCObject*) const {
		MyInfoLayer::handleAction();
	}
	void handleAction(const bool isFavorite = false) const {
		int accountIDTarget = -1;
		std::string username = "FOOBARBAZ"_spr;

		if (m_levelList) {
			accountIDTarget = m_levelList->m_accountID;
			username = m_levelList->m_creatorName;
		} else if (m_level) {
			accountIDTarget = m_level->m_accountID.value();
			username = m_level->m_creatorName;
		} else return log::info("unable to fetch author information. m_levelList null: {}, m_level null: {}", m_levelList == nullptr, m_level == nullptr);

		if (accountIDTarget == -1 || username == "FOOBARBAZ"_spr) return;
		if (accountIDTarget <= 0) return Notification::create("Oof! That's an unregistered user.")->show();
		if (utils::string::toLower(username) == Manager::getSharedInstance()->username) return Notification::create(fmt::format("Oof! You can't {} yourself.", isFavorite ? "favorite" : "ignore"))->show();
		if (!isFavorite && accountIDTarget == 71) return Notification::create("Nice try, but you can't ignore RobTop!")->show();

		if (isFavorite) {
			if (Utils::addFavoriteUser(accountIDTarget, username)) Notification::create(fmt::format("{} is now a favorite user!", username))->show();
			return;
		}
		if (Utils::addIgnoredUser(accountIDTarget, username)) return Notification::create(fmt::format("{} has been ignored.", username))->show();
	}
};