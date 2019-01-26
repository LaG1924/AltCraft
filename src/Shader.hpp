#pragma once

#include <map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
	std::map<std::string, GLuint> uniforms;
	GLuint program = 0;

	GLuint GetUniformLocation(const std::string &name);

public:
	Shader(const Shader &) = delete;
	Shader(Shader &&other) = delete;
	Shader &operator=(const Shader &) = delete;
	Shader &operator=(Shader &&other) = delete;

	Shader(const std::string &vertSource, const std::string &fragSource, const std::vector<std::string> &uniformsNames);
	
	~Shader();

	void Activate();

	inline void SetUniform(const std::string &name, int val) {
		glUniform1i(GetUniformLocation(name), val);
	}

	inline void SetUniform(const std::string &name, float val) {
		glUniform1f(GetUniformLocation(name), val);
	}

	inline void SetUniform(const std::string &name, glm::vec2 val) {
		glUniform2f(GetUniformLocation(name), val.x, val.y);
	}

	inline void SetUniform(const std::string &name, glm::vec3 val) {
		glUniform3f(GetUniformLocation(name), val.x, val.y, val.z);
	}

	inline void SetUniform(const std::string &name, glm::vec4 val) {
		glUniform4f(GetUniformLocation(name), val.x, val.y, val.z, val.w);
	}

	inline void SetUniform(const std::string &name, glm::mat4 val) {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(val));
	}
};