#include <iostream>
#include "Game.hpp"
#include "graphics/AssetManager.hpp"
#include "Debug.hpp"

int main() {
    LOG("Process started",INFO,Unknown);
    try {
        Game game;
        game.Exec();
    } catch (std::exception e) {
        std::cerr << "Catch exception: " << e.what() << std::endl;
    }
    return 0;
}