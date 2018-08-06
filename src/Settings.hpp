#pragma once

#include <string>

namespace Settings {
	void Load();

	void Save();

	std::string Read(const std::string &key, const std::string &defaultValue);

	void Write(const std::string &key, const std::string &value);
}