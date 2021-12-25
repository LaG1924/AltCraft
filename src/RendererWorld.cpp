#include "RendererWorld.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <optick.h>

#include "DebugInfo.hpp"
#include "Frustum.hpp"
#include "Event.hpp"
#include "AssetManager.hpp"
#include "GameState.hpp"
#include "Section.hpp"
#include "RendererSectionData.hpp"
#include "Game.hpp"
#include "RenderConfigs.hpp"

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
	while (!tasksListener.Empty() && isRunning)
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
		
        for (int x = -1; x < 2; x++) {
            for (int y = -1; y < 2; y++) {
                for (int z = -1; z < 2; z++) {
                    parsing[id].data.data[x + 1][y + 1][z + 1] = GetGameState()->GetWorld().GetSection(vec + Vector(x, y, z));
                }
            }
        }

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
            if (parsing[i].data.data[1][1][1].GetHash() == section.GetHash()) {
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

RendererWorld::RendererWorld(std::shared_ptr<Gal::Framebuffer> target, bool defferedShading) {
	OPTICK_EVENT();
    MaxRenderingDistance = 2;
    numOfWorkers = _max(1, (signed int) std::thread::hardware_concurrency() - 2);

    listener = std::make_unique<EventListener>();

	globalTimeStart = std::chrono::high_resolution_clock::now();

    PrepareRender(target, defferedShading);
    
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
			sections.emplace(std::make_pair(parsing[id].renderer.sectionPos, RendererSection(parsing[id].renderer, sectionsPipeline, sectionsBufferBinding)));

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

void RendererWorld::Render(float screenRatio) {
	OPTICK_EVENT();
    //Common

    auto globalSpb = Gal::GetImplementation()->GetGlobalShaderParameters();

    auto& projection = globalSpb->Get<GlobalShaderParameters>()->proj;
    projection = glm::perspective(
        glm::radians(70.0f), screenRatio,
        0.1f, 10000000.0f
    );

    globalSpb->Get<GlobalShaderParameters>()->invProj = glm::inverse(projection);

    auto& view = globalSpb->Get<GlobalShaderParameters>()->view;
    view = GetGameState()->GetViewMatrix();

    auto& projView = globalSpb->Get<GlobalShaderParameters>()->projView;
    projView = projection * view;

    //Render Entities
    constexpr size_t entitiesVerticesCount = 240;
    entitiesPipeline->Activate();
    entitiesPipelineInstance->Activate();
    for (auto& it : entities) {
        it.Render(entitiesPipeline, &GetGameState()->GetWorld());
        entitiesPipelineInstance->Render(0, entitiesVerticesCount);
    }

    //Render selected block
    Vector selectedBlock = GetGameState()->GetSelectionStatus().selectedBlock;
    if (selectedBlock != Vector()) {
        {
            glm::mat4 model = glm::mat4(1.0);
            model = glm::translate(model, selectedBlock.glm());
            model = glm::translate(model,glm::vec3(0.5f,0.5f,0.5f));
            model = glm::scale(model,glm::vec3(1.01f,1.01f,1.01f));
            entitiesPipeline->SetShaderParameter("model", model);
            entitiesPipeline->SetShaderParameter("entityColor", glm::vec3(0, 0, 0));
            entitiesPipelineInstance->Render(0, entitiesVerticesCount);
        }
    }

    //Render raycast hit
    const bool renderHit = false;
    if (renderHit) {
        VectorF hit = GetGameState()->GetSelectionStatus().raycastHit;
        {
            glm::mat4 model;
            model = glm::translate(model, hit.glm());
            model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
            //entityShader->SetUniform("model", model);
            entitiesPipeline->SetShaderParameter("model", model);
            if (selectedBlock == Vector())
                entitiesPipeline->SetShaderParameter("entityColor", glm::vec3(0.7f, 0.0f, 0.0f));
            else
                entitiesPipeline->SetShaderParameter("entityColor", glm::vec3(0.0f, 0.0f, 0.7f));
            entitiesPipelineInstance->Render(0, entitiesVerticesCount);
        }
    }

    //Render sections
    auto rawGlobalTime = (std::chrono::high_resolution_clock::now() - globalTimeStart);
    float globalTime = rawGlobalTime.count() / 1000000000.0f;
    globalSpb->Get<GlobalShaderParameters>()->globalTime = globalTime;
    sectionsPipeline->Activate();

    Frustum frustum(projView);
    renderList.clear();
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
        renderList.push_back(section.first);
        renderedFaces += section.second.numOfFaces;
    }
    glm::vec3 playerChunk(GetGameState()->GetPlayer()->pos / 16);
    std::sort(renderList.begin(), renderList.end(), [playerChunk](const Vector& lhs, const Vector& rhs) {
        return glm::distance2(lhs.glm(), playerChunk) < glm::distance2(rhs.glm(), playerChunk);
        });
    for (const auto& renderPos : renderList) {
        sections.at(renderPos).Render();
    }
    DebugInfo::culledSections = culledSections;
    DebugInfo::renderFaces = renderedFaces;

    //Render sky
    glm::mat4 model = glm::mat4(1.0);
    model = glm::translate(model, GetGameState()->GetPlayer()->pos.glm());
    const float scale = 1000000.0f;
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    float shift = GetGameState()->GetTimeStatus().interpolatedTimeOfDay / 24000.0f;
    if (shift < 0)
        shift *= -1.0f;
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(360.0f * shift), glm::vec3(-1.0f, 0.0f, 0.0f));

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

    globalSpb->Get<GlobalShaderParameters>()->dayTime = mixLevel;

    skyPipeline->Activate();
    skyPipeline->SetShaderParameter("model", model);
    skyPipelineInstance->Activate();
    skyPipelineInstance->Render(0, 36);
}

void RendererWorld::PrepareRender(std::shared_ptr<Gal::Framebuffer> target, bool defferedShading) {
    std::string sectionVertexSource, sectionPixelSource;
    {
        auto vertAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/vert/face");
        sectionVertexSource = std::string((char*)vertAsset->data.data(), (char*)vertAsset->data.data() + vertAsset->data.size());

        auto pixelAsset = defferedShading ? AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/face") :
            AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/fwd_face");
        sectionPixelSource = std::string((char*)pixelAsset->data.data(), (char*)pixelAsset->data.data() + pixelAsset->data.size());
    }

    std::string entitiesVertexSource, entitiesPixelSource;
    {
        auto vertAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/vert/entity");
        entitiesVertexSource = std::string((char*)vertAsset->data.data(), (char*)vertAsset->data.data() + vertAsset->data.size());

        auto pixelAsset = defferedShading ? AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/entity") :
            AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/fwd_entity");
        entitiesPixelSource = std::string((char*)pixelAsset->data.data(), (char*)pixelAsset->data.data() + pixelAsset->data.size());
    }

    std::string skyVertexSource, skyPixelSource;
    {
        auto vertAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/vert/sky");
        skyVertexSource = std::string((char*)vertAsset->data.data(), (char*)vertAsset->data.data() + vertAsset->data.size());

        auto pixelAsset = defferedShading ? AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/sky") :
            AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/fwd_sky");
        skyPixelSource = std::string((char*)pixelAsset->data.data(), (char*)pixelAsset->data.data() + pixelAsset->data.size());
    }

    auto gal = Gal::GetImplementation();
    {
        auto sectionsPLC = gal->CreatePipelineConfig();
        sectionsPLC->SetTarget(target);
        sectionsPLC->AddStaticTexture("textureAtlas", AssetManager::GetTextureAtlas());
        sectionsPLC->SetVertexShader(gal->LoadVertexShader(sectionVertexSource));
        sectionsPLC->SetPixelShader(gal->LoadPixelShader(sectionPixelSource));
        sectionsPLC->SetPrimitive(Gal::Primitive::TriangleFan);
        sectionsBufferBinding = sectionsPLC->BindVertexBuffer({
            {"pos", Gal::Type::Vec3, 4, 1},
            {"uv", Gal::Type::Vec2, 4, 1},
            {"light", Gal::Type::Vec2, 4, 1},
            {"normal", Gal::Type::Vec3, 1, 1},
            {"color", Gal::Type::Vec3, 1, 1},
            {"layerAnimationAo", Gal::Type::Vec3, 1, 1},
            });
        sectionsPipeline = gal->BuildPipeline(sectionsPLC);
    }
    
    {
        auto entitiesPLC = gal->CreatePipelineConfig();
        entitiesPLC->SetTarget(target);
        entitiesPLC->AddShaderParameter("model", Gal::Type::Mat4);
        entitiesPLC->AddShaderParameter("entityColor", Gal::Type::Vec3);
        entitiesPLC->SetVertexShader(gal->LoadVertexShader(entitiesVertexSource));
        entitiesPLC->SetPixelShader(gal->LoadPixelShader(entitiesPixelSource));
        entitiesPLC->SetPrimitive(Gal::Primitive::Triangle);
        auto entitiesPosBB = entitiesPLC->BindVertexBuffer({
            {"pos", Gal::Type::Vec3},
            });
        auto entitiesIndicesBB = entitiesPLC->BindIndexBuffer();

        entitiesPipeline = gal->BuildPipeline(entitiesPLC);
        
        constexpr float lw = 0.485f; // line width

        constexpr float vertices[] = {
            0.0f, 0.0f, 0.0f,
            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, lw, 0.5f,
            0.5f, lw, -0.5f,
            0.5f, lw, 0.5f,
            -0.5f, lw, -0.5f,
            0.5f, -lw, -0.5f,
            0.5f, -lw, 0.5f,
            -0.5f, -lw, -0.5f,
            -0.5f, -lw, 0.5f,
            -0.5f, -0.5f, lw,
            0.5f, 0.5f, lw,
            0.5f, lw, lw,
            0.5f, -lw, lw,
            -0.5f, 0.5f, lw,
            0.5f, -0.5f, lw,
            -0.5f, lw, lw,
            -0.5f, -lw, lw,
            -0.5f, 0.5f, -lw,
            0.5f, -0.5f, -lw,
            -0.5f, lw, -lw,
            -0.5f, -lw, -lw,
            -0.5f, -0.5f, -lw,
            0.5f, 0.5f, -lw,
            0.5f, lw, -lw,
            0.5f, -lw, -lw,
            -lw, lw, 0.5f,
            -lw, -lw, 0.5f,
            -lw, lw, -0.5f,
            -lw, -0.5f, lw,
            -lw, -0.5f, -lw,
            -lw, -lw, -0.5f,
            -lw, 0.5f, lw,
            -lw, 0.5f, -lw,
            lw, lw, -0.5f,
            lw, -lw, -0.5f,
            lw, lw, 0.5f,
            lw, 0.5f, lw,
            lw, 0.5f, -lw,
            lw, -lw, 0.5f,
            lw, -0.5f, lw,
            lw, -0.5f, -lw,
        };

        constexpr int indices[] = {
            47, 26, 22,
            7, 33, 43,
            18, 11, 19,
            12, 28, 27,
            31, 13, 10,
            43, 14, 11,
            25, 12, 27,
            20, 4, 22,
            1, 41, 35,
            3, 18, 44,
            9, 24, 16,
            28, 6, 29,
            45, 30, 1,
            11, 20, 19,
            35, 15, 12,
            39, 25, 21,
            2, 6, 38,
            45, 18, 30,
            37, 29, 6,
            8, 4, 46,
            10, 42, 41,
            8, 17, 36,
            9, 34, 33,
            37, 17, 29,
            47, 48, 26,
            11, 3, 43,
            3, 7, 43,
            7, 9, 33,
            1, 30, 10,
            30, 18, 31,
            10, 30, 31,
            18, 3, 11,
            19, 31, 18,
            12, 15, 28,
            31, 32, 13,
            43, 46, 14,
            7, 21, 9,
            21, 25, 23,
            9, 21, 23,
            25, 5, 12,
            27, 23, 25,
            13, 32, 2,
            32, 20, 26,
            2, 32, 26,
            20, 14, 4,
            22, 26, 20,
            12, 5, 35,
            5, 1, 35,
            1, 10, 41,
            44, 39, 3,
            39, 21, 7,
            3, 39, 7,
            9, 23, 24,
            16, 24, 8,
            24, 28, 17,
            8, 24, 17,
            28, 15, 6,
            29, 17, 28,
            1, 5, 40,
            5, 25, 40,
            40, 45, 1,
            11, 14, 20,
            35, 38, 15,
            39, 40, 25,
            15, 38, 6,
            38, 42, 2,
            42, 13, 2,
            45, 44, 18,
            6, 2, 48,
            2, 26, 48,
            48, 37, 6,
            14, 46, 4,
            46, 34, 8,
            34, 16, 8,
            10, 13, 42,
            36, 47, 8,
            47, 22, 4,
            8, 47, 4,
            9, 16, 34,
            37, 36, 17,
        };

        entitiesPosBuffer = gal->CreateBuffer();
        entitiesPosBuffer->SetData({ reinterpret_cast<const std::byte*>(vertices), reinterpret_cast<const std::byte*>(vertices) + sizeof(vertices) });
        entitiesIndexBuffer = gal->CreateBuffer();
        entitiesIndexBuffer->SetData({ reinterpret_cast<const std::byte*>(indices), reinterpret_cast<const std::byte*>(indices) + sizeof(indices) });

        entitiesPipelineInstance = entitiesPipeline->CreateInstance({
            {entitiesPosBB, entitiesPosBuffer},
            {entitiesIndicesBB, entitiesIndexBuffer}
            });
    }

    {
        auto skyPPC = gal->CreatePipelineConfig();
        skyPPC->SetTarget(target);
        skyPPC->AddShaderParameter("sunTexture", Gal::Type::Vec4);
        skyPPC->AddShaderParameter("sunTextureLayer", Gal::Type::Float);
        skyPPC->AddShaderParameter("moonTexture", Gal::Type::Vec4);
        skyPPC->AddShaderParameter("moonTextureLayer", Gal::Type::Float);
        skyPPC->AddShaderParameter("model", Gal::Type::Mat4);
        skyPPC->AddStaticTexture("textureAtlas", AssetManager::GetTextureAtlas());
        skyPPC->SetVertexShader(gal->LoadVertexShader(skyVertexSource));
        skyPPC->SetPixelShader(gal->LoadPixelShader(skyPixelSource));
        auto skyPosUvBB = skyPPC->BindVertexBuffer({
            {"pos", Gal::Type::Vec3},
            {"", Gal::Type::Vec2},
            });

        constexpr float vertices[] = {
            //Z+ Positions       UVs
            -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
            -0.5f, 0.5f, 0.5f,   0.0f, 0.0f,
            0.5f, -0.5f, 0.5f,   1.0f, 0.0f,
            0.5f, -0.5f, 0.5f,   0.0f, 1.0f,
            -0.5f, 0.5f, 0.5f,   1.0f, 0.0f,
            0.5f, 0.5f, 0.5f,    1.0f, 1.0f,

            //Z-
            -0.5f, 0.5f, -0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, 0.5f, -0.5f,   0.0f, 1.0f,

            //X+
            -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
            -0.5f, 0.5f, -0.5f,  1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f,   1.0f, 1.0f,

            //X-
            0.5f, 0.5f, -0.5f,   0.0f, 0.0f,
            0.5f, -0.5f, 0.5f,   1.0f, 1.0f,
            0.5f, 0.5f, 0.5f,    0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, 0.5f,   1.0f, 0.0f,
            0.5f, 0.5f, -0.5f,   1.0f, 1.0f,

            //Y+
            -0.5f, 0.5f, 0.5f,   0.0f, 0.0f,
            0.5f, 0.5f, -0.5f,   1.0f, 1.0f,
            0.5f, 0.5f, 0.5f,    0.0f, 1.0f,
            -0.5f, 0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, 0.5f, -0.5f,   1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f,   1.0f, 1.0f,

            //Y-
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, 0.5f,   0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,  1.0f, 0.0f,
        };

        TextureCoord sunTexture = AssetManager::GetTexture("/minecraft/textures/environment/sun");
        TextureCoord moonTexture = AssetManager::GetTexture("/minecraft/textures/environment/moon_phases");
        moonTexture.w /= 4.0f; // First phase will be fine for now
        moonTexture.h /= 2.0f;

        skyPipeline = gal->BuildPipeline(skyPPC);
        skyPipeline->Activate();
        skyPipeline->SetShaderParameter("sunTexture", glm::vec4(sunTexture.x, sunTexture.y, sunTexture.w, sunTexture.h));
        skyPipeline->SetShaderParameter("sunTextureLayer", static_cast<float>(sunTexture.layer));
        skyPipeline->SetShaderParameter("moonTexture", glm::vec4(moonTexture.x, moonTexture.y, moonTexture.w, moonTexture.h));
        skyPipeline->SetShaderParameter("moonTextureLayer", static_cast<float>(moonTexture.layer));

        skyBuffer = gal->CreateBuffer();
        skyBuffer->SetData({ reinterpret_cast<const std::byte*>(vertices), reinterpret_cast<const std::byte*>(vertices) + sizeof(vertices) });

        skyPipelineInstance = skyPipeline->CreateInstance({
            {skyPosUvBB, skyBuffer}
            });
    }
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

	DebugInfo::readyRenderer = parseQueue.size();
	DebugInfo::renderSections = sections.size();
}
