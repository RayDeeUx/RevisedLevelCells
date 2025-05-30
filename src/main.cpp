#include <Geode/modify/MenuLayer.hpp>
#include "Settings.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	(void) Mod::get()->registerCustomSettingType("updatelists", &MyButtonSettingV3::parse);
	(void) Utils::updateLists();
	Mod::get()->setLoggingEnabled(Utils::getBool("logging"));
	listenForSettingChanges("logging", [](bool logging){
		Mod::get()->setLoggingEnabled(logging);
	});
}

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		if (Manager::getSharedInstance()->calledAlready) return true;
		const auto manager = Manager::getSharedInstance();
		manager->calledAlready = true;

		GameManager* gm = GameManager::get();
		manager->userID = gm->m_playerUserID.value();
		manager->username = utils::string::toLower(gm->m_playerName);

		return true;
	}
};