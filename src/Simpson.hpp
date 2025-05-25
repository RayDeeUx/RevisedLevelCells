#pragma once

using namespace geode::prelude;

class Simpson : public UserListDelegate {

protected:
	static Simpson* instance;
public:

	static Simpson* get() {
		if (!instance) instance = new Simpson();
		return instance;
	}

	void getUserListFinished(cocos2d::CCArray* p0, UserListType p1);
	void getUserListFailed(UserListType p0, GJErrorCode p1);

	// while this is valid destructor code it will never run
	// ty dankmeme01
	/*
	~Simpson() {
		if (GameLevelManager::get()->m_userListDelegate == this)
			GameLevelManager::get()->m_userListDelegate = nullptr;
	}
	*/

};