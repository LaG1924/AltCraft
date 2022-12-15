#pragma once

#include "Vector.hpp"
#include "Gal.hpp"

class RenderState;
class RendererSectionData;

class RendererSection {
    std::shared_ptr<Gal::PipelineInstance> solidPipelineInstance;
    std::shared_ptr<Gal::Buffer> solidBuffer;
    std::shared_ptr<Gal::PipelineInstance> liquidPipelineInstance;
    std::shared_ptr<Gal::Buffer> liquidBuffer;
    Vector sectionPos;
	size_t hash = 0;
    size_t solidFacesCount = 0;
    size_t liquidFacesCount = 0;

    RendererSection(const RendererSection &other) = delete;
public:
    RendererSection(
        const RendererSectionData& data,
        std::shared_ptr<Gal::Pipeline> solidPipeline,
        std::shared_ptr<Gal::BufferBinding> solidBufferBinding,
        std::shared_ptr<Gal::Pipeline> liquidPipeline,
        std::shared_ptr<Gal::BufferBinding> liquidBufferBinding);

    RendererSection(RendererSection &&other);

	~RendererSection();

	void RenderSolid();

    void RenderLiquid();

    Vector GetPosition();

    size_t GetHash();

    inline size_t GetSolidFacesCount() { return solidFacesCount; }

    inline size_t GetLiquidFacesCount() { return liquidFacesCount; }

    friend void swap(RendererSection &lhs, RendererSection &rhs);

	void UpdateData(const RendererSectionData &data);
};