#include "ScreenShader.h"

#include "Texture.h"

ScreenShader::ScreenShader(const char* fragShaderPath, const char* vertShaderPath)
{
    // Build and compile shader program
    shader = new Shader();
    shader->attachShader(vertShaderPath, ShaderInfo(ShaderType::kVertex));
    shader->attachShader(fragShaderPath, ShaderInfo(ShaderType::kFragment));
    shader->linkProgram();

    // Configure data
    configureData();
}

ScreenShader::~ScreenShader()
{
    delete shader;
}

void ScreenShader::draw(const Texture& texture)
{
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ScreenShader::draw(const unsigned int textureID)
{
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ScreenShader::configureData()
{
    // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    float quadVertices[24] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // screen quad VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}
