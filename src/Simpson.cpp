#include "Simpson.hpp"
#include "Manager.hpp"
#include "Utils.hpp"

Simpson* Simpson::instance = nullptr;

void Simpson::getUserListFinished(cocos2d::CCArray* p0, UserListType p1) {
	GameLevelManager::get()->m_userListDelegate = nullptr;
	Manager* manager = Manager::getSharedInstance();
	std::vector<int>& toInsert = p1 == UserListType::Blocked ? manager->ignoredUsers : manager->favoriteUsers;
	std::vector<int>& toInsertTwo = p1 == UserListType::Blocked ? manager->blocked : manager->friends;
	for (const auto person : CCArrayExt<GJUserScore*>(p0)) {
		int id = person->m_accountID;
		if (Utils::contains<int>(toInsert, id)) continue;
		toInsert.push_back(id);
		toInsertTwo.push_back(id);
	}
}

void Simpson::getUserListFailed(UserListType p0, GJErrorCode p1) {
	GameLevelManager::get()->m_userListDelegate = nullptr;
	if (p1 == GJErrorCode::UpdateApp) return;
	log::info("Failed to fetch {}", p0 == UserListType::Blocked ? "blocked users" : "friends");
}