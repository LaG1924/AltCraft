#pragma once

#include <string>

namespace PluginSystem {
	void Init();

	void Execute(const std::string &luaCode, bool except = false);

	void CallOnChangeState(std::string newState);

	void CallOnTick(double deltaTime);
}