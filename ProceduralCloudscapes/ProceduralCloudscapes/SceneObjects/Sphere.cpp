#include "Sphere.h"

#include "../Engine/Scene.h"
#include "../Engine/Environment/SkyboxEnvironment.h"

Sphere::Sphere(Window* _window, const char* texturesPath, glm::vec3 _translate) : SceneObject(_window)
{
    // initialize member variables
    translate = _translate;

    // build and compile shader programs
    shader = new Shader();
    shader->attachShader("Shaders/PBR/PBR.vert", ShaderInfo(ShaderType::kVertex));
    shader->attachShader("Shaders/PBR/PBR.frag", ShaderInfo(ShaderType::kFragment));
    shader->linkProgram();

    // generate paths for the textures
    std::string baseTexturePath(texturesPath);
    // albedo
    std::string albedoPath(baseTexturePath);
    albedoPath.append("albedo.png");
    // normal
    std::string normalPath(baseTexturePath);
    normalPath.append("normal.png");
    // metallic
    std::string metallicPath(baseTexturePath);
    metallicPath.append("metallic.png");
    // roughness
    std::string roughnessPath(baseTexturePath);
    roughnessPath.append("roughness.png");
    // ao
    std::string aoPath(baseTexturePath);
    aoPath.append("ao.png");

    // load and create textures
    albedoTex = new Texture(albedoPath.c_str());
    normalTex = new Texture(normalPath.c_str());
    metallicTex = new Texture(metallicPath.c_str());
    roughnessTex = new Texture(roughnessPath.c_str());
    aoTex = new Texture(aoPath.c_str());

    // configure sphere data
    configureData();
}

Sphere::~Sphere()
{
    // delete shader
    delete shader;
    // delete textures
    delete albedoTex;
    delete normalTex;
    delete metallicTex;
    delete roughnessTex;
    delete aoTex;
}

void Sphere::update()
{
    Camera* camera = window->getCamera();

    shader->use();

    // bind camera info
    shader->setMat4("projection", window->getProjectionMatrix());
    shader->setMat4("view", camera->getViewMatrix());
    shader->setVec3("camPos", camera->getPosition());

    // bind textures
    shader->setSampler("albedoMap", *albedoTex, 0);
    shader->setSampler("normalMap", *normalTex, 1);
    shader->setSampler("metallicMap", *metallicTex, 2);
    shader->setSampler("roughnessMap", *roughnessTex, 3);
    shader->setSampler("aoMap", *aoTex, 4);

    // bind sphere info
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translate);
    shader->setMat4("model", model);

    // set shaders sky info
    SkyboxEnvironment* env = getScene()->getEnvironment<SkyboxEnvironment>();
    if (env != nullptr) {
        shader->setFloat("sunAltitude", env->getSunAltitude());
        shader->setFloat("sunAzimuth", env->getSunAzimuth());
        shader->setFloat("sunIntensity", env->getSunIntensity());
    }
    else {
        std::cout << "ERROR::CLOUDS::update() Clouds should be rendered only using Skybox environment!" << std::endl;
    }

    // render sphere
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
}

void Sphere::configureData()
{
    glGenVertexArrays(1, &sphereVAO);

    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    indexCount = indices.size();

    std::vector<float> data;
    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
    }
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    float stride = (3 + 2 + 3) * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(5 * sizeof(float)));
}
