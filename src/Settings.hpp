#pragma once

#include <string>

namespace Settings {
	void Load();

	void Save();

	std::string Read(const std::string &key, const std::string &defaultValue);

	void Write(const std::string &key, const std::string &value);

	inline bool ReadBool(const std::string &key, bool defaultValue) {
		return Read(key, std::to_string(defaultValue)) == "true";
	}

	inline void WriteBool(const std::string &key, bool value) {
		Write(key, value ? "true" : "false");
	}

	inline int ReadInt(const std::string &key, int defaultValue) {
		return std::stoi(Read(key, std::to_string(defaultValue)));
	}

	inline void WriteInt(const std::string &key, int value) {
		Write(key, std::to_string(value));
	}

	inline double ReadDouble(const std::string &key, double defaultValue) {
		return std::stod(Read(key, std::to_string(defaultValue)));
	}

	inline void WriteDouble(const std::string &key, double value) {
		Write(key, std::to_string(value));
	}

}