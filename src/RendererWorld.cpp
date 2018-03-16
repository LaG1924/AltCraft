#include "RendererWorld.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DebugInfo.hpp"
#include "Frustum.hpp"
#include "Event.hpp"
#include "AssetManager.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "GameState.hpp"
#include "Section.hpp"
#include "RendererSectionData.hpp"

void RendererWorld::WorkerFunction(size_t workerId) {
    EventListener tasksListener;

	tasksListener.RegisterHandler("ParseSection", [&](const Event &eventData) {
		auto data = eventData.get<std::tuple<size_t, size_t, bool>>();
		if (std::get<0>(data) != workerId)
			return;
		size_t id = std::get<1>(data);
		bool forced = std::get<2>(data);
		parsing[id].renderer = ParseSection(parsing[id].data);
		parsing[id].renderer.forced = forced;
		PUSH_EVENT("SectionParsed", id);
	});

    LoopExecutionTimeController timer(std::chrono::milliseconds(50));
    while (isRunning) {
        while (tasksListener.NotEmpty() && isRunning)
            tasksListener.HandleEvent();
        timer.Update();
    }
}

void RendererWorld::ParseQueueUpdate() {
	while (!parseQueue.empty()) {
		size_t id = 0;
		for (; id < RendererWorld::parsingBufferSize && parsing[id].parsing; ++id) {}
		if (id >= RendererWorld::parsingBufferSize)
			break;

		Vector vec = parseQueue.front();
		parseQueue.pop();

		bool forced = false;

		if (vec.y > 4000) {
			forced = true;
			vec.y -= 4500;
		}
		
		parsing[id].data.section = gs->world.GetSection(vec);
		parsing[id].data.north = gs->world.GetSection(vec + Vector(0, 0, 1));
		parsing[id].data.south = gs->world.GetSection(vec + Vector(0, 0, -1));
		parsing[id].data.west = gs->world.GetSection(vec + Vector(1, 0, 0));
		parsing[id].data.east = gs->world.GetSection(vec + Vector(-1, 0, 0));
		parsing[id].data.bottom = gs->world.GetSection(vec + Vector(0, -1, 0));
		parsing[id].data.top = gs->world.GetSection(vec + Vector(0, 1, 0));

		parsing[id].parsing = true;

		PUSH_EVENT("ParseSection", std::make_tuple(currentWorker++, id, forced));
		if (currentWorker >= numOfWorkers)
			currentWorker = 0;
	}
}

void RendererWorld::ParseQeueueRemoveUnnecessary() {
	size_t size = parseQueue.size();
	static std::vector<Vector> elements;
	elements.clear();
	elements.reserve(size);

	for (size_t i = 0; i < size; i++) {
		Vector vec = parseQueue.front();
		parseQueue.pop();

		if (vec.y > 4000) {
			parseQueue.push(vec);
			continue;
		}

		if (std::find(elements.begin(), elements.end(), vec) != elements.end())
			continue;
				
		const Section& section = gs->world.GetSection(vec);

		bool skip = false;

		for (int i = 0; i < RendererWorld::parsingBufferSize; i++) {
			if (parsing[i].data.section.GetHash() == section.GetHash()) {
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		auto it = sections.find(vec);
		if (it != sections.end() && section.GetHash() == it->second.GetHash()) {
			continue;
		}

		parseQueue.push(vec);
		elements.push_back(vec);
	}

	parseQueueNeedRemoveUnnecessary = false;
}

void RendererWorld::UpdateAllSections(VectorF playerPos) {
    Vector playerChunk(std::floor(gs->player->pos.x / 16), 0, std::floor(gs->player->pos.z / 16));

    std::vector<Vector> suitableChunks;
    auto chunks = gs->world.GetSectionsList();
    for (auto& it : chunks) {
        double distance = (Vector(it.x, 0, it.z) - playerChunk).GetLength();
        if (distance > MaxRenderingDistance)
            continue;
        suitableChunks.push_back(it);
    }

    std::vector<Vector> toRemove;

    for (auto& it : sections) {
        if (std::find(suitableChunks.begin(), suitableChunks.end(), it.first) == suitableChunks.end())
            toRemove.push_back(it.first);
    }

    for (auto& it : toRemove) {
		PUSH_EVENT("DeleteSectionRender", it);
    }

    playerChunk.y = std::floor(gs->player->pos.y / 16.0);
    std::sort(suitableChunks.begin(), suitableChunks.end(), [playerChunk](Vector lhs, Vector rhs) {
        double leftLengthToPlayer = (playerChunk - lhs).GetLength();
        double rightLengthToPlayer = (playerChunk - rhs).GetLength();
        return leftLengthToPlayer < rightLengthToPlayer;
    });

    for (auto& it : suitableChunks) {
		PUSH_EVENT("ChunkChanged", it);
    }	
}

RendererWorld::RendererWorld(GameState* ptr) {
    gs = ptr;
    frustum = std::make_unique<Frustum>();
    MaxRenderingDistance = 2;
    numOfWorkers = 2;

    listener = std::make_unique<EventListener>();

    PrepareRender();
    
    listener->RegisterHandler("DeleteSectionRender", [this](const Event& eventData) {
		auto vec = eventData.get<Vector>();
        auto it = sections.find(vec);
        if (it == sections.end())
            return;
        sections.erase(it);
    });

    listener->RegisterHandler("SectionParsed",[this](const Event &eventData) {
		auto id = eventData.get<size_t>();
		parsing[id].parsing = false;

		auto it = sections.find(parsing[id].renderer.sectionPos);

		if (it != sections.end() && parsing[id].renderer.hash == it->second.GetHash() && !parsing[id].renderer.forced) {
			LOG(WARNING) << "Generated not necessary RendererSectionData: " << parsing[id].renderer.sectionPos;
			return;
		}

		if (it != sections.end())
			sections.erase(it);

		const RendererSectionData &data = parsing[id].renderer;
		
		sections.emplace(std::make_pair(parsing[id].renderer.sectionPos, RendererSection(data)));

		parsing[id] = RendererWorld::SectionParsing();
    });
    
    listener->RegisterHandler("EntityChanged", [this](const Event& eventData) {
		auto data = eventData.get<unsigned int>();
        for (unsigned int entityId : gs->world.GetEntitiesList()) {
            if (entityId == data) {
                entities.push_back(RendererEntity(&gs->world, entityId));
            }
        }
    });

    listener->RegisterHandler("ChunkChanged", [this](const Event& eventData) {
		auto vec = eventData.get<Vector>();
		if (vec == Vector())
			return;

        Vector playerChunk(std::floor(gs->player->pos.x / 16), 0, std::floor(gs->player->pos.z / 16));

        double distanceToChunk = (Vector(vec.x, 0, vec.z) - playerChunk).GetLength();
        if (MaxRenderingDistance != 1000 && distanceToChunk > MaxRenderingDistance) {
            return;
        }

		parseQueue.push(vec);

		parseQueueNeedRemoveUnnecessary = true;
    });

	listener->RegisterHandler("ChunkChangedForce", [this](const Event& eventData) {
		auto vec = eventData.get<Vector>();
		if (vec == Vector())
			return;

		Vector playerChunk(std::floor(gs->player->pos.x / 16), 0, std::floor(gs->player->pos.z / 16));

		double distanceToChunk = (Vector(vec.x, 0, vec.z) - playerChunk).GetLength();
		if (MaxRenderingDistance != 1000 && distanceToChunk > MaxRenderingDistance) {
			return;
		}

		vec.y += 4500;

		parseQueue.push(vec);

		parseQueueNeedRemoveUnnecessary = true;
	});

    listener->RegisterHandler("UpdateSectionsRender", [this](const Event&) {
        UpdateAllSections(gs->player->pos);
    });

    listener->RegisterHandler("PlayerPosChanged", [this](const Event& eventData) {
		auto pos = eventData.get<VectorF>();
        UpdateAllSections(pos);
    });

    listener->RegisterHandler("ChunkDeleted", [this](const Event& eventData) {
		auto pos = eventData.get<Vector>();
        auto it = sections.find(pos);
        if (it != sections.end())
            sections.erase(it);
    });

    for (int i = 0; i < numOfWorkers; i++)
        workers.push_back(std::thread(&RendererWorld::WorkerFunction, this, i));

	PUSH_EVENT("UpdateSectionsRender", 0);
}

RendererWorld::~RendererWorld() {
    size_t faces = 0;
    for (auto& it : sections) {
        faces += it.second.numOfFaces;
    }
    LOG(INFO) << "Total faces to render: " << faces;
    isRunning = false;
    for (int i = 0; i < numOfWorkers; i++)
        workers[i].join();
    delete blockShader;
    delete entityShader;
    delete skyShader;
    DebugInfo::renderSections = 0;
    DebugInfo::readyRenderer = 0;
}

void RendererWorld::Render(RenderState & renderState) {
    //Common
    GLint projectionLoc, viewLoc, modelLoc, pvLoc, windowSizeLoc, colorLoc;
    glm::mat4 projection = glm::perspective(
        45.0f, (float) renderState.WindowWidth / (float) renderState.WindowHeight,
        0.1f, 10000000.0f
    );
    glm::mat4 view = gs->GetViewMatrix();
    glm::mat4 projView = projection * view;

    //Render Entities
    glLineWidth(3.0);
    renderState.SetActiveShader(entityShader->Program);
    glCheckError();
    projectionLoc = glGetUniformLocation(entityShader->Program, "projection");
    viewLoc = glGetUniformLocation(entityShader->Program, "view");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glCheckError();
    modelLoc = glGetUniformLocation(entityShader->Program, "model");
    colorLoc = glGetUniformLocation(entityShader->Program, "color");
    for (auto& it : entities) {
        it.modelLoc = modelLoc;
        it.colorLoc = colorLoc;
        it.Render(renderState);
    }
    glLineWidth(1.0);
    glCheckError();

    //Render selected block
    Vector selectedBlock = gs->selectedBlock;
    if (selectedBlock != Vector()) {
        glLineWidth(2.0f);
        {
            glm::mat4 model;
            model = glm::translate(model, selectedBlock.glm());
            model = glm::translate(model,glm::vec3(0.5f,0.5f,0.5f));
            model = glm::scale(model,glm::vec3(1.01f,1.01f,1.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(colorLoc, 0.0, 0.0, 0.0);
            glCheckError();
            glDrawArrays(GL_LINE_STRIP, 0, 36);
        }
        glLineWidth(1.0f);
        glCheckError();
    }

    //Render raycast hit
    bool renderHit = false;
    if (renderHit) {
        VectorF hit = gs->raycastHit;
        glLineWidth(2.0f);
        {
            glm::mat4 model;
            model = glm::translate(model, hit.glm());
            model = glm::scale(model,glm::vec3(0.3f,0.3f,0.3f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            if (selectedBlock == Vector())
                glUniform3f(colorLoc,0.7,0.0,0.0);
            else
                glUniform3f(colorLoc, 0.0, 0.0, 0.7);
            glCheckError();
            glDrawArrays(GL_LINE_STRIP, 0, 36);
        }
        glLineWidth(1.0f);
        glCheckError();
    }

	//Render sky
	renderState.TimeOfDay = gs->TimeOfDay;
	renderState.SetActiveShader(skyShader->Program);
	projectionLoc = glGetUniformLocation(skyShader->Program, "projection");
	viewLoc = glGetUniformLocation(skyShader->Program, "view");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glm::mat4 model = glm::mat4();
	model = glm::translate(model, gs->player->pos.glm());
	const float scale = 1000000.0f;
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	float shift = gs->TimeOfDay / 24000.0f;
	if (shift < 0)
		shift *= -1.0f;
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(360.0f * shift), glm::vec3(-1.0f, 0.0f, 0.0f));
	modelLoc = glGetUniformLocation(skyShader->Program, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glCheckError();

	const int sunriseMin = 22000;
	const int sunriseMax = 23500;
	const int moonriseMin = 12000;
	const int moonriseMax = 13500;
	const float sunriseLength = sunriseMax - sunriseMin;
	const float moonriseLength = moonriseMax - moonriseMin;

	float mixLevel = 0;
	int dayTime = gs->TimeOfDay;
	if (dayTime < 0)
		dayTime *= -1;
	while (dayTime > 24000)
		dayTime -= 24000;
	if (dayTime > 0 && dayTime < moonriseMin || dayTime > sunriseMax) //day
		mixLevel = 1.0;
	if (dayTime > moonriseMax && dayTime < sunriseMin) //night
		mixLevel = 0.0;
	if (dayTime >= sunriseMin && dayTime <= sunriseMax) //sunrise
		mixLevel = (dayTime - sunriseMin) / sunriseLength;
	if (dayTime >= moonriseMin && dayTime <= moonriseMax) { //moonrise
		float timePassed = (dayTime - moonriseMin);
		mixLevel = 1.0 - (timePassed / moonriseLength);
	}

	glUniform1f(glGetUniformLocation(skyShader->Program, "DayTime"), mixLevel);

	rendererSky.Render(renderState);
	glCheckError();

    //Render sections
    renderState.SetActiveShader(blockShader->Program);
    projectionLoc = glGetUniformLocation(blockShader->Program, "projection");
    viewLoc = glGetUniformLocation(blockShader->Program, "view");
    windowSizeLoc = glGetUniformLocation(blockShader->Program, "windowSize");
    pvLoc = glGetUniformLocation(blockShader->Program, "projView");

	glUniform1f(glGetUniformLocation(blockShader->Program, "DayTime"), mixLevel);
    glUniformMatrix4fv(pvLoc, 1, GL_FALSE, glm::value_ptr(projView));
    glUniform2f(windowSizeLoc, renderState.WindowWidth, renderState.WindowHeight);
    glCheckError();

    frustum->UpdateFrustum(projView);

    size_t culledSections = sections.size();
    for (auto& section : sections) {
        const static Vector sectionCorners[] = {
            Vector(0, 0, 0),
            Vector(0, 0, 16),
            Vector(0, 16, 0),
            Vector(0, 16, 16),
            Vector(16, 0, 0),
            Vector(16, 0, 16),
            Vector(16, 16, 0),
            Vector(16, 16, 16),
        };
        bool isVisible = false;
        for (const auto &it : sectionCorners) {
            VectorF point(section.second.GetPosition().x * 16 + it.x,
                section.second.GetPosition().y * 16 + it.y,
                section.second.GetPosition().z * 16 + it.z);
            if (frustum->TestPoint(point)) {
                isVisible = true;
                break;
            }
        }

        double lengthToSection = (gs->player->pos -
                                  VectorF(section.first.x*16,
                                          section.first.y*16,
                                          section.first.z*16)
                                 ).GetLength();
        
        if (!isVisible && lengthToSection > 30.0f) {
            culledSections--;
            continue;
        }
        section.second.Render(renderState);
    }
    this->culledSections = culledSections;
    glCheckError();
}

void RendererWorld::PrepareRender() {
    blockShader = new Shader("./shaders/face.vs", "./shaders/face.fs");
    blockShader->Use();
    glUniform1i(glGetUniformLocation(blockShader->Program, "textureAtlas"), 0);

    entityShader = new Shader("./shaders/entity.vs", "./shaders/entity.fs");

    skyShader = new Shader("./shaders/sky.vs", "./shaders/sky.fs");
    skyShader->Use();
    glUniform1i(glGetUniformLocation(skyShader->Program, "textureAtlas"), 0);
    TextureCoordinates skyTexture = AssetManager::Instance().GetTextureByAssetName("minecraft/textures/entity/end_portal");
    glUniform4f(glGetUniformLocation(skyShader->Program, "skyTexture"),skyTexture.x,skyTexture.y,skyTexture.w,skyTexture.h);
    TextureCoordinates sunTexture = AssetManager::Instance().GetTextureByAssetName("minecraft/textures/environment/sun");
    glUniform4f(glGetUniformLocation(skyShader->Program, "sunTexture"), sunTexture.x, sunTexture.y, sunTexture.w, sunTexture.h);
    TextureCoordinates moonTexture = AssetManager::Instance().GetTextureByAssetName("minecraft/textures/environment/moon_phases");
    moonTexture.w /= 4.0f; //First phase will be fine for now
    moonTexture.h /= 2.0f;
    glUniform4f(glGetUniformLocation(skyShader->Program, "moonTexture"), moonTexture.x, moonTexture.y, moonTexture.w, moonTexture.h);
}

void RendererWorld::Update(double timeToUpdate) {
    static auto timeSincePreviousUpdate = std::chrono::steady_clock::now();

	if (parseQueueNeedRemoveUnnecessary)
		ParseQeueueRemoveUnnecessary();

	ParseQueueUpdate();

	listener->HandleAllEvents();
    
    if (std::chrono::steady_clock::now() - timeSincePreviousUpdate > std::chrono::seconds(5)) {
		PUSH_EVENT("UpdateSectionsRender", 0);
        timeSincePreviousUpdate = std::chrono::steady_clock::now();
    }

	DebugInfo::readyRenderer = parseQueue.size();
	DebugInfo::renderSections = sections.size();
}

GameState* RendererWorld::GameStatePtr() {
    return gs;
}