#include "Shader.hpp"

Shader::Shader(const GLchar *vertexPath, const GLchar *fragmentPath, const GLchar *geometryPath) {
	vertex = vertexPath;
	fragment = fragmentPath;
	// 1. Получаем исходный код шейдера из filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// Удостоверимся, что ifstream объекты могут выкидывать исключения
	vShaderFile.exceptions(std::ifstream::failbit);
	fShaderFile.exceptions(std::ifstream::failbit);
	gShaderFile.exceptions(std::ifstream::failbit);
	try {
		// Открываем файлы
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		if (geometryPath != nullptr)
			gShaderFile.open(geometryPath);
		std::stringstream vShaderStream, fShaderStream, gShaderStream;
		// Считываем данные в потоки
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		if (geometryPath != nullptr)
			gShaderStream << gShaderFile.rdbuf();
		// Закрываем файлы
		vShaderFile.close();
		fShaderFile.close();
		if (geometryPath != nullptr)
			gShaderFile.close();
		// Преобразовываем потоки в массив GLchar
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		if (geometryPath != nullptr)
			geometryCode = gShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		LOG(ERROR) << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ";
	}
	const GLchar *vShaderCode = vertexCode.c_str();
	const GLchar *fShaderCode = fragmentCode.c_str();
	const GLchar *gShaderCode = geometryCode.c_str();

	// 2. Сборка шейдеров
	GLuint vertex, fragment, geometry;
	GLint success;
	GLchar infoLog[512];

	// Вершинный шейдер
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	// Если есть ошибки - вывести их
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		LOG(ERROR) << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog;
	};

	// Вершинный шейдер
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	// Если есть ошибки - вывести их
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		LOG(ERROR) << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog;
	};

	if (geometryPath != nullptr) {
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		// Если есть ошибки - вывести их
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			LOG(ERROR) << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog;
		};
	}

	// Шейдерная программа
	this->Program = glCreateProgram();
	glAttachShader(this->Program, vertex);
	glAttachShader(this->Program, fragment);
	if (geometryPath != nullptr)
		glAttachShader(this->Program, geometry);
	glLinkProgram(this->Program);
	//Если есть ошибки - вывести их
	glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
		LOG(FATAL) << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog;
	}

	// Удаляем шейдеры, поскольку они уже в программу и нам больше не нужны.
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
