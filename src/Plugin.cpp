#include "Plugin.hpp"
#include "ModLoader.hpp"

#include <vector>

#include <easylogging++.h>
#include <sol.hpp>
#include <optick.h>

#include "GameState.hpp"
#include "Game.hpp"
#include "Event.hpp"
#include "AssetManager.hpp"
#include "Platform.hpp"


static std::vector<Plugin> plugins;
static sol::state lua;


namespace PluginApi {
	AC_API void LogWarning(std::string text) {
		LOG(WARNING) << text;
	}

	AC_API void LogInfo(std::string text) {
		LOG(INFO) << text;
	}

	AC_API void LogError(std::string text) {
		LOG(ERROR) << text;
	}

	AC_API GameState *GetGameState() {
		return ::GetGameState();
	}

	AC_API void RegisterBlock(BlockId blockId, bool collides, std::string blockstate, std::string variant) {
		RegisterStaticBlockInfo(blockId, BlockInfo{
			collides,
			blockstate,
			variant
			});
	}
}

namespace LuaWrappers {
	static void RegisterPlugin(sol::table plugin) {
		Plugin nativePlugin {
				0,
				plugin["modid"].get_or<std::string>(""),
				plugin["onLoad"].get_or(std::function<void()>()),
				plugin["onUnload"].get_or(std::function<void()>()),
				plugin["onChangeState"].get_or(std::function<void(std::string)>()),
				plugin["onTick"].get_or(std::function<void(double)>())
		};
		PluginSystem::RegisterPlugin(nativePlugin);
	}
}

int LoadFileRequire(lua_State* L) {
	std::string path = sol::stack::get<std::string>(L);

	std::string package = path.substr(0, path.find('/'));
	std::string script = path.substr(path.find('/') + 1);

	std::string scriptPath = "/" + package + "/code/lua/" + script;

	AssetScript *asset = AssetManager::GetAsset<AssetScript>(scriptPath);
	if (!asset) {
		sol::stack::push(L, "Module '" + scriptPath + "' not found");
		return 1;
	}

	luaL_loadbuffer(L, asset->code.data(), asset->code.size(), path.c_str());
	return 1;
}

void PluginSystem::RegisterPlugin(Plugin &plugin) {
	plugins.push_back(plugin);
	plugin.onLoad();

	std::shared_ptr<ModLoader::Mod> modinfo=ModLoader::GetModByModid(plugin.modid);
	LOG(INFO) << "Loaded plugin " << (modinfo ? modinfo->name : plugin.modid);
}

void PluginSystem::Init() noexcept {
	OPTICK_EVENT();
	LOG(INFO) << "Initializing plugin system";
	for (Plugin &plugin : plugins) {
		if (plugin.onUnload && plugin.errors < 10)
			plugin.onUnload();
	}

	plugins.clear();
	lua = sol::state();
	lua.open_libraries();

	lua["package"]["searchers"] = lua.create_table_with(
		1, LoadFileRequire
	);

	lua.new_usertype<Entity>("Entity",
		"pos", &Entity::pos);

	lua.new_usertype<GameState>("GameState",
		"GetPlayer", &GameState::GetPlayer,
		"GetWorld", &GameState::GetWorld,
		"GetTimeStatus", &GameState::GetTimeStatus,
		"GetGameStatus", &GameState::GetGameStatus,
		"GetPlayerStatus", &GameState::GetPlayerStatus,
		"GetSelectionStatus", &GameState::GetSelectionStatus,
		"GetInventory", &GameState::GetInventory);

	lua.new_usertype<TimeStatus>("TimeStatus",
		"interpolatedTimeOfDay", &TimeStatus::interpolatedTimeOfDay,
		"worldAge", &TimeStatus::worldAge,
		"timeOfDay", &TimeStatus::timeOfDay,
		"doDaylightCycle", &TimeStatus::doDaylightCycle);

	lua.new_usertype<GameStatus>("GameStatus",
		"levelType", &GameStatus::levelType,
		"spawnPosition", &GameStatus::spawnPosition,
		"gamemode", &GameStatus::gamemode,
		"dimension", &GameStatus::dimension,
		"difficulty", &GameStatus::difficulty,
		"maxPlayers", &GameStatus::maxPlayers,
		"isGameStarted", &GameStatus::isGameStarted,
		"reducedDebugInfo", &GameStatus::reducedDebugInfo);

	lua.new_usertype<SelectionStatus>("SelectionStatus",
		"raycastHit", &SelectionStatus::raycastHit,
		"selectedBlock", &SelectionStatus::selectedBlock,
		"distanceToSelectedBlock", &SelectionStatus::distanceToSelectedBlock,
		"isBlockSelected", &SelectionStatus::isBlockSelected);

	lua.new_usertype<PlayerStatus>("PlayerStatus",
		"uid", &PlayerStatus::uid,
		"name", &PlayerStatus::name,
		"flyingSpeed", &PlayerStatus::flyingSpeed,
		"fovModifier", &PlayerStatus::fovModifier,
		"health", &PlayerStatus::health,
		"eid", &PlayerStatus::eid,
		"invulnerable", &PlayerStatus::invulnerable,
		"flying", &PlayerStatus::flying,
		"allowFlying", &PlayerStatus::allowFlying,
		"creativeMode", &PlayerStatus::creativeMode);

	lua.new_usertype<World>("World",
		"GetEntitiesList", &World::GetEntitiesList,
		"GetEntity",&World::GetEntityPtr,
		"Raycast", &World::Raycast,
		"GetBlockId", &World::GetBlockId,
		"SetBlockId", &World::SetBlockId);

	lua.new_usertype<BlockId>("BlockId",
		"new", sol::factories([]() {return BlockId{ 0,0 };},
			[](unsigned short id, unsigned char state) {return BlockId{ id, state };}),
		"id", sol::property(
			[](BlockId & bid) { return bid.id; },
			[](BlockId & bid, unsigned short id) { bid.id = id; }),
		"state", sol::property(
			[](BlockId & bid) { return bid.state; },
			[](BlockId & bid, unsigned char state) { bid.state = state; }));

	lua.new_usertype<Vector>("Vector",
		sol::constructors<Vector(),Vector(long long, long long, long long)>(),
		"x", &Vector::x,
		"y", &Vector::y,
		"z", &Vector::z);

	lua.new_usertype<VectorF>("VectorF",
		sol::constructors<VectorF(), VectorF(double, double, double)>(),
		"x", &VectorF::x,
		"y", &VectorF::y,
		"z", &VectorF::z);

	lua.new_usertype<BlockInfo>("BlockInfo",
		"collides", &BlockInfo::collides,
		"blockstate", &BlockInfo::blockstate,
		"variant", &BlockInfo::variant);

	lua.new_usertype<Dimension>("Dimension",
		"new", sol::factories([]() {return Dimension{ nullptr, 0 }; },
			[](std::string dimName, bool skylight) {return Dimension{ dimName, skylight }; }),
		"name", &Dimension::name,
		"skylight", &Dimension::skylight);

	lua.new_usertype<Biome>("Biome",
		"new", sol::factories([]() {return Biome{ nullptr, 0, 0 }; },
			[](std::string bioName, float temperature, float rainfall) {return Biome{ bioName, temperature, rainfall }; }),
		"name", &Biome::name,
		"temperature", &Biome::temperature,
		"rainfall", &Biome::rainfall);

	sol::table apiTable = lua["AC"].get_or_create<sol::table>();

	apiTable["RegisterPlugin"] = LuaWrappers::RegisterPlugin;
	apiTable["LogWarning"] = PluginApi::LogWarning;
	apiTable["LogInfo"] = PluginApi::LogInfo;
	apiTable["LogError"] = PluginApi::LogError;
	apiTable["GetGameState"] = PluginApi::GetGameState;
	apiTable["RegisterBlock"] = PluginApi::RegisterBlock;
	apiTable["RegisterDimension"] = RegisterNewDimension;
	apiTable["RegisterBiome"] = RegisterNewBiome;
}

void PluginSystem::Deinit() noexcept {
	OPTICK_EVENT();
	LOG(INFO) << "Deinitializing plugin system";
	for (Plugin &plugin : plugins) {
		if (plugin.onUnload && plugin.errors < 10)
			plugin.onUnload();
	}

	plugins.clear();
	lua.~state();
}

void PluginSystem::Execute(const std::string &luaCode, bool except) {
	OPTICK_EVENT();
	try {
		lua.safe_script(luaCode);
	} catch (std::runtime_error &e) {
		LOG(ERROR) << e.what();
		if (except)
			throw;
	}
}

void PluginSystem::CallOnChangeState(std::string newState) {
	OPTICK_EVENT();
	for (Plugin &plugin : plugins) {
		if (plugin.onChangeState && plugin.errors < 10)
			try {
				plugin.onChangeState(newState);
			}
			catch (std::runtime_error &e) {
				LOG(ERROR) << e.what();
				plugin.errors++;
			}
	}
}

void PluginSystem::CallOnTick(double deltaTime) {
	OPTICK_EVENT();
	for (Plugin& plugin : plugins) {
		if (plugin.onTick && plugin.errors < 10)
			try {
				plugin.onTick(deltaTime);
			}
			catch (std::runtime_error &e) {
				LOG(ERROR) << e.what();
				plugin.errors++;
			}
	}
}
