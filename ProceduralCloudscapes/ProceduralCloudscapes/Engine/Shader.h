#ifndef SHADER_H
#define SHADER_H

// include glad to get all the required OpenGL headers
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>

class Texture;

enum class ShaderType {
	kVertex = 0,
	kFragment = 1,
	kTessEvaluation = 2,
	kTessControl = 3,
	kGeometry = 4,
	kCompute = 5
};

struct ShaderInfo {
	ShaderInfo(ShaderType _type) : type(_type) {
		switch (type)
		{
		case ShaderType::kVertex:
			name = "VERTEX";
			glType = GL_VERTEX_SHADER;
			break;
		case ShaderType::kFragment:
			name = "FRAGMENT";
			glType = GL_FRAGMENT_SHADER;
			break;
		case ShaderType::kTessEvaluation:
			name = "TESS_EVALUATION";
			glType = GL_TESS_EVALUATION_SHADER;
			break;
		case ShaderType::kTessControl:
			name = "TESS_CONTROL";
			glType = GL_TESS_CONTROL_SHADER;
			break;
		case ShaderType::kGeometry:
			name = "GEOMETRY";
			glType = GL_GEOMETRY_SHADER;
			break;
		case ShaderType::kCompute:
			name = "COMPUTE";
			glType = GL_COMPUTE_SHADER;
			break;
		default:
			name = "";
			glType = -1;
			break;
		}
	}
	ShaderType type;
	unsigned int glType;
	std::string name;
};

class Shader
{
public:
	// the program ID
	unsigned int ID;

	// constructor creates the shader program
	Shader();
	~Shader();

	// attaches a shader code script to the program
	void attachShader(const char* shaderPath, ShaderInfo shaderInfo);
	// links program
	void linkProgram();
	// use/activate the shader
	void use();

	// utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setMat4(const std::string& name, glm::mat4 value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec2(const std::string& name, glm::vec2 value) const;
	void setSampler(const std::string& name, const Texture& texture, GLenum unit);
private:
	// utility function for checking shader compilation/linking errros
	void checkCompileErrors(unsigned int shader, std::string type);

	// functions for reading and compiling shader code script
	std::string loadShaderFromFile(const char* shaderPath);
	unsigned int compileShader(const char* shaderCode, ShaderInfo shaderInfo);

	// stored shaders
	std::list<unsigned int> shaders;
	bool bIsLinked;
};

#endif