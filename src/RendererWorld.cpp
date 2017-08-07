#include "RendererWorld.hpp"

void RendererWorld::LoadedSectionController() {

    std::function<void(Vector)> updateAllSections = [this](Vector playerPos) {
        Vector playerChunk(std::floor(gs->g_PlayerX / 16), 0, std::floor(gs->g_PlayerZ / 16));

        std::vector<Vector> suitableChunks;
        for (auto& it : gs->world.GetSectionsList()) {
            double distance = (Vector(it.GetX(),0,it.GetZ()) - playerChunk).GetMagnitude();
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

        for (auto& it : suitableChunks) {
            EventAgregator::PushEvent(EventType::ChunkChanged, ChunkChangedData{ it });
        }
    };


    EventListener contentListener;
    contentListener.RegisterHandler(EventType::ChunkChanged, [this](EventData eventData) {
        auto vec = std::get<ChunkChangedData>(eventData).chunkPosition;
        Vector playerChunk(std::floor(gs->g_PlayerX / 16), 0, std::floor(gs->g_PlayerZ / 16));

        if ((playerChunk - Vector(vec.GetX(), 0, vec.GetZ())).GetMagnitude() > MaxRenderingDistance)
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
        sections.find(vec)->second.PrepareResources();
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

RendererWorld::RendererWorld(GameState * ptr):gs(ptr) {
    MaxRenderingDistance = 2;

    PrepareRender();

    listener.RegisterHandler(EventType::InitalizeSectionRender, [this](EventData eventData) {
        auto data = std::get<InitalizeSectionRenderData>(eventData);
        sectionsMutex.lock();
        sections.find(data.pos)->second.PrepareRender();
        sections.find(data.pos)->second.SetEnabled(true);
        sectionsMutex.unlock();
    });

    listener.RegisterHandler(EventType::CreateSectionRender, [this](EventData eventData) {
        auto vec = std::get<CreateSectionRenderData>(eventData).pos;
        sectionsMutex.lock();
        sections.insert(std::make_pair(vec, RendererSection(&gs->world, vec)));
        sectionsMutex.unlock();
        EventAgregator::PushEvent(EventType::CreatedSectionRender, CreatedSectionRenderData{ vec });
    });

    listener.RegisterHandler(EventType::DeleteSectionRender, [this](EventData eventData) {
        auto vec = std::get<DeleteSectionRenderData>(eventData).pos;
        sectionsMutex.lock();
        sections.erase(sections.find(vec));
        sectionsMutex.unlock();
    });

    resourceLoader = std::thread(&RendererWorld::LoadedSectionController, this);
}

RendererWorld::~RendererWorld() {
    isRunning = false;
    resourceLoader.join();
    delete shader;
}

void RendererWorld::Render(RenderState & renderState) {
    renderState.SetActiveShader(shader->Program);
    glCheckError();

    GLint projectionLoc = glGetUniformLocation(shader->Program, "projection");
    GLint viewLoc = glGetUniformLocation(shader->Program, "view"); 
    GLint windowSizeLoc = glGetUniformLocation(shader->Program, "windowSize");
    glm::mat4 projection = glm::perspective(45.0f, (float)renderState.WindowWidth / (float)renderState.WindowHeight, 0.1f, 10000000.0f);
    glm::mat4 view = gs->GetViewMatrix();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));    
    glUniform2f(windowSizeLoc, renderState.WindowWidth, renderState.WindowHeight);

    glCheckError();

    sectionsMutex.lock();
    for (auto& it : sections)
        it.second.Render(renderState);
    sectionsMutex.unlock();

    listener.HandleEvent();
}

void RendererWorld::PrepareResources() {
    LOG(ERROR) << "Incorrect call";
}

void RendererWorld::PrepareRender() {
    shader = new Shader("./shaders/face.vs", "./shaders/face.fs");
    shader->Use();
    glUniform1i(glGetUniformLocation(shader->Program, "textureAtlas"), 0);
}

bool RendererWorld::IsNeedResourcesPrepare() {
    LOG(ERROR) << "Incorrect call";
    return false;
}
