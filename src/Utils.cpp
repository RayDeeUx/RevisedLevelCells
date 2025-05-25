#include "Utils.hpp"

using namespace geode::cocos;

namespace Utils {
	template<class T> T getSetting(const std::string& setting) { return Mod::get()->getSettingValue<T>(setting); }

	bool getBool(const std::string& setting) { return getSetting<bool>(setting); }
	
	int64_t getInt(const std::string& setting) { return getSetting<int64_t>(setting); }
	
	double getDouble(const std::string& setting) { return getSetting<double>(setting); }

	std::string getString(const std::string& setting) { return getSetting<std::string>(setting); }

	ccColor3B getColor(const std::string& setting) { return getSetting<ccColor3B>(setting); }

	ccColor4B getColorAlpha(const std::string& setting) { return getSetting<ccColor4B>(setting); }

	bool modEnabled() { return getBool("enabled"); }
	
	bool isModLoaded(const std::string& modID) { return Loader::get()->isModLoaded(modID); }

	Mod* getMod(const std::string& modID) { return Loader::get()->getLoadedMod(modID); }

	std::string getModVersion(Mod* mod) { return mod->getVersion().toNonVString(); }

	bool isSceneRunning(const std::string& sceneName) { return CCDirector::get()->getRunningScene()->getChildByID(sceneName); }

	bool doesNodeExist(const std::string& parentNodeName, const std::string& nodeName) { return CCDirector::get()->getRunningScene()->getChildByIDRecursive(parentNodeName) && CCDirector::get()->getRunningScene()->getChildByIDRecursive(parentNodeName)->getChildByIDRecursive(nodeName); }

	bool doesNodeExistNoParent(const std::string& nodeName) { return CCDirector::get()->getRunningScene()->getChildByIDRecursive(nodeName); }

	void writeToFile(const std::string_view fileName, int accountID, const std::string_view username) {
		std::ofstream output;
		output.open((Mod::get()->getConfigDir() / fileName), std::ios_base::app);
		output << std::endl << fmt::format("{} # [RLC] Username: {} [RLC] #", accountID, username);
		output.close();
	}

	bool addIgnoredUser(int accountID, std::string username) {
		/*
		if (accID == 71) return Notification::create("Nice try, but you can't ignore RobTop!")->show();
		if (accID <= 0) return Notification::create("Oof! That's an unregistered user.")->show();
		*/
		Manager* manager = Manager::getSharedInstance();
		if (contains<int>(manager->favoriteUsers, accountID)) {
			log::info("tried to ignore user: {} (username: {}) but they are already favorited", accountID, username);
			FLAlertLayer::create("Oops!", fmt::format("{} is already in your list of <cy>favorite</c> users.\nRevisit your mod settings if you believe this is a mistake.\n--RevisedLevelCells", username), "Close")->show();
			return false;
		}
		if (contains<int>(manager->ignoredUsers, accountID)) {
			log::info("tried to ingore user: {} (username: {}) but they are already ignored", accountID, username);
			Notification::create(fmt::format("{} is already ignored.", username))->show();
			return false;
		}
		log::info("ignoring user: {} (username: {})", accountID, username);
		manager->ignoredUsers.push_back(accountID);
		Utils::writeToFile("ignoredUsers.txt", accountID, username);
		return true;
	}

	bool addFavoriteUser(int accountID, std::string username) {
		Manager* manager = Manager::getSharedInstance();
		if (contains<int>(manager->ignoredUsers, accountID)) {
			log::info("tried to favorite user: {} (username: {}) but they are already ignored", accountID, username);
			FLAlertLayer::create("Oops!", fmt::format("{} is already in your list of <cr>ignored</c> users.\nRevisit your mod settings if you believe this is a mistake.\n--RevisedLevelCells", username), "Close")->show();
			return false;
		}
		if (contains<int>(manager->favoriteUsers, accountID)) {
			log::info("tried to favorite user: {} (username: {}) but they are already favorited (possibly from friends list)", accountID, username);
			Notification::create(fmt::format("{} is already a favorite user!", username))->show();
			return false;
		}
		log::info("favoriting user: {} (username: {})", accountID, username);
		manager->favoriteUsers.push_back(accountID);
		Utils::writeToFile("favoriteUsers.txt", accountID, username);
		return true;
	}

	bool updateLists(Manager* manager, Simpson* simpson) {
		manager->dislikedWords.clear();
		manager->ignoredUsers.clear();
		manager->favoriteUsers.clear();
		auto configDir = Mod::get()->getConfigDir();
		auto pathDislikedWords = (configDir / "dislikedWords.txt");
		auto pathIgnoredUsers = (configDir / "ignoredUsers.txt");
		auto pathFavoriteUsers = (configDir / "favoriteUsers.txt");
		if (!std::filesystem::exists(pathDislikedWords)) {
			std::string content = R"(# hello there
# this is the text file where you add phrases or words from level names you don't want to see while browsing levels
# separate phrases/words by new lines like you see in this file
# also, i didn't include any words in here by default as that would
# cause more confusion for you in the long run, let's be honest
# don't worry, the mod ignores all lines that start with "#"
# you will need to restart the game to apply your changes when editing this file
# all matches will be case insensitive and will ignore your own levels; keep that in mind! :)
# have fun!
# --raydeeux)";
			(void) utils::file::writeString(pathDislikedWords, content);
		} else {
			std::ifstream dislikedWordsFile(pathDislikedWords);
			std::string dislikedString;
			while (std::getline(dislikedWordsFile, dislikedString)) {
				if (dislikedString.starts_with('#') || dislikedString.empty()) continue;
				manager->dislikedWords.push_back(utils::string::toLower(dislikedString));
			}
		}
		if (!std::filesystem::exists(pathIgnoredUsers)) {
			std::string content = R"(# hello there
# this is the text file where you add user IDs of those whose levels/level lists you'd like to ignore
# separate user IDs by new lines like you see in this file
# also, i didn't include any user IDs in here by default as that would
# cause more confusion for you in the long run, let's be honest
# don't worry, the mod ignores all lines that start with "#" and aren't exclusively numbers
# you will need to restart the game to apply your changes when editing this file
# any crashes or bugs caused by improperly editing this file will be ignored
# have fun!
# --raydeeux)";
			(void) utils::file::writeString(pathIgnoredUsers, content);
		} else {
			std::ifstream ignoredUsersFile(pathIgnoredUsers);
			std::string ignoredUserString;
			while (std::getline(ignoredUsersFile, ignoredUserString)) {
				if (ignoredUserString.starts_with('#') || ignoredUserString.empty()) continue;
				std::string ignoredUserStringModified = ignoredUserString;
				if (ignoredUserStringModified.ends_with(" [RLC] #"))
					ignoredUserStringModified = ignoredUserStringModified.substr(0, ignoredUserStringModified.find(" # [RLC] Username: "));
				if (int ignoredUserID = utils::numFromString<int>(ignoredUserStringModified).unwrapOr(-2); ignoredUserID > 0 && ignoredUserID != 71)
					manager->ignoredUsers.push_back(ignoredUserID);
			}
		}
		if (!std::filesystem::exists(pathFavoriteUsers)) {
			std::string content = R"(# hello there
# this is the text file where you add user IDs of those whose levels/level lists you'd like to highlight
# separate user IDs by new lines like you see in this file
# also, i didn't include any user IDs in here by default as that would
# cause more confusion for you in the long run, let's be honest
# don't worry, the mod ignores all lines that start with "#" and aren't exclusively numbers
# you will need to restart the game to apply your changes when editing this file
# any crashes or bugs caused by improperly editing this file will be ignored
# have fun!
# --raydeeux)";
			(void) utils::file::writeString(pathFavoriteUsers, content);
		} else {
			std::ifstream favoriteUsersFile(pathFavoriteUsers);
			std::string favoriteUserString;
			while (std::getline(favoriteUsersFile, favoriteUserString)) {
				if (favoriteUserString.starts_with('#') || favoriteUserString.empty()) continue;
				std::string favoriteUserStringModified = favoriteUserString;
				if (favoriteUserStringModified.ends_with(" [RLC] #"))
					favoriteUserStringModified = favoriteUserStringModified.substr(0, favoriteUserStringModified.find(" # [RLC] Username: "));
				if (int favoriteUserID = utils::numFromString<int>(favoriteUserStringModified).unwrapOr(-2); favoriteUserID > 0)
					manager->favoriteUsers.push_back(favoriteUserID);
			}
		}
		if (!Utils::getBool("friendsAreFavoriteUsers") && !Utils::getBool("blockedAreIgnoredPeople")) return true;
		GameLevelManager* glm = GameLevelManager::get();
		if (!glm) {
			log::info("gamelevelmanager not found, oof!");
			return false;
		}
		if (Utils::getBool("friendsAreFavoriteUsers")) {
			glm->m_userListDelegate = simpson;
			log::info("fetching UserListType::Friends");
			glm->getUserList(UserListType::Friends);
		}
		if (Utils::getBool("blockedAreIgnoredPeople")) {
			glm->m_userListDelegate = simpson;
			log::info("fetching UserListType::Blocked");
			glm->getUserList(UserListType::Blocked);
		}
		return true;
	}

	void createButton(const std::string& spriteName, CCNode* target, const SEL_MenuHandler function, const std::string_view purpose, CCNode* parent, const CCPoint& position, const float scale) {
		CCSprite* sprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
		sprite->setID(fmt::format("{}-sprite"_spr, purpose));
		sprite->setScale(scale);
		CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(sprite, target, function);
		button->setID(fmt::format("{}-button"_spr, purpose));
		parent->addChild(button);
		button->setPosition(position);
	}

	void favoriteSuccess(const std::string_view username) {
		return Notification::create(fmt::format("{} is now a favorite user!", username))->show();
	}

	void ignoreSuccess(const std::string_view username) {
		if (Utils::getBool("dontHideIfRated")) return Notification::create(fmt::format("{} has been ignored.\nRated levels/lists from ignored users are still visible.", username))->show();
		return Notification::create(fmt::format("{} has been ignored.\nRated levels/lists from ignored users will also be hidden.", username))->show();
	}
}