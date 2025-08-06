#pragma once
#include <Geode/loader/SettingV3.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include "Utils.hpp"

using namespace geode::prelude;

class MyButtonSettingV3 : public SettingV3 {
public:
	static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
		auto res = std::make_shared<MyButtonSettingV3>();
		auto root = checkJson(json, "MyButtonSettingV3");
		res->init(key, modID, root);
		res->parseNameAndDescription(root);
		res->parseEnableIf(root);
		return root.ok(std::static_pointer_cast<SettingV3>(res));
	}
	bool load(matjson::Value const& json) override {
		return true;
	}
	bool save(matjson::Value& json) const override {
		return true;
	}
	bool isDefaultValue() const override {
		return true;
	}
	void reset() override {}
	// SettingNodeV3* createNode(float width) override; // this line is [absolutely] certified cringe
	SettingNodeV3 *createNode(float width) override; // this line is NOT certified cringe
};

class MyButtonSettingNodeV3 : public SettingNodeV3 {
private:
	std::string m_title = "";
	std::string m_desc = "";
public:
	void onCommit() {}
	void onResetToDefault() {}
	bool hasUncommittedChanges() const {
		return false;
	}
	bool hasNonDefaultValue() const {
		return false;
	}
	void onSettingsButton(CCObject*) {
		if (m_title == "Refresh Personal Phrases Filter + Favorite/Ignored Users") {
			if (Utils::updateLists()) return FLAlertLayer::create("Success!", "You've successfully <cg>refreshed</c> your <co>personal phrases filter</c> and <co>favorited/ignored users list</c>.", "Close")->show();
			return FLAlertLayer::create("Hmm...", "Something went wrong when trying to <cg>refresh</c> your <co>personal phrases filter</c> and <co>favorited/ignored users list</c>.", "Oof...")->show();
		}
		file::openFolder(Mod::get()->getConfigDir());
		#ifndef GEODE_IS_MOBILE
		if (!CCKeyboardDispatcher::get()->getShiftKeyPressed()) return;
		FLAlertLayer::create("Config Directory Opened!", m_desc, "Aight")->show();
		#endif
	}
	bool init(std::shared_ptr<MyButtonSettingV3> setting, float width) {
		if (!SettingNodeV3::init(setting, width)) return false;
		this->setContentSize({ width, 40.f });
		m_title = setting->getName().value();
		m_desc = setting->getDescription().value();
		auto theMenu = CCMenu::create();
		auto theLabel = CCLabelBMFont::create(m_title.c_str(), "bigFont.fnt");

		// copied a bit from louis ck jr
		theLabel->setScale(.35f);
		theLabel->limitLabelWidth(250.f, .35f, .25f); // added by Ery. max width is 346.f

		auto theLabelAsAButton = CCMenuItemSpriteExtra::create(theLabel, this, menu_selector(MyButtonSettingNodeV3::onSettingsButton));

		theLabelAsAButton->setPositionX(0);
		theLabelAsAButton->setID(fmt::format("{}-shortcut-button"_spr, setting->getKey()));

		theMenu->addChild(theLabelAsAButton);
		theMenu->setPosition(width / 2, 20.f);
		theMenu->setID(fmt::format("{}-shortcut-menu"_spr, setting->getKey()));

		this->addChild(theMenu);

		for (CCNode* node : CCArrayExt<CCNode*>(this->getChildren())) {
			node->setVisible(!node->getID().empty() && utils::string::startsWith(node->getID(), Mod::get()->getID()));
		}

		return true;
	}
	static MyButtonSettingNodeV3* create(std::shared_ptr<MyButtonSettingV3> setting, float width) {
		auto ret = new MyButtonSettingNodeV3();
		if (ret && ret->init(setting, width)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
};