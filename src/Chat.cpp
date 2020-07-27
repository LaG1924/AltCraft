#include "Chat.hpp"

#include <nlohmann/json.hpp>

using nlohmann::json;

Component RecurParseComponent(json j) {
	Component component;
	if (j.is_string()) {
		component.type = Component::STRING;
		component.text = j.get<std::string>();
		return component;
	}

	if (j.find("text") != j.end()) {
		component.type = Component::STRING;
		component.text = j["text"].get<std::string>();
	}
	else if (j.find("translate") != j.end()) {
		component.type = Component::TRANSLATION;
		component.translate = j["translate"].get<std::string>();
		for (auto& it : j["with"]) {
			component.with.push_back(RecurParseComponent(it));
		}
	}
	else if (j.find("keybind") != j.end()) {

	}
	else if (j.find("score") != j.end()) {

	}
	else if (j.find("selector") != j.end()) {

	}

	if (j.find("bold") != j.end())
		component.isBold = j["bold"].get<bool>();
	if (j.find("italic") != j.end())
		component.isItalic = j["italic"].get<bool>();
	if (j.find("underlined") != j.end())
		component.isUnderlined = j["underlined"].get<bool>();
	if (j.find("strikethrough") != j.end())
		component.isStrikethrough = j["strikethrough"].get<bool>();
	if (j.find("obfuscated") != j.end())
		component.isObfuscated = j["obfuscated"].get<bool>();
	if (j.find("color") != j.end())
		component.color = j["color"].get<std::string>();
	if (j.find("insertion") != j.end())
		component.insertion = j["insertion"].get<std::string>();

	if (j.find("clickEvent") != j.end()) {
		component.clickEvent.value = j["clickEvent"]["value"].get<std::string>();

		if (j["clickEvent"]["action"] == "open_url")
			component.clickEvent.action = ClickEvent::OPEN_URL;
		else if (j["clickEvent"]["action"] == "open_file")
			component.clickEvent.action = ClickEvent::OPEN_FILE;
		else if (j["clickEvent"]["action"] == "run_command")
			component.clickEvent.action = ClickEvent::RUN_COMMAND;
		else if (j["clickEvent"]["action"] == "suggest_command")
			component.clickEvent.action = ClickEvent::SUGGEST_COMMAND;
		else if (j["clickEvent"]["action"] == "change_page")
			component.clickEvent.action = ClickEvent::CHANGE_PAGE;
	}

	if (j.find("hoverEvent") != j.end()) {
		if (j["hoverEvent"]["value"].is_string())
			component.hoverEvent.value = j["hoverEvent"]["value"].get<std::string>();
		else
			component.hoverEvent.value = j["hoverEvent"]["value"]["text"].get<std::string>();

		if (j["hoverEvent"]["action"] == "show_text")
			component.hoverEvent.action = HoverEvent::SHOW_TEXT;
		else if (j["hoverEvent"]["action"] == "show_item")
			component.hoverEvent.action = HoverEvent::SHOW_ITEM;
		else if (j["hoverEvent"]["action"] == "show_entity")
			component.hoverEvent.action = HoverEvent::SHOW_ENTITY;
		else if (j["hoverEvent"]["action"] == "show_achievement")
			component.hoverEvent.action = HoverEvent::SHOW_ACHIEVEMENT;
	}

	if (j.find("extra") != j.end()) {
		for (auto& it : j["extra"]) {
			component.extra.push_back(RecurParseComponent(it));
		}
	}

	return component;
}

Chat::Chat(const std::string &json) {
	static const size_t s = sizeof(Component);
	nlohmann::json j = nlohmann::json::parse(json);
	volatile size_t f = s + 1;
	component = RecurParseComponent(j);
}

void RecurPlainTextAppend(const Component& component, std::string& str) {
	switch (component.type) {
	case Component::STRING:
		str += component.text;
		break;
	case Component::TRANSLATION:
		if (component.translate == "chat.type.text") {
			str += "<";
			RecurPlainTextAppend(component.with[0], str);
			str += "> ";
			for (int i = 1; i < component.with.size(); i++)
				RecurPlainTextAppend(component.with[i], str);
		} else {
			str += component.translate;
			str += "{";
			for (auto& it : component.with) {
				RecurPlainTextAppend(it, str);
				str += ";";
			}
			str += "}";
		}
		break;
	default:
		break;
	}

	for (auto& it : component.extra) {
		RecurPlainTextAppend(it, str);
	}
}

std::string Chat::ToPlainText() const
{
	std::string ret;
	RecurPlainTextAppend(component, ret);
	return ret;
}

std::string Chat::ToJson() const {
	return "";
}
