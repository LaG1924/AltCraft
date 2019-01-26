#include "Shader.hpp"

#include <fstream>
#include <sstream>

#include <easylogging++.h>

Shader::Shader(const GLchar *vertexPath, const GLchar *fragmentPath, const GLchar *geometryPath) {
	vertex = vertexPath;
	fragment = fragmentPath;
	
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	
	vShaderFile.exceptions(std::ifstream::failbit);
	fShaderFile.exceptions(std::ifstream::failbit);
	gShaderFile.exceptions(std::ifstream::failbit);
	try {		
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		if (geometryPath != nullptr)
			gShaderFile.open(geometryPath);
		std::stringstream vShaderStream, fShaderStream, gShaderStream;
		
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		if (geometryPath != nullptr)
			gShaderStream << gShaderFile.rdbuf();
		
		vShaderFile.close();
		fShaderFile.close();
		if (geometryPath != nullptr)
			gShaderFile.close();
		
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		if (geometryPath != nullptr)
			geometryCode = gShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		LOG(ERROR) << "Shader source not readed";
	}
	const GLchar *vShaderCode = vertexCode.c_str();
	const GLchar *fShaderCode = fragmentCode.c_str();
	const GLchar *gShaderCode = geometryCode.c_str();

	GLuint vertex, fragment, geometry;
	GLint success;
	GLchar infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		LOG(ERROR) << "Vertex shader compilation failed: " << infoLog;
	};

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		LOG(ERROR) << "Fragment shader compilation failed: " << infoLog;
	};

	if (geometryPath != nullptr) {
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			LOG(ERROR) << "Geometry shader compilation failed: " << infoLog;
		};
	}

	this->Program = glCreateProgram();
	glAttachShader(this->Program, vertex);
	glAttachShader(this->Program, fragment);
	if (geometryPath != nullptr)
		glAttachShader(this->Program, geometry);
	glLinkProgram(this->Program);
	glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
		LOG(FATAL) << "Shader program not linked: " << infoLog;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::Use() {
	glUseProgram(this->Program);
}

void Shader::Reload() {
	const GLchar *vertexPath = vertex;
	const GLchar *fragmentPath = fragment;
	this->~Shader();
	new(this) Shader(vertexPath, fragmentPath);
	LOG(INFO) << "Shader is realoded!";
}

GLuint NewShader::GetUniformLocation(const std::string &name) {
	auto it = uniforms.find(name);
	if (it == uniforms.end()) {
		LOG(ERROR) << "Accessed not existing uniform " << name;
		return 0;
	}
	return it->second;
}

NewShader::NewShader(const std::string &vertSource, const std::string &fragSource, const std::vector<std::string> &uniformsNames)
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

NewShader::~NewShader()
{
	if (program)
		glDeleteProgram(program);
}

void NewShader::Activate()
{
	glUseProgram(program);
}
