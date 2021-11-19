#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <queue>
#include <memory>
#include <thread>

#include "RendererSection.hpp"
#include "RendererEntity.hpp"
#include "RendererSectionData.hpp"

class Frustum;
class GameState;
class Texture;
class Shader;
class EventListener;
class RenderState;

class RendererWorld {
    struct SectionParsing {
        SectionsData data;
        RendererSectionData renderer;
        bool parsing = false;
    };

    //General
    std::unique_ptr<EventListener> listener;
    size_t numOfWorkers;
    size_t currentWorker = 0;
    std::vector<std::thread> workers;
    void WorkerFunction(size_t WorkerId);
    bool isRunning = true;
    const static size_t parsingBufferSize = 64;
    SectionParsing parsing[parsingBufferSize];
    std::queue<Vector> parseQueue;
    bool parseQueueNeedRemoveUnnecessary = false;
    void ParseQueueUpdate();
    void ParseQeueueRemoveUnnecessary();
    //Blocks
    std::vector<Vector> renderList;
    std::map<Vector, RendererSection> sections;
    void UpdateAllSections(VectorF playerPos);
    std::chrono::time_point<std::chrono::high_resolution_clock> globalTimeStart;
    std::shared_ptr<Gal::Pipeline> sectionsPipeline;
    std::shared_ptr<Gal::BufferBinding> sectionsBufferBinding;
    //Entities
    std::vector<RendererEntity> entities;
    std::shared_ptr<Gal::Pipeline> entitiesPipeline;
    std::shared_ptr<Gal::PipelineInstance> entitiesPipelineInstance;
    std::shared_ptr<Gal::Buffer> entitiesPosBuffer, entitiesIndexBuffer;
    //Sky
    std::shared_ptr<Gal::Pipeline> skyPipeline;
    std::shared_ptr<Gal::PipelineInstance> skyPipelineInstance;
    std::shared_ptr<Gal::Buffer> skyBuffer;
public:
    RendererWorld(std::shared_ptr<Gal::Framebuffer> target);
    ~RendererWorld();

    void Render(RenderState& renderState);
    void PrepareRender(std::shared_ptr<Gal::Framebuffer> target);

    double MaxRenderingDistance;

    void Update(double timeToUpdate);

};
