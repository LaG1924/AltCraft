#pragma once

#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/RenderInterface.h>

#include "Renderer.hpp"

class RmlSystemInterface : public Rml::SystemInterface {
	double totalTime;

public:

	virtual double GetElapsedTime() override;

	virtual bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

	inline void Update(double timeToUpdate) {
		totalTime += timeToUpdate;
	}

};

class RmlRenderInterface : public Rml::RenderInterface {
	RenderState* State;

	GLuint Vao, Vbo, Ebo;

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