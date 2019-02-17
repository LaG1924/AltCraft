#pragma once

#include <string>

namespace PluginSystem {
	void Init();

	void Execute(const std::string &luaCode);

	void CallOnChangeState(std::string newState);
}