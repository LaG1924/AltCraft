#include "Plugin.hpp"

#include <vector>

#include <easylogging++.h>
#include <sol.hpp>
#include <optick.h>

#include "GameState.hpp"
#include "Game.hpp"


struct Plugin {
	const std::string name;
	const std::string displayName;
	const std::function<void()> onLoad;
	const std::function<void()> onUnload;
	const std::function<void(std::string)> onChangeState;
	const std::function<void(double)> onTick;
};


std::vector<Plugin> plugins;
sol::state lua;


namespace PluginApi {

	void RegisterPlugin(sol::table plugin) {
		Plugin nativePlugin {
				plugin["name"].get_or<std::string>(""),
				plugin["displayName"].get_or<std::string>(""),
				plugin["onLoad"].get_or(std::function<void()>()),
				plugin["onUnload"].get_or(std::function<void()>()),
				plugin["onChangeState"].get_or(std::function<void(std::string)>()),
				plugin["onTick"].get_or(std::function<void(double)>())
		};
		plugins.push_back(nativePlugin);
		nativePlugin.onLoad();
		
		LOG(INFO) << "Loaded plugin " << (!nativePlugin.displayName.empty() ? nativePlugin.displayName : nativePlugin.name);
	}

	void LogWarning(std::string text) {
		LOG(WARNING) << text;
	}

	void LogInfo(std::string text) {
		LOG(INFO) << text;
	}

	void LogError(std::string text) {
		LOG(ERROR) << text;
	}

	GameState *GetGameState() {
		return ::GetGameState();
	}
}

void PluginSystem::Init() {
	OPTICK_EVENT();
	LOG(INFO) << "Initializing plugin system";
	for (Plugin &plugin : plugins) {
		if (plugin.onUnload)
			plugin.onUnload();
	}

	plugins.clear();
	lua = sol::state();
	lua.open_libraries();

	lua.new_usertype<Entity>("Entity",
		"pos", &Entity::pos);

	lua.new_usertype<GameState>("GameState",
		"GetPlayer", &GameState::GetPlayer,
		"GetWorld", &GameState::GetWorld);

	lua.new_usertype<World>("World");

	lua.new_usertype<Vector>("Vector",
		"x", &Vector::x,
		"y", &Vector::y,
		"z", &Vector::z);

	lua.new_usertype<VectorF>("VectorF",
		"x", &VectorF::x,
		"y", &VectorF::y,
		"z", &VectorF::z);

	sol::table apiTable = lua["AC"].get_or_create<sol::table>();

	apiTable["RegisterPlugin"] = PluginApi::RegisterPlugin;
	apiTable["LogWarning"] = PluginApi::LogWarning;
	apiTable["LogInfo"] = PluginApi::LogInfo;
	apiTable["LogError"] = PluginApi::LogError;
	apiTable["GetGameState"] = PluginApi::GetGameState;
}

void PluginSystem::Execute(const std::string &luaCode, bool except) {
	OPTICK_EVENT();
	try {
		lua.safe_script(luaCode);
	} catch (sol::error &e) {
		LOG(ERROR) << e.what();
		if (except)
			throw;
	}
}

void PluginSystem::CallOnChangeState(std::string newState) {
	OPTICK_EVENT();
	for (Plugin &plugin : plugins) {
		if (plugin.onChangeState)
			try {
				plugin.onChangeState(newState);
			}
			catch (sol::error &e) {
				LOG(ERROR) << e.what();
			}
	}
}

void PluginSystem::CallOnTick(double deltaTime) {
	OPTICK_EVENT();
	for (Plugin& plugin : plugins) {
		if (plugin.onTick)
			try {
				plugin.onTick(deltaTime);
			}
			catch (sol::error &e) {
				LOG(ERROR) << e.what();
			}
	}
}
