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
		std::string username = "FOOBARBAZ";

		const auto fields = m_fields.self();
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

		if (accountIDTarget == -1 || username == "FOOBARBAZ") return;
		if (accountIDTarget <= 0) return Notification::create("Oof! That's an unregistered user.")->show();
		if (utils::string::toLower(username) == Manager::getSharedInstance()->username) return Notification::create(fmt::format("Oof! You can't {} yourself.", isFavorite ? "favorite" : "ignore"))->show();
		if (!isFavorite && accountIDTarget == 71) return Notification::create("Nice try, but you can't ignore RobTop!")->show();

		if (isFavorite) {
			if (Utils::addFavoriteUser(accountIDTarget, username)) Notification::create(fmt::format("{} is now a favorite user!", username))->show();
			return;
		}
		if (Utils::addIgnoredUser(accountIDTarget, username)) return Notification::create(fmt::format("{} has been ignored.", username))->show();
	}
	bool init(LikeItemType type, int itemID, int p2) {
		if (!LikeItemLayer::init(type, itemID, p2)) return false;
		if (type != LikeItemType::Level && type != LikeItemType::LevelList) return true;
		if (!Utils::modEnabled() || (!Utils::getBool("applyToLists") && type == LikeItemType::LevelList) || !m_buttonMenu || !m_mainLayer) return true;
		if (!CCScene::get()->getChildByID("LevelListLayer") && !CCScene::get()->getChildByID("LevelInfoLayer")) return true;

		const auto fields = m_fields.self();
		fields->isList = type == LikeItemType::LevelList;
		fields->itemID = itemID;

		if (Utils::getBool("favoriteUsers")) {
			CCSprite* favoriteSprite = CCSprite::createWithSpriteFrameName("GJ_starBtn_001.png");
			favoriteSprite->setID("favorite-sprite"_spr);
			CCMenuItemSpriteExtra* favorite = CCMenuItemSpriteExtra::create(favoriteSprite, this, menu_selector(MyLikeItemLayer::onFavoriteUser));
			favorite->setID("favorite-button"_spr);
			m_buttonMenu->addChild(favorite);
			favorite->setPosition({-98.f, -55.f});
		}
		if (Utils::getBool("ignorePeople")) {
			CCSprite* ignoreSprite = CCSprite::createWithSpriteFrameName("accountBtn_blocked_001.png");
			ignoreSprite->setID("ignore-sprite"_spr);
			CCMenuItemSpriteExtra* ignore = CCMenuItemSpriteExtra::create(ignoreSprite, this, menu_selector(MyLikeItemLayer::onIgnoreUser));
			ignore->setID("ignore-button"_spr);
			m_buttonMenu->addChild(ignore);
			ignore->setPosition({98.f, -55.f});
		}
		return true; // T0D0: InfoLayer hook, add to main-menu node using manual positioning
	}
};