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

	std::thread gameStateLoopThread;

	Shader *shader;
	//Cube verticies, Cube VAO, Cube UVs, TextureIndexes UboTextureIndexes, TextureData UboTextureIndexes, TextureData2 UboTextureIndexes, Blocks VBO, Models VBO, Line VAO, Lines VBO
	GLuint UboTextureIndexes, UboTextureData;
	std::vector<Vector> toRender;
	std::map<Vector, RenderSection> availableChunks;

	int ChunkDistance = 1;

	RenderState renderState;

	double tickRate = 0;

public:
	Core();

	~Core();

	void Exec();
};
