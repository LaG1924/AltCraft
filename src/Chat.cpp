#include "Chat.hpp"

#include <nlohmann/json.hpp>
#include <easylogging++.h>

Chat::Chat(const std::string &str) {
    using nlohmann::json;
    json j = json::parse(str);

    /*LOG(WARNING) << j.dump(4);

    std::function<void(json::iterator)> iterating = [&](json::iterator iter) {
        json val = *iter;

        if (val.is_object() && val.find("text") != val.end()) {
            text.append(val["text"].get<std::string>());
        }

        if (val.is_array() || val.is_object()) {
            for (auto it = val.begin(); it != val.end(); ++it) {
                iterating(it);
            }
        }
    };

    for (auto it = j.begin(); it != j.end(); ++it) {
        iterating(it);
    }*/

    text = j.dump(4);
}

std::string Chat::ToJson() const {
    throw std::logic_error("Chat not deserealizable");
    return text;
}