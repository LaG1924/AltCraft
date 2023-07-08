#include "Settings.hpp"

#include <fstream>
#include <map>

#include <nlohmann/json.hpp>
#include <easylogging++.h>


using json = nlohmann::json;

std::map<std::string, std::string> values;

const std::string saveFileName = "./settings.json";

void Settings::Load() {
	std::ifstream stream(saveFileName);
	if (!stream) {
		LOG(ERROR) << "Loading settings failed: Can't open ifstream for " << saveFileName << ": " << strerror(errno);
		return;
	}
	json j;
	stream >> j;

	for (json::iterator it = j.begin(); it != j.end(); ++it) {
		values.try_emplace(it.key(), it->get<std::string>());
	}
	LOG(INFO) << "Loaded " << values.size() << " settings";
}

void Settings::Save() {
	json j;
	for (auto &it : values) {
		j[it.first] = it.second;
	}
	std::string text = j.dump(4);

	std::ofstream stream(saveFileName);
	if (!stream) {
		LOG(ERROR) << "Saving settings failed: Can't open ofstream for " << saveFileName << ": "<< strerror(errno);
		return;
	}
	stream << text;
	LOG(INFO) << "Saved " << values.size() << " settings";
}

std::string Settings::Read(const std::string &key, const std::string &defaultValue) {
	auto it = values.find(key);
	if (it == values.end()) {
		values.try_emplace(key, defaultValue);
		it = values.find(key);
	}
	return it->second;
}

void Settings::Write(const std::string &key, const std::string &value) {
	values[key] = value;
}
