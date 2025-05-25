#include "Simpson.hpp"
#include "Manager.hpp"
#include "Utils.hpp"

Simpson* Simpson::instance = nullptr;

void Simpson::getUserListFinished(cocos2d::CCArray* p0, UserListType p1) {
	GameLevelManager::get()->m_userListDelegate = nullptr;
	std::vector<int>& toInsert = p1 == UserListType::Blocked ? manager->ignoredUsers : manager->favoriteUsers;
	std::vector<int>& toInsertTwo = p1 == UserListType::Blocked ? manager->blocked : manager->friends;
	log::info("——— editing {} users using {} from GD ———", p1 == UserListType::Blocked ? "ignored users" : "favorite users", p1 == UserListType::Blocked ? "blocked users" : "friends list");
	for (GJUserScore* person : CCArrayExt<GJUserScore*>(p0)) {
		const int id = person->m_accountID;
		if (Utils::contains<int>(toInsert, id)) {
			log::info("user ID {} was in existing vector in Manager class, skipping", id);
			continue;
		}
		log::info("user ID {} was not in existing vector in Manager class, inserting it now", id);
		toInsert.push_back(id);
		toInsertTwo.push_back(id);
	}
}

void Simpson::getUserListFailed(UserListType p0, GJErrorCode p1) {
	GameLevelManager::get()->m_userListDelegate = nullptr;
	if (p1 == GJErrorCode::UpdateApp) return;
	log::info("Failed to fetch {}", p0 == UserListType::Blocked ? "blocked users" : "friends");
}