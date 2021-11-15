#include "RendererSection.hpp"

#include <cstddef>

#include <easylogging++.h>
#include <optick.h>

#include "Utility.hpp"
#include "RendererSectionData.hpp"


RendererSection::RendererSection(const RendererSectionData& data, std::shared_ptr<Gal::Pipeline> pipeline, std::shared_ptr<Gal::BufferBinding> bufferBinding) {
	OPTICK_EVENT();

	auto gal = Gal::GetImplementation();
	buffer = gal->CreateBuffer();

	pipelineInstance = pipeline->CreateInstance({
		{bufferBinding, buffer}
		});
	pipelineInstance->SetInstancesCount(4);
	
	UpdateData(data);
}

RendererSection::RendererSection(RendererSection && other) {
    using std::swap;
    swap(*this, other);
}

RendererSection::~RendererSection() {
    
}

void swap(RendererSection & lhs, RendererSection & rhs) {
    std::swap(lhs.pipelineInstance, rhs.pipelineInstance);
	std::swap(lhs.buffer, rhs.buffer);
    std::swap(lhs.hash, rhs.hash);
    std::swap(lhs.numOfFaces, rhs.numOfFaces);
    std::swap(lhs.sectionPos, rhs.sectionPos);
}

void RendererSection::Render() {
	OPTICK_EVENT();
	pipelineInstance->Activate();
	pipelineInstance->Render(0, numOfFaces);
}

Vector RendererSection::GetPosition() {
    return sectionPos;
}

size_t RendererSection::GetHash() {
    return hash;
}

void RendererSection::UpdateData(const RendererSectionData & data) {
	OPTICK_EVENT();

	buffer->SetData({ reinterpret_cast<const std::byte*>(data.vertices.data()), reinterpret_cast<const std::byte*>(data.vertices.data() + data.vertices.size())});

	numOfFaces = data.vertices.size();
	sectionPos = data.sectionPos;
	hash = data.hash;
}
