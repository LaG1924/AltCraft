#pragma once

#include <map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
private:
    const GLchar *vertex;
    const GLchar *fragment;
public:
    GLuint Program;
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr);
    void Use();

    void Reload();
};

class NewShader {
	std::map<std::string, GLuint> uniforms;
	GLuint program = 0;

	GLuint GetUniformLocation(const std::string &name);

public:
	NewShader(const NewShader &) = delete;
	NewShader(NewShader &&other) = delete;
	NewShader &operator=(const NewShader &) = delete;
	NewShader &operator=(NewShader &&other) = delete;

	NewShader(const std::string &vertSource, const std::string &fragSource, const std::vector<std::string> &uniformsNames);
	
	~NewShader();

	void Activate();

	inline void SetUniform(const std::string &name, int val) {
		glUniform1i(GetUniformLocation(name), val);
	}

	inline void SetUniform(const std::string &name, float val) {
		glUniform1f(GetUniformLocation(name), val);
	}

	inline void SetUniform(const std::string &name, glm::vec4 val) {
		glUniform4f(GetUniformLocation(name), val.x, val.y, val.z, val.w);
	}

	inline void SetUniform(const std::string &name, glm::mat4 val) {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(val));
	}
};