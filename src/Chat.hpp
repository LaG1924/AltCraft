#pragma once

#include <vector>
#include <string>


struct ClickEvent {
	enum ActionType {
		NONE,
		OPEN_URL,
		OPEN_FILE,
		RUN_COMMAND,
		SUGGEST_COMMAND,
		CHANGE_PAGE,
	} action = NONE;
	std::string value;
};

struct HoverEvent {
	enum ActionType {
		NONE,
		SHOW_TEXT,
		SHOW_ITEM,
		SHOW_ENTITY,
		SHOW_ACHIEVEMENT,
	} action = NONE;
	std::string value;
};

struct Component {
	enum ComponentType {
		DEFAULT,
		STRING,
		TRANSLATION,
		KEYBIND,
		SCORE,
		SELECTOR,
		UNKNOWN,
	} type = DEFAULT;
	 
	bool isBold = false;
	bool isItalic = false;
	bool isUnderlined = false;
	bool isStrikethrough = false;
	bool isObfuscated = false;
	std::string color;
	std::string insertion;
	ClickEvent clickEvent;
	HoverEvent hoverEvent;
	std::vector<Component> extra;

	//string component
	std::string text;

	//tranlation component
	std::string translate;
	std::vector<Component> with;

	//keybind component
	std::string keybind;

	//score component
	struct {
		std::string name;
		std::string objective;
		std::string value;
	} score;

	//selector component
	std::string selector;
};

class Chat {
	Component component;

public:

	Chat() = default;

	Chat(const std::string &json);

	std::string ToPlainText() const;

	std::string ToJson() const;
};
