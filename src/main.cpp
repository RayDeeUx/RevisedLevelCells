#include "Settings.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
	(void) Mod::get()->registerCustomSettingType("configdir", &MyButtonSettingV3::parse);
	(void) Mod::get()->registerCustomSettingType("updatelists", &MyButtonSettingV3::parse);
	(void) Utils::updateLists();
}