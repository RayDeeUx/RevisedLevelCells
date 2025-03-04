#include "Settings.hpp"

SettingNodeV3* MyButtonSettingV3::createNode(float width) {
	return MyButtonSettingNodeV3::create(
		std::static_pointer_cast<MyButtonSettingV3>(shared_from_this()),
		width
	);
}

// i had to include this cpp file because C++ is a legendary language!