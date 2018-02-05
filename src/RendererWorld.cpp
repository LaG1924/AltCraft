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

    tasksListener.RegisterHandler("RendererWorkerTask", [&](const Event& eventData) {
		auto data = eventData.get<std::tuple<size_t, Vector>>();
        if (std::get<0>(data) != workerId)
            return;
        Vector vec = std::get<1>(data);

        sectionsMutex.lock();
        auto result = sections.find(vec);
        if (result != sections.end()) {
            if (result->second.GetHash() != gs->world.GetSection(result->first).GetHash()) {
                sectionsMutex.unlock();
                auto data = std::make_unique<RendererSectionData>(&gs->world, vec);
                renderDataMutex.lock();
                renderData.push(std::move(data));
                renderDataMutex.unlock();
				PUSH_EVENT("NewRenderDataAvailable", 0);
                sectionsMutex.lock();
            }
            else {
                isParsingMutex.lock();
                isParsing[vec] = false;
                isParsingMutex.unlock();
            }                
        }
        else {
            sectionsMutex.unlock();
            auto data = std::make_unique<RendererSectionData>(&gs->world, vec);
            renderDataMutex.lock();
            renderData.push(std::move(data));
            renderDataMutex.unlock();
			PUSH_EVENT("NewRenderDataAvailable", 0);
            sectionsMutex.lock();
        }
        sectionsMutex.unlock();
    });

    LoopExecutionTimeController timer(std::chrono::milliseconds(50));
    while (isRunning) {
        while (tasksListener.NotEmpty() && isRunning)
            tasksListener.HandleEvent();
        timer.Update();
    }
}

void RendererWorld::UpdateAllSections(VectorF playerPos)
{
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

    sectionsMutex.lock();
    for (auto& it : sections) {
        if (std::find(suitableChunks.begin(), suitableChunks.end(), it.first) == suitableChunks.end())
            toRemove.push_back(it.first);
    }
    sectionsMutex.unlock();

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
        sectionsMutex.lock();
        auto it = sections.find(vec);
        if (it == sections.end()) {
            sectionsMutex.unlock();
            return;
        }
        sections.erase(it);
        sectionsMutex.unlock();
    });

    listener->RegisterHandler("NewRenderDataAvailable",[this](const Event&) {
        renderDataMutex.lock();
        int i = 0;
        while (!renderData.empty() && i++ < 20) {
            auto data = std::move(renderData.front());
            renderData.pop();
            isParsingMutex.lock();
            if (isParsing[data->sectionPos] != true)
                LOG(WARNING) << "Generated not parsed data";
            isParsing[data->sectionPos] = false;
            isParsingMutex.unlock();

            sectionsMutex.lock();
            if (sections.find(data->sectionPos) != sections.end()) {
                if (sections.find(data->sectionPos)->second.GetHash() == data->hash) {
                    LOG(INFO) << "Generated not necesarry RendererData";
                    sectionsMutex.unlock();
                    renderData.pop();
                    continue;
                }
                sections.erase(sections.find(data->sectionPos));
            }
            RendererSection renderer(*data);
            sections.insert(std::make_pair(data->sectionPos, std::move(renderer)));
            sectionsMutex.unlock();
        }
        renderDataMutex.unlock();
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
        Vector playerChunk(std::floor(gs->player->pos.x / 16), 0, std::floor(gs->player->pos.z / 16));

        double distanceToChunk = (Vector(vec.x, 0, vec.z) - playerChunk).GetLength();
        if (MaxRenderingDistance != 1000 && distanceToChunk > MaxRenderingDistance) {
            return;
        }

        isParsingMutex.lock();
        if (isParsing.find(vec) == isParsing.end())
            isParsing[vec] = false;
        if (isParsing[vec] == true) {
            isParsingMutex.unlock();
            return;
        }
        isParsing[vec] = true;
        isParsingMutex.unlock();

		PUSH_EVENT("RendererWorkerTask", std::make_tuple(currentWorker++, vec));
        if (currentWorker >= numOfWorkers)
            currentWorker = 0;
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
        sectionsMutex.lock();
        auto it = sections.find(pos);
        if (it != sections.end())
            sections.erase(it);
        sectionsMutex.unlock();
    });

    for (int i = 0; i < numOfWorkers; i++)
        workers.push_back(std::thread(&RendererWorld::WorkerFunction, this, i));

	PUSH_EVENT("UpdateSectionsRender", 0);
}

RendererWorld::~RendererWorld() {
    size_t faces = 0;
    sectionsMutex.lock();
    for (auto& it : sections) {
        faces += it.second.numOfFaces;
    }
    sectionsMutex.unlock();
    LOG(INFO) << "Total faces to render: "<<faces;
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
    glm::mat4 projection = glm::perspective(45.0f, (float)renderState.WindowWidth / (float)renderState.WindowHeight, 0.1f, 10000000.0f);
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

    //Render sections
    renderState.SetActiveShader(blockShader->Program);
    projectionLoc = glGetUniformLocation(blockShader->Program, "projection");
    viewLoc = glGetUniformLocation(blockShader->Program, "view");
    windowSizeLoc = glGetUniformLocation(blockShader->Program, "windowSize");
    pvLoc = glGetUniformLocation(blockShader->Program, "projView");
    
    glUniformMatrix4fv(pvLoc, 1, GL_FALSE, glm::value_ptr(projView));
    glUniform2f(windowSizeLoc, renderState.WindowWidth, renderState.WindowHeight);
    glCheckError();

    frustum->UpdateFrustum(projView);

    sectionsMutex.lock();
    size_t culledSections = sections.size();
    for (auto& section : sections) {        
        sectionsMutex.unlock();
        std::vector<Vector> sectionCorners = {
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

        double lengthToSection = (gs->player->pos - VectorF(section.first.x*16,section.first.y*16,section.first.z*16)).GetLength();
        
        if (!isVisible && lengthToSection > 30.0f) {
            sectionsMutex.lock();
            culledSections--;
            continue;
        }
        section.second.Render(renderState);
        sectionsMutex.lock();
    }
    this->culledSections = culledSections;
    sectionsMutex.unlock();
    glCheckError();

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
    int i = 0;
    while (listener->NotEmpty() && i++ < 50)
        listener->HandleEvent();
    if (std::chrono::steady_clock::now() - timeSincePreviousUpdate > std::chrono::seconds(5)) {
		PUSH_EVENT("UpdateSectionsRender", 0);
        timeSincePreviousUpdate = std::chrono::steady_clock::now();
    }

    DebugInfo::readyRenderer = this->renderData.size();
    DebugInfo::renderSections = this->sections.size();
}

GameState* RendererWorld::GameStatePtr() {
    return gs;
}