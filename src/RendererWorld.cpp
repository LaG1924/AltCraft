#include "RendererWorld.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optick.h>

#include "DebugInfo.hpp"
#include "Frustum.hpp"
#include "Event.hpp"
#include "AssetManager.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "GameState.hpp"
#include "Section.hpp"
#include "RendererSectionData.hpp"
#include "Game.hpp"
#include "Audio.hpp"

void RendererWorld::WorkerFunction(size_t workerId) {
	OPTICK_THREAD("Worker");
    EventListener tasksListener;

	tasksListener.RegisterHandler("ParseSection", [&](const Event &eventData) {
		OPTICK_EVENT("EV_ParseSection");
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
	OPTICK_EVENT();
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
		
		parsing[id].data.section = GetGameState()->GetWorld().GetSection(vec);
		parsing[id].data.north = GetGameState()->GetWorld().GetSection(vec + Vector(0, 0, 1));
		parsing[id].data.south = GetGameState()->GetWorld().GetSection(vec + Vector(0, 0, -1));
		parsing[id].data.west = GetGameState()->GetWorld().GetSection(vec + Vector(1, 0, 0));
		parsing[id].data.east = GetGameState()->GetWorld().GetSection(vec + Vector(-1, 0, 0));
		parsing[id].data.bottom = GetGameState()->GetWorld().GetSection(vec + Vector(0, -1, 0));
		parsing[id].data.top = GetGameState()->GetWorld().GetSection(vec + Vector(0, 1, 0));

		parsing[id].parsing = true;

		PUSH_EVENT("ParseSection", std::make_tuple(currentWorker++, id, forced));
		if (currentWorker >= numOfWorkers)
			currentWorker = 0;
	}
}

void RendererWorld::ParseQeueueRemoveUnnecessary() {
	OPTICK_EVENT();
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
				
		const Section& section = GetGameState()->GetWorld().GetSection(vec);

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
	OPTICK_EVENT();
    Vector playerChunk(std::floor(GetGameState()->GetPlayer()->pos.x / 16), 0, std::floor(GetGameState()->GetPlayer()->pos.z / 16));

    std::vector<Vector> suitableChunks;
    auto chunks = GetGameState()->GetWorld().GetSectionsList();
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

    playerChunk.y = std::floor(GetGameState()->GetPlayer()->pos.y / 16.0);
    std::sort(suitableChunks.begin(), suitableChunks.end(), [playerChunk](Vector lhs, Vector rhs) {
        double leftLengthToPlayer = (playerChunk - lhs).GetLength();
        double rightLengthToPlayer = (playerChunk - rhs).GetLength();
        return leftLengthToPlayer < rightLengthToPlayer;
    });

    for (auto& it : suitableChunks) {
		PUSH_EVENT("ChunkChanged", it);
    }	
}

RendererWorld::RendererWorld() {
	OPTICK_EVENT();
    MaxRenderingDistance = 2;
    numOfWorkers = _max(1, (signed int) std::thread::hardware_concurrency() - 2);

    listener = std::make_unique<EventListener>();

	globalTimeStart = std::chrono::high_resolution_clock::now();

    PrepareRender();
    
    listener->RegisterHandler("DeleteSectionRender", [this](const Event& eventData) {
		OPTICK_EVENT("EV_DeleteSectionRender");
		auto vec = eventData.get<Vector>();
        auto it = sections.find(vec);
        if (it == sections.end())
            return;
        sections.erase(it);
    });

    listener->RegisterHandler("SectionParsed",[this](const Event &eventData) {
		OPTICK_EVENT("EV_SectionParsed");
		auto id = eventData.get<size_t>();
		parsing[id].parsing = false;

		auto it = sections.find(parsing[id].renderer.sectionPos);

		if (it != sections.end()) {
			if (parsing[id].renderer.hash == it->second.GetHash() && !parsing[id].renderer.forced) {
				LOG(WARNING) << "Generated not necessary RendererSectionData: " << parsing[id].renderer.sectionPos;
				parsing[id] = RendererWorld::SectionParsing();
				return;
			}
			it->second.UpdateData(parsing[id].renderer);
		} else
			sections.emplace(std::make_pair(parsing[id].renderer.sectionPos, RendererSection(parsing[id].renderer)));

		parsing[id] = RendererWorld::SectionParsing();
    });
    
    listener->RegisterHandler("EntityChanged", [this](const Event& eventData) {
		OPTICK_EVENT("EV_EntityChanged");
		auto data = eventData.get<unsigned int>();
        for (unsigned int entityId : GetGameState()->GetWorld().GetEntitiesList()) {
            if (entityId == data) {
                entities.push_back(RendererEntity(entityId));
            }
        }
    });

    listener->RegisterHandler("ChunkChanged", [this](const Event& eventData) {
		OPTICK_EVENT("EV_ChunkChanged");
		auto vec = eventData.get<Vector>();
		if (vec == Vector())
			return;

        Vector playerChunk(std::floor(GetGameState()->GetPlayer()->pos.x / 16), 0, std::floor(GetGameState()->GetPlayer()->pos.z / 16));

        double distanceToChunk = (Vector(vec.x, 0, vec.z) - playerChunk).GetLength();
        if (MaxRenderingDistance != 1000 && distanceToChunk > MaxRenderingDistance) {
            return;
        }

		parseQueue.push(vec);

		parseQueueNeedRemoveUnnecessary = true;
    });

	listener->RegisterHandler("ChunkChangedForce", [this](const Event& eventData) {
		OPTICK_EVENT("EV_ChunkChangedForce");
		auto vec = eventData.get<Vector>();
		if (vec == Vector())
			return;

		Vector playerChunk(std::floor(GetGameState()->GetPlayer()->pos.x / 16), 0, std::floor(GetGameState()->GetPlayer()->pos.z / 16));

		double distanceToChunk = (Vector(vec.x, 0, vec.z) - playerChunk).GetLength();
		if (MaxRenderingDistance != 1000 && distanceToChunk > MaxRenderingDistance) {
			return;
		}

		vec.y += 4500;

		parseQueue.push(vec);

		parseQueueNeedRemoveUnnecessary = true;
	});

    listener->RegisterHandler("UpdateSectionsRender", [this](const Event&) {
        UpdateAllSections(GetGameState()->GetPlayer()->pos);
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

	listener->RegisterHandler("SetMinLightLevel", [this](const Event& eventData) {
		auto value = eventData.get<float>();
		AssetManager::GetAsset<AssetShader>("/altcraft/shaders/face")->shader->Activate();
		AssetManager::GetAsset<AssetShader>("/altcraft/shaders/face")->shader->SetUniform("MinLightLevel", value);
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
    DebugInfo::renderSections = 0;
    DebugInfo::readyRenderer = 0;
}

void RendererWorld::Render(RenderState & renderState) {
	OPTICK_EVENT();
    //Common
    glm::mat4 projection = glm::perspective(
        glm::radians(70.0f), (float) renderState.WindowWidth / (float) renderState.WindowHeight,
        0.1f, 10000000.0f
    );
    glm::mat4 view = GetGameState()->GetViewMatrix();
	glm::mat4 projView = projection * view;

	{//Set listener position
	Entity *player = GetGameState()->GetPlayer();

	float playerYaw = Entity::DecodeYaw(player->yaw);
	float playerYawR = playerYaw * (M_PI / 180.f);

	float f = player->pitch * (M_PI / 180.f);
	float back = (player->pitch + 90.f) * (M_PI / 180.f);
	float cosYaw = cosf(playerYawR),
			sinYaw = sinf(playerYawR);
	float cosf0 = cosf(f),
			sinf0 = sinf(f);
	float cosBack = cosf(back),
			sinBack = sinf(back);

	Audio::UpdateListener(Vector3<float>(cosYaw*cosf0, sinf0, sinYaw*cosf0), Vector3<float>(cosBack*cosYaw, sinBack, cosBack*sinYaw), Vector3<float>(player->pos.x, player->pos.y+player->EyeOffset.y, player->pos.z), Vector3<float>(player->vel.x, player->vel.y, player->vel.z));
	}


    //Render Entities
    glLineWidth(3.0);
	Shader *entityShader = AssetManager::GetAsset<AssetShader>("/altcraft/shaders/entity")->shader.get();
	entityShader->Activate();
	entityShader->SetUniform("projection", projection);
	entityShader->SetUniform("view", view);
    glCheckError();
		
    renderState.SetActiveVao(RendererEntity::GetVao());
    for (auto& it : entities) {
        it.Render(renderState, &GetGameState()->GetWorld());
    }

    //Render selected block
    Vector selectedBlock = GetGameState()->GetSelectionStatus().selectedBlock;
    if (selectedBlock != Vector()) {
        glLineWidth(2.0f);
        {
            glm::mat4 model = glm::mat4(1.0);
            model = glm::translate(model, selectedBlock.glm());
            model = glm::translate(model,glm::vec3(0.5f,0.5f,0.5f));
            model = glm::scale(model,glm::vec3(1.01f,1.01f,1.01f));
			entityShader->SetUniform("model", model);
			entityShader->SetUniform("color", glm::vec3(0, 0, 0));
            glCheckError();
            glDrawArrays(GL_LINES, 0, 24);
        }
    }

    //Render raycast hit
    const bool renderHit = false;
    if (renderHit) {
    VectorF hit = GetGameState()->GetSelectionStatus().raycastHit;
        glLineWidth(2.0f);
        {
            glm::mat4 model;
            model = glm::translate(model, hit.glm());
            model = glm::scale(model,glm::vec3(0.3f,0.3f,0.3f));
			entityShader->SetUniform("model", model);
            if (selectedBlock == Vector())
				entityShader->SetUniform("color", glm::vec3(0.7f, 0, 0));
            else
				entityShader->SetUniform("color", glm::vec3(0, 0, 0.7f));
            glCheckError();
            glDrawArrays(GL_LINE_STRIP, 0, 36);
        }
    }

	glLineWidth(1.0);
	glCheckError();

	//Render sky
	renderState.TimeOfDay = GetGameState()->GetTimeStatus().timeOfDay;
	Shader *skyShader = AssetManager::GetAsset<AssetShader>("/altcraft/shaders/sky")->shader.get();
	skyShader->Activate();
	skyShader->SetUniform("projection", projection);
	skyShader->SetUniform("view", view);
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, GetGameState()->GetPlayer()->pos.glm());
	const float scale = 1000000.0f;
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	float shift = GetGameState()->GetTimeStatus().interpolatedTimeOfDay / 24000.0f;
	if (shift < 0)
		shift *= -1.0f;
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(360.0f * shift), glm::vec3(-1.0f, 0.0f, 0.0f));
	skyShader->SetUniform("model", model);

	glCheckError();

	const int sunriseMin = 22000;
	const int sunriseMax = 23500;
	const int moonriseMin = 12000;
	const int moonriseMax = 13500;
	const float sunriseLength = sunriseMax - sunriseMin;
	const float moonriseLength = moonriseMax - moonriseMin;

	float mixLevel = 0;
	float dayTime = GetGameState()->GetTimeStatus().interpolatedTimeOfDay;
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

	skyShader->SetUniform("DayTime", mixLevel);

	rendererSky.Render(renderState);
	glCheckError();

    //Render sections
	auto rawGlobalTime = (std::chrono::high_resolution_clock::now() - globalTimeStart);
	float globalTime = rawGlobalTime.count() / 1000000000.0f;
	Shader *blockShader = AssetManager::GetAsset<AssetShader>("/altcraft/shaders/face")->shader.get();
	blockShader->Activate();
	blockShader->SetUniform("DayTime", mixLevel);
	blockShader->SetUniform("projView", projView);
	blockShader->SetUniform("GlobalTime", globalTime);
    glCheckError();

	Frustum frustum(projView);

    size_t culledSections = sections.size();
	unsigned int renderedFaces = 0;
    for (auto& section : sections) { 
		glm::vec3 point{
			section.second.GetPosition().x * 16 + 8,
			section.second.GetPosition().y * 16 + 8,
			section.second.GetPosition().z * 16 + 8
		};

		bool isVisible = frustum.TestSphere(point, 16.0f);
        
        if (!isVisible) {
            culledSections--;
            continue;
        }
        section.second.Render(renderState);
		renderedFaces += section.second.numOfFaces;
    }
    this->culledSections = culledSections;
	DebugInfo::renderFaces.store(renderedFaces, std::memory_order_relaxed);
    glCheckError();
}

void RendererWorld::PrepareRender() {
	Shader *blockShader = AssetManager::GetAsset<AssetShader>("/altcraft/shaders/face")->shader.get();
	blockShader->Activate();
	blockShader->SetUniform("textureAtlas", 0);
	blockShader->SetUniform("MinLightLevel", 0.2f);

	TextureCoord sunTexture = AssetManager::GetTexture("/minecraft/textures/environment/sun");
	TextureCoord moonTexture = AssetManager::GetTexture("/minecraft/textures/environment/moon_phases");
	moonTexture.w /= 4.0f; //First phase will be fine for now
	moonTexture.h /= 2.0f;

	Shader *sky = AssetManager::GetAsset<AssetShader>("/altcraft/shaders/sky")->shader.get();
	sky->Activate();
	sky->SetUniform("textureAtlas", 0);	
	sky->SetUniform("sunTexture", glm::vec4(sunTexture.x, sunTexture.y, sunTexture.w, sunTexture.h));
	sky->SetUniform("sunTextureLayer", (float)sunTexture.layer);
	sky->SetUniform("moonTexture", glm::vec4(moonTexture.x, moonTexture.y, moonTexture.w, moonTexture.h));
	sky->SetUniform("moonTextureLayer", (float)moonTexture.layer);
}

void RendererWorld::Update(double timeToUpdate) {
	OPTICK_EVENT();
    static auto timeSincePreviousUpdate = std::chrono::steady_clock::now();

	if (parseQueueNeedRemoveUnnecessary)
		ParseQeueueRemoveUnnecessary();

	ParseQueueUpdate();

	listener->HandleAllEvents();
    
    if (std::chrono::steady_clock::now() - timeSincePreviousUpdate > std::chrono::seconds(5)) {
		PUSH_EVENT("UpdateSectionsRender", 0);
        timeSincePreviousUpdate = std::chrono::steady_clock::now();
    }

	DebugInfo::readyRenderer.store(parseQueue.size(), std::memory_order_relaxed);
	DebugInfo::renderSections.store(sections.size(), std::memory_order_relaxed);
}
