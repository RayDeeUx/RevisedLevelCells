#include <Geode/modify/LikeItemLayer.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

#define PREFERRED_HOOK_PRIO (-3999)

using namespace geode::prelude;

class $modify(MyLikeItemLayer, LikeItemLayer) {
	struct Fields {
		int itemID = 0;
		bool isList = false;
	};
	static void onModify(auto & self) {
		(void) self.setHookPriority("LikeItemLayer::init", PREFERRED_HOOK_PRIO);
	}
	void onIgnoreUser(CCObject*) {
		MyLikeItemLayer::handleAction();
	}
	void onFavoriteUser(CCObject*) {
		MyLikeItemLayer::handleAction(true);
	}
	void handleAction(const bool isFavorite = false) {
		int accountIDTarget = -1;
		std::string username = "FOOBARBAZ"_spr;

		Fields* fields = m_fields.self();
		CCScene* scene = CCScene::get();
		if (!scene) return log::info("scene was not found? wild imo");

		if (fields->isList) {
			if (const auto listLayer = typeinfo_cast<LevelListLayer*>(scene->getChildByID("LevelListLayer")); listLayer && listLayer->m_levelList) {
				 accountIDTarget = listLayer->m_levelList->m_accountID;
				 username = listLayer->m_levelList->m_creatorName;
			} else return log::info("tried to ignore the author of a list, but could not find the information. the ID of the list was {}", fields->itemID);
		} else {
			if (const auto infoLayer = typeinfo_cast<LevelInfoLayer*>(scene->getChildByID("LevelInfoLayer")); infoLayer && infoLayer->m_level) {
				 accountIDTarget = infoLayer->m_level->m_accountID.value();
				 username = infoLayer->m_level->m_creatorName;
			} else return log::info("tried to ignore the author of a level, but could not find the information. the ID of the level was {}", fields->itemID);
		}

		if (accountIDTarget == -1 || username == "FOOBARBAZ"_spr) return;
		if (accountIDTarget < 1) return Notification::create("Oof! That's an unregistered user.")->show();
		if (utils::string::toLower(username) == Manager::getSharedInstance()->username) return Notification::create(fmt::format("Oof! You can't {} yourself.", isFavorite ? "favorite" : "ignore"))->show();
		if (!isFavorite && accountIDTarget == 71) return Notification::create("Nice try, but you can't ignore RobTop!")->show();

		if (isFavorite) {
			if (Utils::addFavoriteUser(accountIDTarget, username)) Utils::favoriteSuccess(username);
			return;
		}
		if (Utils::addIgnoredUser(accountIDTarget, username)) return Utils::ignoreSuccess(username);
	}
	bool init(LikeItemType type, int itemID, int p2) {
		if (!LikeItemLayer::init(type, itemID, p2)) return false;
		if (type != LikeItemType::Level && type != LikeItemType::LevelList) return true;
		if (!Utils::modEnabled() || (!Utils::getBool("applyToLists") && type == LikeItemType::LevelList) || !m_buttonMenu || !m_mainLayer || (!Utils::getBool("favoriteUsers") && !Utils::getBool("ignorePeople"))) return true;
		if (!CCScene::get()->getChildByID("LevelListLayer") && !CCScene::get()->getChildByID("LevelInfoLayer")) return true;

		Fields* fields = m_fields.self();
		fields->isList = type == LikeItemType::LevelList;
		fields->itemID = itemID;

		if (Utils::getBool("favoriteUsers") && !Utils::getBool("dontAddFavoriteButton"))
			Utils::createButton("GJ_starBtn_001.png", this, menu_selector(MyLikeItemLayer::onFavoriteUser), "favorite", m_buttonMenu, {-98.f, -55.f});
		if (Utils::getBool("ignorePeople") && !Utils::getBool("dontAddIgnoreButton"))
			Utils::createButton("accountBtn_blocked_001.png", this, menu_selector(MyLikeItemLayer::onIgnoreUser), "ignore", m_buttonMenu, {98.f, -55.f});
		return true; // T0D0: InfoLayer hook, add to main-menu node using manual positioning
	}
};