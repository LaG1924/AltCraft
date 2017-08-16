#include "RendererWorld.hpp"

void RendererWorld::LoadedSectionController() {
    el::Helpers::setThreadName("RenderParser");
    std::function<void(Vector)> updateAllSections = [this](Vector playerPos) {
        Vector playerChunk(std::floor(gs->g_PlayerX / 16), 0, std::floor(gs->g_PlayerZ / 16));

        std::vector<Vector> suitableChunks;
        for (auto& it : gs->world.GetSectionsList()) {
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
        std::sort(suitableChunks.begin(), suitableChunks.end(), [playerChunk](Vector lhs, Vector rhs) {
            return (playerChunk - lhs).GetLength() < (playerChunk - rhs).GetLength();
        });
        for (auto& it : suitableChunks) {
            EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ it });
        }
    };


    EventListener contentListener;    
    contentListener.RegisterHandler(EventType::ChunkChanged, [this](EventData eventData) {
        auto vec = std::get<ChunkChangedData>(eventData).chunkPosition;
        Vector playerChunk(std::floor(gs->g_PlayerX / 16), 0, std::floor(gs->g_PlayerZ / 16));   

        //if (playerChunk != Vector())
        if ((Vector(vec.x, 0, vec.z) - playerChunk).GetLength() > MaxRenderingDistance)
            return;

        sectionsMutex.lock();
        auto& result = sections.find(vec);
        if (result != sections.end()) {
            sectionsMutex.unlock();
            if (result->second.IsNeedResourcesPrepare())
                result->second.PrepareResources();
            sectionsMutex.lock();
        }
        else {
            EventAgregator::PushEvent(EventType::CreateSectionRender, CreateSectionRenderData{ vec });
        }
        sectionsMutex.unlock();
    });

    contentListener.RegisterHandler(EventType::CreatedSectionRender, [this](EventData eventData) {
        auto vec = std::get<CreatedSectionRenderData>(eventData).pos;
        sectionsMutex.lock();
        auto it = sections.find(vec);
        if (it == sections.end()) {
            LOG(ERROR) << "Created wrnog sectionRenderer";
            sectionsMutex.unlock();
            return;
        }            
        it->second.PrepareResources();
        sectionsMutex.unlock();
        EventAgregator::PushEvent(EventType::InitalizeSectionRender, InitalizeSectionRenderData{ vec });
    });

    contentListener.RegisterHandler(EventType::PlayerPosChanged, [this,&updateAllSections](EventData eventData) {
        auto pos = std::get<PlayerPosChangedData>(eventData).newPos;
        updateAllSections(pos);
    });

    contentListener.RegisterHandler(EventType::UpdateSectionsRender, [this,&updateAllSections](EventData eventData) {
        updateAllSections(Vector(gs->g_PlayerX, gs->g_PlayerY, gs->g_PlayerZ));
    });
    

    LoopExecutionTimeController timer(std::chrono::milliseconds(32));
    auto timeSincePreviousUpdate = std::chrono::steady_clock::now();
    while (isRunning) {
        while (contentListener.IsEventsQueueIsNotEmpty())
            contentListener.HandleEvent();
        if (std::chrono::steady_clock::now() - timeSincePreviousUpdate > std::chrono::seconds(15)) {
            EventAgregator::PushEvent(EventType::UpdateSectionsRender, UpdateSectionsRenderData{});
            timeSincePreviousUpdate = std::chrono::steady_clock::now();
        }
        timer.Update();
    }
}

void RendererWorld::RenderBlocks(RenderState& renderState)
{
    renderState.SetActiveShader(blockShader->Program);
    glCheckError();

    GLint projectionLoc = glGetUniformLocation(blockShader->Program, "projection");
    GLint viewLoc = glGetUniformLocation(blockShader->Program, "view");
    GLint windowSizeLoc = glGetUniformLocation(blockShader->Program, "windowSize");
    glm::mat4 projection = glm::perspective(45.0f, (float)renderState.WindowWidth / (float)renderState.WindowHeight, 0.1f, 10000000.0f);
    glm::mat4 view = gs->GetViewMatrix();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform2f(windowSizeLoc, renderState.WindowWidth, renderState.WindowHeight);

    glCheckError();

    sectionsMutex.lock();
    for (auto& it : sections) {

        it.second.Render(renderState);
    }

    sectionsMutex.unlock();
}

void RendererWorld::RenderEntities(RenderState& renderState)
{
    renderState.SetActiveShader(entityShader->Program);
    glCheckError();

}

RendererWorld::RendererWorld(GameState * ptr):gs(ptr) {
    MaxRenderingDistance = 2;

    PrepareRender();

    listener.RegisterHandler(EventType::InitalizeSectionRender, [this](EventData eventData) {
        auto data = std::get<InitalizeSectionRenderData>(eventData);
        sectionsMutex.lock();
        auto it = sections.find(data.pos);
        if (it == sections.end()) {
            LOG(ERROR) << "Initializing wrong sectionRenderer";
            sectionsMutex.unlock();
            return;
        }
        it->second.PrepareRender();
        it->second.SetEnabled(true);
        sectionsMutex.unlock();
    });

    listener.RegisterHandler(EventType::CreateSectionRender, [this](EventData eventData) {
        auto vec = std::get<CreateSectionRenderData>(eventData).pos;
        auto pair = std::make_pair(vec, RendererSection(&gs->world, vec));
        sectionsMutex.lock();
        sections.insert(pair);
        sectionsMutex.unlock();
        EventAgregator::PushEvent(EventType::CreatedSectionRender, CreatedSectionRenderData{ vec });
    });

    listener.RegisterHandler(EventType::DeleteSectionRender, [this](EventData eventData) {
        auto vec = std::get<DeleteSectionRenderData>(eventData).pos;
        sectionsMutex.lock();
        auto it = sections.find(vec);
        if (it == sections.end()) {
            LOG(ERROR) << "Deleting wrong sectionRenderer";
            sectionsMutex.unlock();
            return;
        }
        sections.erase(it);
        sectionsMutex.unlock();
    });
    
    listener.RegisterHandler(EventType::EntityChanged, [this](EventData eventData) {
        auto data = std::get<EntityChangedData>(eventData);
        for (auto&it : gs->world.entities) {
            if (it.entityId == data.EntityId) {
                entities.push_back(RendererEntity(&gs->world,it.entityId));
                return;
            }
        }
    });

    resourceLoader = std::thread(&RendererWorld::LoadedSectionController, this);
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
    resourceLoader.join();
    delete blockShader;
    delete entityShader;
}

void RendererWorld::Render(RenderState & renderState) {
    renderState.SetActiveShader(blockShader->Program);
    glCheckError();

    GLint projectionLoc = glGetUniformLocation(blockShader->Program, "projection");
    GLint viewLoc = glGetUniformLocation(blockShader->Program, "view");
    GLint windowSizeLoc = glGetUniformLocation(blockShader->Program, "windowSize");
    glm::mat4 projection = glm::perspective(45.0f, (float)renderState.WindowWidth / (float)renderState.WindowHeight, 0.1f, 10000000.0f);
    glm::mat4 view = gs->GetViewMatrix();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform2f(windowSizeLoc, renderState.WindowWidth, renderState.WindowHeight);
    glCheckError();

    sectionsMutex.lock();
    for (auto& it : sections) {

        it.second.Render(renderState);
    }
    sectionsMutex.unlock();


    renderState.SetActiveShader(entityShader->Program);
    glCheckError();
    projectionLoc = glGetUniformLocation(entityShader->Program, "projection");
    viewLoc = glGetUniformLocation(entityShader->Program, "view");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glCheckError();
    GLint modelLoc = glGetUniformLocation(entityShader->Program, "model");
    GLint colorLoc = glGetUniformLocation(entityShader->Program, "color");
    for (auto& it : entities) {
        it.modelLoc = modelLoc;
        it.colorLoc = colorLoc;
        it.Render(renderState);
    }

}

void RendererWorld::PrepareResources() {
    LOG(ERROR) << "Incorrect call";
}

void RendererWorld::PrepareRender() {
    blockShader = new Shader("./shaders/face.vs", "./shaders/face.fs");
    blockShader->Use();
    glUniform1i(glGetUniformLocation(blockShader->Program, "textureAtlas"), 0);
    entityShader = new Shader("./shaders/entity.vs", "./shaders/entity.fs");
}

bool RendererWorld::IsNeedResourcesPrepare() {
    LOG(ERROR) << "Incorrect call";
    return false;
}

void RendererWorld::Update() {
    listener.HandleEvent();
}
