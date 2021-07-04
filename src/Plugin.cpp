#include "Plugin.hpp"

#include <vector>

#include <easylogging++.h>
#include <sol/sol.hpp>
#include <optick.h>

#include "GameState.hpp"
#include "Game.hpp"
#include "Event.hpp"
#include "AssetManager.hpp"
#include "Settings.hpp"
#include "DebugInfo.hpp"
#include "Chat.hpp"


struct Plugin {
	int errors;
	const std::string name;
	const std::string displayName;
	const std::function<void()> onLoad;
	const std::function<void()> onUnload;
	const std::function<void(std::string)> onChangeState;
	const std::function<void(double)> onTick;
	const std::function<BlockInfo(Vector)> onRequestBlockInfo;
	const std::function<void(Chat, int)> onChatMessage;
};


std::vector<Plugin> plugins;
sol::state lua;


namespace PluginApi {

	void RegisterPlugin(sol::table plugin) {
		Plugin nativePlugin {
				0,
				plugin["name"].get_or<std::string>(""),
				plugin["displayName"].get_or<std::string>(""),
				plugin["onLoad"].get_or(std::function<void()>()),
				plugin["onUnload"].get_or(std::function<void()>()),
				plugin["onChangeState"].get_or(std::function<void(std::string)>()),
				plugin["onTick"].get_or(std::function<void(double)>()),
				plugin["onRequestBlockInfo"].get_or(std::function<BlockInfo(Vector)>()),
				plugin["onChatMessage"].get_or(std::function<void(Chat, int)>()),
		};
		plugins.push_back(nativePlugin);
		LOG(INFO)<<"Loading plugin " << (!nativePlugin.displayName.empty() ? nativePlugin.displayName : nativePlugin.name);
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

	void RegisterBlock(BlockId blockId, bool collides, std::string blockstate, std::string variant) {
		RegisterStaticBlockInfo(blockId, BlockInfo{
			collides,
			blockstate,
			variant
			});
	}

	void RegisterDimension(int dimId, Dimension dim) {
		RegisterNewDimension(dimId, dim);
	}

	void ConnectToServer(std::string host, std::string username) {
		size_t index = host.find_last_of(':');
		unsigned short port;
		if (index == std::string::npos)
			port = 25565;
		else {
			try {
				port = std::stoi(host.substr(index + 1));
			}
			catch (std::exception& e) {
				port = 25565;
				LOG(WARNING) << "Incorrect host format: " << host;
			}
		}
		PUSH_EVENT("ConnectToServer", std::make_tuple(host.substr(0, index), port, username));
	}

	void Exit() {
		PUSH_EVENT("Exit", 0);
	}

	void Disconnect() {
		PUSH_EVENT("Disconnect", std::string("Disconnected by user"));
	}

	void SetStatePlaying() {
		SetState(State::Playing);
	}

	void SettingsUpdate() {
		PUSH_EVENT("SettingsUpdate", 0);
	}

	int GetDebugValue(int valId) {
		switch (valId) {
		case 0:
			return DebugInfo::totalSections;
		case 1:
			return DebugInfo::renderSections;
		case 2:
			return DebugInfo::readyRenderer;
		case 3:
			return DebugInfo::gameThreadTime;
		case 4:
			return DebugInfo::renderFaces;
		case 5:
			return DebugInfo::culledSections;
		default:
			return 0;
		}
	}

	void SendChatMessage(const std::string& msg) {
		PUSH_EVENT("SendChatMessage", msg);
	}
}

int LoadFileRequire(lua_State* L) {
	std::string path = sol::stack::get<std::string>(L);

	std::string package = path.substr(0, path.find('/'));
	std::string script = path.substr(path.find('/') + 1);

	std::string scriptPath = "/" + package + "/scripts/" + script;

	AssetScript *asset = AssetManager::GetAsset<AssetScript>(scriptPath);
	if (!asset) {
		sol::stack::push(L, "Module '" + scriptPath + "' not found");
		return 1;
	}

	luaL_loadbuffer(L, asset->code.data(), asset->code.size(), path.c_str());
	return 1;
}

void PluginSystem::Init() {
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
		"GetInventory", &GameState::GetInventory,
		"PerformRespawn", &GameState::PerformRespawn);

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
		"GetBlockLight", sol::resolve<unsigned char(Vector)const>(&World::GetBlockLight),
		"GetBlockSkyLight", sol::resolve<unsigned char(Vector)const>(&World::GetBlockSkyLight),
		"GetBlockId", &World::GetBlockId,
		"SetBlockId", &World::SetBlockId);

	auto bidFactory1 = []() {
		return BlockId{ 0,0 };
	};
	auto bidFactory2 = [](unsigned short id, unsigned char state) {
		return BlockId{ id,state };
	};

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
		"new", sol::factories([]() {return Dimension{ 0,0 }; },
			[](std::string dimName, bool skylight) {return Dimension{ dimName, skylight }; }),
		"name", &Dimension::name,
		"skylight", &Dimension::skylight);

	lua.new_usertype<LoopExecutionTimeController>("LoopExecutionTimeController",
		"GetIterations", &LoopExecutionTimeController::GetIterations,
		"GetDeltaS", &LoopExecutionTimeController::GetDeltaS,
		"GetRealDeltaS", &LoopExecutionTimeController::GetRealDeltaS);

	lua.new_usertype<Chat>("Chat",
		"ToPlainText", &Chat::ToPlainText);

	sol::table apiTable = lua["AC"].get_or_create<sol::table>();
	sol::table apiSettings = lua["AC"]["Settings"].get_or_create<sol::table>();

	apiTable["RegisterPlugin"] = PluginApi::RegisterPlugin;
	apiTable["LogWarning"] = PluginApi::LogWarning;
	apiTable["LogInfo"] = PluginApi::LogInfo;
	apiTable["LogError"] = PluginApi::LogError;
	apiTable["GetGameState"] = PluginApi::GetGameState;
	apiTable["RegisterBlock"] = PluginApi::RegisterBlock;
	apiTable["RegisterDimension"] = PluginApi::RegisterDimension;
	apiTable["ConnectToServer"] = PluginApi::ConnectToServer;
	apiTable["Exit"] = PluginApi::Exit;
	apiTable["Disconnect"] = PluginApi::Disconnect;
	apiTable["SetStatePlaying"] = PluginApi::SetStatePlaying;
	apiSettings["Load"] = Settings::Load;
	apiSettings["Save"] = Settings::Save;
	apiSettings["Read"] = Settings::Read;
	apiSettings["Write"] = Settings::Write;
	apiSettings["ReadBool"] = Settings::ReadBool;
	apiSettings["WriteBool"] = Settings::WriteBool;
	apiSettings["ReaIntd"] = Settings::ReadInt;
	apiSettings["WriteInt"] = Settings::WriteInt;
	apiSettings["ReadDouble"] = Settings::ReadDouble;
	apiSettings["WriteDouble"] = Settings::WriteDouble;
	apiTable["SettingsUpdate"] = PluginApi::SettingsUpdate;
	apiTable["GetTime"] = GetTime;
	apiTable["GetBlockInfo"] = GetBlockInfo;
	apiTable["GetDebugValue"] = PluginApi::GetDebugValue;
	apiTable["SendChatMessage"] = PluginApi::SendChatMessage;
}

lua_State* PluginSystem::GetLuaState() {
	return lua.lua_state();
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
		if (plugin.onChangeState && plugin.errors < 10)
			try {
				plugin.onChangeState(newState);
			}
			catch (sol::error &e) {
				LOG(ERROR) << e.what();
				plugin.errors++;
			}
	}
}

void PluginSystem::CallOnTick(double deltaTime) {
	OPTICK_EVENT();
	lua.safe_script("collectgarbage('collect')");
	for (Plugin& plugin : plugins) {
		if (plugin.onTick && plugin.errors < 10)
			try {
				plugin.onTick(deltaTime);
			}
			catch (sol::error &e) {
				LOG(ERROR) << e.what();
				plugin.errors++;
			}
	}
}

BlockInfo PluginSystem::RequestBlockInfo(Vector blockPos) {
	OPTICK_EVENT();
	BlockInfo ret;
	for (Plugin& plugin : plugins) {
		if (plugin.onRequestBlockInfo && plugin.errors < 10)
			try {
			ret = plugin.onRequestBlockInfo(blockPos);
				if (!ret.blockstate.empty())
					break;
			}
			catch (sol::error & e) {
				LOG(ERROR) << e.what();
				plugin.errors++;
			}
	}
	return ret;
}

void PluginSystem::CallOnChatMessage(const Chat& chat, int position) {
	OPTICK_EVENT();
	for (Plugin& plugin : plugins) {
		if (plugin.onRequestBlockInfo && plugin.errors < 10)
			try {
				plugin.onChatMessage(chat, position);
			}
			catch (sol::error& e) {
				LOG(ERROR) << e.what();
				plugin.errors++;
			}
	}
}
