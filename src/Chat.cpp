#include "Chat.hpp"

#include <nlohmann/json.hpp>

using nlohmann::json;

void Recursive(const json::iterator &iter, bool isIterObj, std::string &str) {
    if (isIterObj && iter.key() == "text") {
        str.append(iter->get<std::string>() + " ");
    }

    if (iter->is_structured()) {
        for (json::iterator it = iter->begin(); it != iter->end(); ++it)
            Recursive(it, iter->is_object(), str);
    }
}

Chat::Chat(const std::string &str) {
    json j = json::parse(str);

    if (j["translate"] == "chat.type.text") {
        text = j["with"][0]["text"].get<std::string>() + ": " + j["with"][1].get<std::string>();
    } else
        for (json::iterator it = j.begin(); it != j.end(); ++it) {
            Recursive(it, j.is_object(), text);
        }
}

std::string Chat::ToJson() const {
    throw std::logic_error("Chat not deserealizable");
    return text;
}