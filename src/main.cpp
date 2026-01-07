#include <Geode/modify/MenuLayer.hpp>
#include "Settings.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
	Manager::getSharedInstance()->accountID = GJAccountManager::get()->m_accountID;
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	(void) Mod::get()->registerCustomSettingType("updatelists", &MyButtonSettingV3::parse);
	(void) Utils::updateLists();
	Mod::get()->setLoggingEnabled(Utils::getBool("logging"));
	listenForSettingChanges("logging", [](const bool logging){
		Mod::get()->setLoggingEnabled(logging);
	});
}

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		Manager* manager = Manager::getSharedInstance();
		if (manager->calledAlready) return true;
		manager->calledAlready = true;

		GameManager* gm = GameManager::get();
		manager->userID = gm->m_playerUserID.value();
		manager->username = utils::string::toLower(gm->m_playerName);

		return true;
	}
};