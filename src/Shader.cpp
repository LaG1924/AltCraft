#include "Shader.hpp"

#include <fstream>
#include <sstream>

#include <easylogging++.h>


GLuint Shader::GetUniformLocation(const std::string &name) {
	auto it = uniforms.find(name);
	if (it == uniforms.end()) {
		LOG(ERROR) << "Accessed not existing uniform " << name;
		return 0;
	}
	return it->second;
}

Shader::Shader(const std::string &vertSource, const std::string &fragSource, const std::vector<std::string> &uniformsNames)
{
	bool vertFailed = false, fragFailed = false, linkFailed = false, uniformsFailed = false;
	const GLchar *vertSourcePtr = vertSource.c_str();
	const GLchar *fragSourcePtr = fragSource.c_str();

	GLuint vertex, fragment;
	GLint success;
	GLchar infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertSourcePtr, NULL);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		LOG(ERROR) << "Vertex shader compilation failed: " << std::endl << infoLog;
		vertFailed = true;
	};

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragSourcePtr, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		LOG(ERROR) << "Fragment shader compilation failed: " << std::endl << infoLog;
		fragFailed = true;
	};

	if (vertFailed || fragFailed)
		throw std::runtime_error("Shaders not compiled");

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);	
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		LOG(ERROR) << "Shader program not linked: " << std::endl << infoLog;
		linkFailed = true;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	if (linkFailed)
		throw std::runtime_error("Shader not linked");

	glUseProgram(program);

	for (auto &it : uniformsNames) {
		GLuint location = glGetUniformLocation(program, it.c_str());
		if (location == -1) {
			glDeleteProgram(program);
			LOG(ERROR) << "Uniform name \"" << it << "\" not found in shader";
			throw std::runtime_error("Invalid uniform");
		}

		uniforms[it] = location;
	}
}

Shader::~Shader()
{
	if (program)
		glDeleteProgram(program);
}

void Shader::Activate()
{
	glUseProgram(program);
}
