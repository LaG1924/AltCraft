#pragma once

#include <string>
#include <functional>

#include "Vector.hpp"
#include "Platform.hpp"

class BlockInfo;

struct Plugin {
	int errors;
	const std::string modid;
	std::function<void()> onLoad;
	std::function<void()> onUnload;
	std::function<void(std::string)> onChangeState;
	std::function<void(double)> onTick;
};

namespace PluginSystem {
	AC_API void RegisterPlugin(Plugin &plugin);

	AC_INTERNAL void Init() noexcept;
	AC_INTERNAL void Deinit() noexcept;

	void Execute(const std::string &luaCode, bool except = false);

	void CallOnChangeState(std::string newState);

	void CallOnTick(double deltaTime);
}
