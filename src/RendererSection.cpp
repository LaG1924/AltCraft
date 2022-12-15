#include "RendererSection.hpp"

#include <cstddef>

#include <easylogging++.h>
#include <optick.h>

#include "Utility.hpp"
#include "RendererSectionData.hpp"


RendererSection::RendererSection(const RendererSectionData& data,
	std::shared_ptr<Gal::Pipeline> solidPipeline,
	std::shared_ptr<Gal::BufferBinding> solidBufferBinding,
	std::shared_ptr<Gal::Pipeline> liquidPipeline,
	std::shared_ptr<Gal::BufferBinding> liquidBufferBinding) {
	OPTICK_EVENT();

	auto gal = Gal::GetImplementation();

	solidBuffer = gal->CreateBuffer();
	solidPipelineInstance = solidPipeline->CreateInstance({
		{solidBufferBinding, solidBuffer}
		});
	solidPipelineInstance->SetInstancesCount(4);

	liquidBuffer = gal->CreateBuffer();
	liquidPipelineInstance = liquidPipeline->CreateInstance({
		{liquidBufferBinding, liquidBuffer}
		});
	liquidPipelineInstance->SetInstancesCount(4);
	
	UpdateData(data);
}

RendererSection::RendererSection(RendererSection && other) {
    using std::swap;
    swap(*this, other);
}

RendererSection::~RendererSection() {
    
}

void RendererSection::RenderSolid() {
	OPTICK_EVENT();
	solidPipelineInstance->Activate();
	solidPipelineInstance->Render(0, solidFacesCount);
}

void RendererSection::RenderLiquid() {
	OPTICK_EVENT();
	liquidPipelineInstance->Activate();
	liquidPipelineInstance->Render(0, liquidFacesCount);
}

void swap(RendererSection & lhs, RendererSection & rhs) {
    std::swap(lhs.solidPipelineInstance, rhs.solidPipelineInstance);
	std::swap(lhs.solidBuffer, rhs.solidBuffer);
	std::swap(lhs.liquidPipelineInstance, rhs.liquidPipelineInstance);
	std::swap(lhs.liquidBuffer, rhs.liquidBuffer);
    std::swap(lhs.hash, rhs.hash);
    std::swap(lhs.solidFacesCount, rhs.solidFacesCount);
    std::swap(lhs.liquidFacesCount, rhs.liquidFacesCount);
    std::swap(lhs.sectionPos, rhs.sectionPos);
}

Vector RendererSection::GetPosition() {
    return sectionPos;
}

size_t RendererSection::GetHash() {
    return hash;
}

void RendererSection::UpdateData(const RendererSectionData & data) {
	OPTICK_EVENT();

	solidBuffer->SetData({ reinterpret_cast<const std::byte*>(data.solidVertices.data()), reinterpret_cast<const std::byte*>(data.solidVertices.data() + data.solidVertices.size())});
	solidFacesCount = data.solidVertices.size();

	liquidBuffer->SetData({ reinterpret_cast<const std::byte*>(data.liquidVertices.data()), reinterpret_cast<const std::byte*>(data.liquidVertices.data() + data.liquidVertices.size()) });
	liquidFacesCount = data.liquidVertices.size();

	sectionPos = data.sectionPos;
	hash = data.hash;
}
