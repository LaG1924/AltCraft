#include "Framebuffer.hpp"
#include "Shader.hpp"
#include <string>
#include "Utility.hpp"

GLuint quadVao, quadVbo;
Shader *quadShader = nullptr;

Framebuffer::Framebuffer(unsigned int width, unsigned int height, bool createDepthStencilBuffer) : width(width), height(height) {
	if (quadShader == nullptr) {
		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &quadVao);
		glGenBuffers(1, &quadVbo);
		glBindVertexArray(quadVao);
		glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		quadShader = new Shader("./shaders/fbo.vs", "./shaders/fbo.fs");
		quadShader->Use();
		glUniform1i(glGetUniformLocation(quadShader->Program, "inputTexture"), 1);
		glActiveTexture(GL_TEXTURE1);
		glCheckError();
	}

	glGenTextures(1, &texColor);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckError();

	if (createDepthStencilBuffer) {
		glGenRenderbuffers(1, &rboDepthStencil);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	}

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);
	if(createDepthStencilBuffer)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);
	glCheckError();

	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Failed to initialize framebuffer: " + std::to_string(framebufferStatus));	
}

Framebuffer::~Framebuffer() {
	if (rboDepthStencil)
		glDeleteRenderbuffers(1, &rboDepthStencil);
	if (texColor)
		glDeleteTextures(1, &texColor);

	glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::Activate() {
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::RenderTo(Framebuffer &target) {
	glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
	glViewport(0, 0, target.width, target.height);
	glBindVertexArray(quadVao);
	glUseProgram(quadShader->Program);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Framebuffer::Resize(unsigned int newWidth, unsigned int newHeight) {
	width = newWidth;
	height = newHeight;
	if (texColor) {
		glBindTexture(GL_TEXTURE_2D, texColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}	
	if (rboDepthStencil) {
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	}
}

Framebuffer &Framebuffer::GetDefault()
{
	static char fboDefaultData[sizeof(Framebuffer)];
	static Framebuffer *fboDefault = nullptr;
	if (fboDefault == nullptr) {
		fboDefault = reinterpret_cast<Framebuffer*>(fboDefaultData);
		fboDefault->fbo = 0;
		fboDefault->width = 1;
		fboDefault->height = 1;
		fboDefault->texColor = 0;
		fboDefault->rboDepthStencil = 0;
	}
	return *fboDefault;
}

void Framebuffer::Clear(bool color, bool depth, bool stencil)
{
	Activate();
	GLbitfield clearBits = 0;
	if (color)
		clearBits |= GL_COLOR_BUFFER_BIT;
	if (depth)
		clearBits |= GL_DEPTH_BUFFER_BIT;
	if (stencil)
		clearBits |= GL_STENCIL_BUFFER_BIT;
	glClear(clearBits);
}
