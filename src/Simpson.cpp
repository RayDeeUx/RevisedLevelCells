#include "Simpson.hpp"
#include "Manager.hpp"
#include "Utils.hpp"

Simpson* Simpson::instance = nullptr;

void Simpson::getUserListFinished(cocos2d::CCArray* p0, UserListType p1) {
	GameLevelManager* glm = GameLevelManager::get();
	Manager* manager = Manager::getSharedInstance();
	const bool fetchedBlocked = p1 == UserListType::Blocked;
	glm->m_userListDelegate = nullptr;
	std::vector<int>& toInsert = fetchedBlocked ? manager->ignoredUsers : manager->favoriteUsers;
	std::vector<int>& toInsertTwo = fetchedBlocked ? manager->blocked : manager->friends;
	log::info("——— editing {} users using {} from GD ———", fetchedBlocked ? "ignored users" : "favorite users", fetchedBlocked ? "blocked list" : "friends list");
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
	if (fetchedBlocked || !Utils::getBool("blockedAreIgnoredPeople")) return log::info("either blocked people have been fetched, or user chose not to fetch blocked people. who knows? oh, right: fetchedBlocked: {}, !Utils::getBool(\"blockedAreIgnoredPeople\"): {}", fetchedBlocked, !Utils::getBool("blockedAreIgnoredPeople"));
	glm->m_userListDelegate = Simpson::get();
	log::info("finished fetching friends, now fetching UserListType::Blocked");
	glm->getUserList(UserListType::Blocked);
}

void Simpson::getUserListFailed(UserListType p0, GJErrorCode p1) {
	GameLevelManager::get()->m_userListDelegate = nullptr;
	if (p1 == GJErrorCode::UpdateApp) return;
	log::info("Failed to fetch {}", p0 == UserListType::Blocked ? "blocked users" : "friends");
}