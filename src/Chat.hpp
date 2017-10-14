#pragma once

#include <vector>

struct TextModifier {
    size_t offset;
    size_t length;
    enum {
        Italic,
        Bold,
        Underline,
    } type;
};

struct Chat {
    std::vector<TextModifier> modifiers;
    std::string text;

    Chat(const std::string &str);

    Chat() = default;

    std::string ToJson() const;
};