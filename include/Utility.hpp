#pragma once

#include <algorithm>

#include <GL/glew.h>

template<class T>
void endswap(T *objp) {
	unsigned char *memp = reinterpret_cast<unsigned char *>(objp);
	std::reverse(memp, memp + sizeof(T));
}

template<class T>
void endswap(T &obj) {
	unsigned char *raw = reinterpret_cast<unsigned char *>(&obj);
	std::reverse(raw, raw + sizeof(T));
}

inline void endswap(unsigned char *arr, size_t arrLen) {
	std::reverse(arr, arr + arrLen);
}

inline GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
			case GL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				error = "STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				error = "STACK_UNDERFLOW";
				break;
			case GL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error = "INVALID_FRAMEBUFFER_OPERATION";
				break;
		}
		LOG(ERROR) << "OpenGL error: " << error << " at " << file << ":" << line;
	}
	return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)