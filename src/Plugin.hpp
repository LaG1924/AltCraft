#pragma once

#include <string>

#include "Vector.hpp"

class BlockInfo;
struct lua_State;
class Chat;

namespace PluginSystem {
	void Init();

	lua_State* GetLuaState();

	void Execute(const std::string &luaCode, bool except = false);

	void CallOnChangeState(std::string newState);

	void CallOnTick(double deltaTime);

	BlockInfo RequestBlockInfo(Vector blockPos);

	void CallOnChatMessage(const Chat& chat, int position);

	void CallOnDisconnected(const std::string &reason);
}