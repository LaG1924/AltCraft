#pragma once

#include <iomanip>
#include <tuple>

#include <easylogging++.h>
#include <SFML/Window.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <GameState.hpp>
#include <AssetManager.hpp>
#include <graphics/Shader.hpp>
#include <graphics/Gui.hpp>
#include <graphics/RenderSection.hpp>
#include <network/NetworkClient.hpp>

struct MyMutex {
    std::mutex mtx;
    std::string str;
    MyMutex(std::string name);
    void lock();
    void unlock();
};

class Core {
	GameState *gameState;
	NetworkClient *client;
	sf::Window *window;
	AssetManager *assetManager;
	bool isMouseCaptured = false;
	bool isRunning = true;
	enum {
		MainMenu,
		Loading,
		Playing,
		PauseMenu,
	} currentState = Playing;
	float mouseXDelta, mouseYDelta;
	float deltaTime;
	float absTime;

	void RenderWorld();

	void HandleMouseCapture();

	void HandleEvents();

	void InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle);

	void InitGlew();

	void SetMouseCapture(bool IsCaptured);

	void PrepareToRendering();

	void RenderFrame();

	unsigned int width();

	unsigned int height();

	void UpdateChunksToRender();

	void UpdateGameState();

	void UpdateSections();

	std::thread gameStateLoopThread;
	std::thread sectionUpdateLoopThread;

	Shader *shader;
	//Cube verticies, Cube VAO, Cube UVs, TextureIndexes UboTextureIndexes, TextureData UboTextureIndexes, TextureData2 UboTextureIndexes, Blocks VBO, Models VBO, Line VAO, Lines VBO
	bool isRendersShouldBeCreated=false;
	std::condition_variable waitRendersCreated;
	std::vector<Vector> renders;
	std::mutex toRenderMutex;
	std::vector<Vector> toRender;
	std::map<Vector, RenderSection> availableChunks;
	std::mutex availableChunksMutex;

	int ChunkDistance = 1;

	RenderState renderState;

	double tickRate = 0;
    double sectionRate = 0;

public:
	Core();

	~Core();

	void Exec();
};
