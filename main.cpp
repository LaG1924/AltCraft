#include <iostream>
#include "Game.hpp"
#include "graphics/AssetManager.hpp"

int main() {
    try {
        Game game;
        game.Exec();
    } catch (int e) {
        std::cerr << "Catch exception " << e << std::endl;
    }
    return 0;
}