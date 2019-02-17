#include "Plugin.hpp"

#include <vector>

#include <easylogging++.h>
#include <sol.hpp>

struct Plugin {
	const std::string name;
	const std::string displayName;
	const std::function<void()> onLoad;
	const std::function<void()> onUnload;
	const std::function<void(std::string)> onChangeState;
};


std::vector<Plugin> plugins;
sol::state lua;


namespace PluginApi {

	void RegisterPlugin(sol::table &self, sol::table &plugin) {
		Plugin nativePlugin {
				plugin["name"].get_or<std::string>("75"),
				plugin["displayName"].get_or<std::string>(""),
				plugin["onLoad"].get_or(std::function<void()>()),
				plugin["onUnload"].get_or(std::function<void()>()),
				plugin["onChangeState"].get_or(std::function<void(std::string)>())
		};
		plugins.push_back(nativePlugin);
		nativePlugin.onLoad();
		
		LOG(INFO) << "Loaded plugin " << (!nativePlugin.displayName.empty() ? nativePlugin.displayName : nativePlugin.name);
	}

	void LogWarning(sol::table &self, std::string text) {
		LOG(WARNING) << text;
	}

}

void PluginSystem::Init()
{
	LOG(INFO) << "Initializing plugin system";
	for (Plugin &plugin : plugins) {
		if (plugin.onUnload)
			plugin.onUnload();
	}

	plugins.clear();
	lua = sol::state();
	lua.open_libraries();

	sol::table apiTable = lua["AC"].get_or_create<sol::table>();

	apiTable["RegisterPlugin"] = PluginApi::RegisterPlugin;
	apiTable["LogWarning"] = PluginApi::LogWarning;
}

void PluginSystem::Execute(const std::string &luaCode)
{
	try {
		lua.safe_script(luaCode);
	} catch (sol::error &e) {
		LOG(ERROR) << e.what();
	}
}

void PluginSystem::CallOnChangeState(std::string newState)
{
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
