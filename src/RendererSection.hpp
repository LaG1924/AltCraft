#pragma once

#include "Vector.hpp"
#include "Gal.hpp"

class RenderState;
class RendererSectionData;

class RendererSection {
    std::shared_ptr<Gal::PipelineInstance> pipelineInstance;
    std::shared_ptr<Gal::Buffer> buffer;
	size_t hash;
    Vector sectionPos;

    RendererSection(const RendererSection &other) = delete;
public:
    RendererSection(const RendererSectionData& data, std::shared_ptr<Gal::Pipeline> pipeline, std::shared_ptr<Gal::BufferBinding> bufferBinding);

    RendererSection(RendererSection &&other);

	~RendererSection();

	void Render();

    Vector GetPosition();

    size_t GetHash();

    size_t numOfFaces;

    friend void swap(RendererSection &lhs, RendererSection &rhs);

	void UpdateData(const RendererSectionData &data);
};