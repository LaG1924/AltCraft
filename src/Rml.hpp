#pragma once

#include <map>

#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/FileInterface.h>

#include "Renderer.hpp"
#include "Gal.hpp"

class AssetTreeNode;

class RmlSystemInterface : public Rml::SystemInterface {
	double totalTime;
public:

	virtual double GetElapsedTime() override;

	virtual bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

	virtual void SetClipboardText(const Rml::String& text) override;

	virtual void GetClipboardText(Rml::String& text) override;

	inline void Update(double timeToUpdate) {
		totalTime += timeToUpdate;
	}

	std::string clipboard;
};

class RmlRenderInterface : public Rml::RenderInterface {
	RenderState* State;

	std::shared_ptr<Gal::Pipeline> pipeline, texPipeline;
	std::shared_ptr<Gal::PipelineInstance> pipelineInstance, texPipelineInstance;
	std::shared_ptr<Gal::Buffer> vertexBuffer, indexBuffer;

	unsigned int vpWidth, vpHeight;
public:

	RmlRenderInterface(RenderState &renderState);

	RmlRenderInterface(const RmlRenderInterface&) = delete;

	RmlRenderInterface(RmlRenderInterface&&) = delete;

	RmlRenderInterface& operator=(const RmlRenderInterface&) = delete;

	RmlRenderInterface& operator=(RmlRenderInterface&&) = delete;

	~RmlRenderInterface();

	virtual void RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) override;

	virtual void EnableScissorRegion(bool enable) override;

	virtual void SetScissorRegion(int x, int y, int width, int height) override;

	virtual bool LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	
	virtual bool GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override;
	
	virtual void ReleaseTexture(Rml::TextureHandle texture) override;

	void Update(unsigned int windowWidth, unsigned int windowHeight);

};

class RmlFileInterface : public Rml::FileInterface {
	struct AssetHandle {
		std::string fileName;
		unsigned long long filePos;
		AssetTreeNode* assetPtr;
	};
	std::map<Rml::FileHandle, AssetHandle> handles;
public:

	virtual Rml::FileHandle Open(const Rml::String& path) override;

	virtual void Close(Rml::FileHandle file) override;

	virtual size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;

	virtual bool Seek(Rml::FileHandle file, long offset, int origin) override;

	virtual size_t Tell(Rml::FileHandle file) override;

};
