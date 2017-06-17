#pragma once

#include <easylogging++.h>
#include <SFML/Window.hpp>
#include <GL/glew.h>
#include <iomanip>
#include <tuple>
#include <glm/gtc/type_ptr.hpp>
#include "../gamestate/GameState.hpp"
#include "../network/NetworkClient.hpp"
#include "../gui/Gui.hpp"
#include "../graphics/Camera3D.hpp"
#include "../graphics/Shader.hpp"
#include "AssetManager.hpp"
#include "../graphics/Frustrum.hpp"

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

	void RenderGui(Gui &Target);

	void HandleMouseCapture();

	void HandleEvents();

	void InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle);

	void InitGlew();

	void SetMouseCapture(bool IsCaptured);

	void PrepareToWorldRendering();

	void RenderFrame();

	unsigned int width();

	unsigned int height();

	void UpdateChunksToRender();

	void UpdateGameState();

	void DrawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color);

	std::thread gameStateLoopThread;

	Shader *shader,*shader2;
	//Cube verticies, Cube VAO, Cube UVs, TextureIndexes UBO, TextureData UBO, TextureData2 UBO, Blocks VBO, Models VBO, Line VAO, Lines VBO
	GLuint VBO, VAO, VBO2, UBO, UBO2, VBO3, VBO4, VAO2, VBO5;
	std::vector<Vector> toRender;
	std::vector<Vector> optimizedRender;

	int ChunkDistance = 2;

	std::map<Vector, std::vector<glm::mat4>> toRenderModels;
	std::map<Vector, std::vector<glm::vec2>> toRenderBlocks;
	//std::map<Vector, std::tuple<std::vector<glm::mat4>, std::vector<glm::vec2> > > sectionsRenderingData;

public:
	Core();

	~Core();

	void Exec();
};
