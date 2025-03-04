#pragma once

// Manager.hpp structure by acaruso
// reused with explicit permission and strong encouragement

using namespace geode::prelude;

class Manager {

protected:
	static Manager* instance;
public:

	bool calledAlready = false;

	std::vector<std::string> dislikedWords;
	std::vector<int> ignoredUsers;
	std::vector<int> favoriteUsers;

	int userID = -1;
	std::string username = "";

	static Manager* getSharedInstance() {
		if (!instance) instance = new Manager();
		return instance;
	}

};