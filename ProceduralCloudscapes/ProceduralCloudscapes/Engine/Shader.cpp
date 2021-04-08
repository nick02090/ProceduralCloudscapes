#include "Shader.h"

Shader::Shader()
{
	// create a shader program
	ID = glCreateProgram();
	// initiliaze member variables
	bIsLinked = false;
}

Shader::~Shader()
{
	glDeleteProgram(ID);
}

void Shader::attachShader(const char* shaderPath, ShaderInfo shaderInfo)
{
	// load the shader code
	std::string shaderCode = loadShaderFromFile(shaderPath);
	// compile the shader
	unsigned int shader = compileShader(shaderCode.c_str(), shaderInfo);
	// attach the shader to the program
	glAttachShader(ID, shader);
	// add shader to the list
	shaders.push_back(shader);
}

void Shader::linkProgram()
{
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	bIsLinked = true;
	// delete the shaders as they're linked into program now and no longer necessary
	while (!shaders.empty()) {
		glDeleteShader(shaders.back());
		shaders.pop_back();
	}
}

void Shader::use()
{
	if (bIsLinked)
		glUseProgram(ID);
	else {
		std::cout << "PROGRAM NOT LINKED!" << std::endl;
	}
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, glm::vec3 value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, glm::vec2 value) const
{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR:SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKIN_ERROR of type: " << type << "\n" << infoLog << std::endl;
		}
	}
}

std::string Shader::loadShaderFromFile(const char* shaderPath)
{
	// retrieve the vertex/fragment source code from filePath
	std::string shaderCode;
	std::ifstream shaderFile;
	// ensure ifstream objects can throw exceptions
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open file
		shaderFile.open(shaderPath);
		std::stringstream shaderStream;
		// read file's buffer contents into stream
		shaderStream << shaderFile.rdbuf();
		// close file handler
		shaderFile.close();
		// convert stream into string
		shaderCode = shaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	return shaderCode;
}

unsigned int Shader::compileShader(const char* shaderCode, ShaderInfo shaderInfo)
{
	unsigned int shader;
	shader = glCreateShader(shaderInfo.glType);
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	checkCompileErrors(shader, shaderInfo.name);
	return shader;
}
