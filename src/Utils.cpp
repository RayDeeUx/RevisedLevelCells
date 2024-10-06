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
}