#include "RendererWorld.hpp"
#include "DebugInfo.hpp"
#include "Frustum.hpp"

void RendererWorld::WorkerFunction(size_t workerId) {
    EventListener tasksListener;

    tasksListener.RegisterHandler(EventType::RendererWorkerTask, [&](EventData eventData) {
        auto data = std::get<RendererWorkerTaskData>(eventData);
        if (data.WorkerId != workerId)
            return;
        Vector vec = data.Task;

        sectionsMutex.lock();
        auto result = sections.find(vec);
        if (result != sections.end()) {
            if (result->second.GetHash() != gs->world.GetSection(result->first).GetHash()) {
                sectionsMutex.unlock();
                RendererSectionData data(&gs->world, vec);
                renderDataMutex.lock();
                renderData.push(data);
                renderDataMutex.unlock();
                EventAgregator::PushEvent(EventType::NewRenderDataAvailable, NewRenderDataAvailableData{});
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
            RendererSectionData data(&gs->world, vec);
            renderDataMutex.lock();
            renderData.push(data);
            renderDataMutex.unlock();
            EventAgregator::PushEvent(EventType::NewRenderDataAvailable, NewRenderDataAvailableData{});
            sectionsMutex.lock();
        }
        sectionsMutex.unlock();
    });

    LoopExecutionTimeController timer(std::chrono::milliseconds(50));
    while (isRunning) {
        while (tasksListener.IsEventsQueueIsNotEmpty() && isRunning)
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
        EventAgregator::PushEvent(EventType::DeleteSectionRender, DeleteSectionRenderData{ it });
    }

    playerChunk.y = std::floor(gs->player->pos.y / 16.0);
    std::sort(suitableChunks.begin(), suitableChunks.end(), [playerChunk](Vector lhs, Vector rhs) {
        double leftLengthToPlayer = (playerChunk - lhs).GetLength();
        double rightLengthToPlayer = (playerChunk - rhs).GetLength();
        return leftLengthToPlayer < rightLengthToPlayer;
    });

    for (auto& it : suitableChunks) {
        EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ it });
    }
}

RendererWorld::RendererWorld(GameState* ptr):gs(ptr) {
    frustum = std::make_unique<Frustum>();
    MaxRenderingDistance = 2;
    numOfWorkers = 2;

    PrepareRender();
    
    listener.RegisterHandler(EventType::DeleteSectionRender, [this](EventData eventData) {
        auto vec = std::get<DeleteSectionRenderData>(eventData).pos;
        sectionsMutex.lock();
        auto it = sections.find(vec);
        if (it == sections.end()) {
            sectionsMutex.unlock();
            return;
        }
        sections.erase(it);
        sectionsMutex.unlock();
    });

    listener.RegisterHandler(EventType::NewRenderDataAvailable,[this](EventData eventData) {
        renderDataMutex.lock();
        int i = 0;
        while (!renderData.empty() && i++ < 20) {
            auto &data = renderData.front();
            isParsingMutex.lock();
            if (isParsing[data.sectionPos] != true)
                LOG(WARNING) << "Generated not parsed data";
            isParsing[data.sectionPos] = false;
            isParsingMutex.unlock();

            sectionsMutex.lock();
            if (sections.find(data.sectionPos) != sections.end()) {
                if (sections.find(data.sectionPos)->second.GetHash() == data.hash) {
                    LOG(INFO) << "Generated not necesarry RendererData";
                    sectionsMutex.unlock();
                    renderData.pop();
                    continue;
                }
                sections.erase(sections.find(data.sectionPos));
            }
            RendererSection renderer(data);
            sections.insert(std::make_pair(data.sectionPos, std::move(renderer)));
            sectionsMutex.unlock();
            renderData.pop();
        }
        if (renderData.empty())
            std::queue<RendererSectionData>().swap(renderData);
        renderDataMutex.unlock();
    });
    
    listener.RegisterHandler(EventType::EntityChanged, [this](EventData eventData) {
        auto data = std::get<EntityChangedData>(eventData);
        for (unsigned int entityId : gs->world.GetEntitiesList()) {
            if (entityId == data.EntityId) {
                entities.push_back(RendererEntity(&gs->world, entityId));
            }
        }
    });

    listener.RegisterHandler(EventType::ChunkChanged, [this](EventData eventData) {
        auto vec = std::get<ChunkChangedData>(eventData).chunkPosition;
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

        EventAgregator::PushEvent(EventType::RendererWorkerTask, RendererWorkerTaskData{ currentWorker++,vec });
        if (currentWorker >= numOfWorkers)
            currentWorker = 0;
    });

    listener.RegisterHandler(EventType::UpdateSectionsRender, [this](EventData eventData) {
        UpdateAllSections(gs->player->pos);
    });

    listener.RegisterHandler(EventType::PlayerPosChanged, [this](EventData eventData) {
        auto pos = std::get<PlayerPosChangedData>(eventData).newPos;
        UpdateAllSections(pos);
    });

    listener.RegisterHandler(EventType::ChunkDeleted, [this](EventData eventData) {
        auto pos = std::get<ChunkDeletedData>(eventData).pos;
        sectionsMutex.lock();
        auto it = sections.find(pos);
        if (it != sections.end())
            sections.erase(it);
        sectionsMutex.unlock();
    });

    for (int i = 0; i < numOfWorkers; i++)
        workers.push_back(std::thread(&RendererWorld::WorkerFunction, this, i));

    EventAgregator::PushEvent(EventType::UpdateSectionsRender, UpdateSectionsRenderData{});
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
    while (listener.IsEventsQueueIsNotEmpty() && i++ < 50)
        listener.HandleEvent();
    if (std::chrono::steady_clock::now() - timeSincePreviousUpdate > std::chrono::seconds(5)) {
        EventAgregator::PushEvent(EventType::UpdateSectionsRender, UpdateSectionsRenderData{});
        timeSincePreviousUpdate = std::chrono::steady_clock::now();
    }

    DebugInfo::readyRenderer = this->renderData.size();
    DebugInfo::renderSections = this->sections.size();
}

GameState* RendererWorld::GameStatePtr() {
    return gs;
}