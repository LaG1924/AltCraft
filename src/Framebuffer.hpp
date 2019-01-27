#pragma once

#include <GL/glew.h>

class Framebuffer {
	unsigned int width, height;
	GLuint fbo, texColor = 0, rboDepthStencil = 0;
public:
	Framebuffer(unsigned int width, unsigned int height, bool createDepthStencilBuffer);
	~Framebuffer();	
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer(Framebuffer &&) = delete;
	Framebuffer &operator=(const Framebuffer &) = delete;
	Framebuffer &operator=(Framebuffer &&) = delete;

	void Activate();

	void RenderTo(Framebuffer &target);

	void Resize(unsigned int newWidth, unsigned int newHeight);

	inline GLuint GetColor() {
		return texColor;
	}

	static Framebuffer &GetDefault();

	void Clear(bool color = true, bool depth = true, bool stencil = true);
};
