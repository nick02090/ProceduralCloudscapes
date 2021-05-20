#include "Terrain.h"

#include "../Engine/Shader.h"
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>
#include "../Engine/GUI/ImGUIExpansions.h"
#include "../Engine/Utilities.h"

Terrain::Terrain(Window* _window) : SceneObject(_window)
{
	// Initialize member variables
	data = new TerrainData();
	data->subdivision = 2;
	data->scale = 1000.f;
	data->terrainNoise = {
		14.f,	// amplitude
		5e-5f,	// frequency
		13,		// octaves
		2.f,	// lacunarity
		0.5f,	// gain
		glm::vec2(0.f, 0.f),	// seed
		3.f,	// power
		1.f		// frequency multiplier
	};
	data->wireframe = true;
	data->tileSize = 100;

	// Subscribe to GUI
	window->getGUI()->subscribe(this);

	// Create terrain shader
	shader = new Shader();
	shader->attachShader("Shaders/Terrain/terrain.vert", ShaderInfo(ShaderType::kVertex));
	shader->attachShader("Shaders/Terrain/terrain.tesc", ShaderInfo(ShaderType::kTessControl));
	shader->attachShader("Shaders/Terrain/terrain.tese", ShaderInfo(ShaderType::kTessEvaluation));
	shader->attachShader("Shaders/Terrain/terrain.frag", ShaderInfo(ShaderType::kFragment));
	shader->linkProgram();

	// Generate VAO, VBO and EBO
	glGenVertexArrays(1, &terrainVAO);
	glGenBuffers(1, &terrainVBO);
	glGenBuffers(1, &terrainEBO);

	// Generate position buffer
	glGenBuffers(1, &positionBuffer);

	// Initialize position buffer data size
	positionBufferData.resize(data->tileSize * data->tileSize);

	// Prepare data for terrain generation
	generateTerrainData();
}

Terrain::~Terrain()
{
	// Delete shader
	delete shader;
	// Delete data
	delete data;
	// Delete buffers
	glDeleteVertexArrays(1, &terrainVAO);
	glDeleteBuffers(1, &terrainVBO);
	glDeleteBuffers(1, &terrainEBO);
	glDeleteBuffers(1, &positionBuffer);
}

void Terrain::update()
{
	Camera* camera = window->getCamera();

	// Prepare data for terrain generation
	updatePositionData();

	// Enable wireframe rendering if set
	if (data->wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Configure shader data
	shader->use();

	// Set camera info
	shader->setVec3("cameraPosition", camera->getPosition());
	shader->setMat4("gVP", window->getProjectionMatrix() * camera->getViewMatrix());

	// Set terrain noise params
	shader->setFloat("terrainNoise.amplitude", data->terrainNoise.amplitude);
	shader->setFloat("terrainNoise.frequency", data->terrainNoise.frequency * data->terrainNoise.frequencyMultiplier);
	shader->setInt("terrainNoise.octaves", data->terrainNoise.octaves);
	shader->setFloat("terrainNoise.lacunarity", data->terrainNoise.lacunarity);
	shader->setFloat("terrainNoise.gain", data->terrainNoise.gain);
	shader->setVec2("terrainNoise.seed", data->terrainNoise.seed);
	shader->setFloat("terrainNoise.power", data->terrainNoise.power);

	// Draw the terrain
	size_t resolution = getResolution();
	glBindVertexArray(terrainVAO);
	glDrawElementsInstanced(GL_PATCHES, static_cast<GLsizei>((resolution - 1) * (resolution - 1) * 2 * 3), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(positionBufferData.size()));
	glBindVertexArray(0);

	// Disable wireframe mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Terrain::buildGUI()
{
	// Create the terrain control window
	ImGui::Begin("Terrain");

	// Create terrain main header
	if (ImGui::CollapsingHeader("Main", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Wireframe mode switch
		bool isWireframe = getWireframe();
		imgui_exp::ToggleButton("Wireframe", &isWireframe);
		setWireframe(isWireframe);

		bool generateNewTerrainData = false;

		// Scale
		float scale = getScale();
		ImGui::SliderFloat("Scale", &scale, 100.f, 2000.f);
		if (scale != getScale()) generateNewTerrainData = true;
		setScale(scale);

		// Subdivisions
		int subdivisions = static_cast<int>(getSubdivisions());
		ImGui::SliderInt("Subdivisions", &subdivisions, 1, 10);
		if (subdivisions != getSubdivisions()) generateNewTerrainData = true;
		setSubdivisions(subdivisions);

		if (generateNewTerrainData) {
			generateTerrainData();
			generateNewTerrainData = false;
		}
	}

	// Create terrain color header
	if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// TODO: After textures are applied on the terrain, enable color switching for them
	}

	// Create terrain noise header
	if (ImGui::CollapsingHeader("Noise", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Amplitude
		float amplitude = data->terrainNoise.amplitude;
		ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 30.0f);
		data->terrainNoise.amplitude = amplitude;

		// Frequency multiplier
		float frequencyMultiplier = data->terrainNoise.frequencyMultiplier;
		ImGui::SliderFloat("Frequency multiplier", &frequencyMultiplier, 1.0f, 10.0f);
		data->terrainNoise.frequencyMultiplier = frequencyMultiplier;

		// Octaves
		int octaves = data->terrainNoise.octaves;
		ImGui::SliderInt("Octaves", &octaves, 1, 20);
		data->terrainNoise.octaves = octaves;

		// Lacunarity
		float lacunarity = data->terrainNoise.lacunarity;
		ImGui::SliderFloat("Lacunarity", &lacunarity, 1.1f, 10.0f);
		data->terrainNoise.lacunarity = lacunarity;

		// Gain
		float gain = data->terrainNoise.gain;
		ImGui::SliderFloat("Gain", &gain, 0.1f, 0.9f);
		data->terrainNoise.gain = gain;

		// Power
		float power = data->terrainNoise.power;
		ImGui::SliderFloat("Power", &power, 1.0f, 10.0f);
		data->terrainNoise.power = power;

		// Seed
		if (ImGui::Button("New seed")) {
			data->terrainNoise.seed = glm::vec2(util::random(), util::random());
		}
		ImGui::SameLine();
		if (ImGui::Button("Default seed")) {
			data->terrainNoise.seed = glm::vec2(0.0);
		}
	}

	// Finish the window
	ImGui::End();
}

void Terrain::generateTerrainData()
{
	size_t resolution = getResolution();
	float scale = getScale();

	// ================================================
	// Vertices and triangles generation
	// ================================================

	// Calculate number of vertices
	size_t numberOfVertices = resolution * resolution;
	// Each vertex has a position (vec3), normal (vec3) and a texture coordinate (vec2)
	size_t vertexBufferSize = numberOfVertices * 3 + numberOfVertices * 3 + numberOfVertices * 2;
	float* vertexBufferData = new float[vertexBufferSize];

	// Generate vertices for the terrain
	for (int i = 0; i < resolution; ++i) {
		for (int j = 0; j < resolution; ++j) {
			// Prepare index information (2D array projected to 1D array with offset of number of data per vertex (8))
			size_t index = (i + resolution * j) * 8;

			// Calculate position for the vertex
			vertexBufferData[index + 0] = j * (float)scale / (resolution - 1) - scale / 2.f;	// position.x
			vertexBufferData[index + 1] = 0.f;	// position.y
			vertexBufferData[index + 2] = -i * (float)scale / (resolution - 1) + scale / 2.f;	// position.z

			// Calculate normals for the vertex
			vertexBufferData[index + 3] = 0.f;	// normal.x
			vertexBufferData[index + 4] = 1.f;	// normal.y
			vertexBufferData[index + 5] = 0.f;	// normal.z

			// Calculate texture coordinates for the vertex
			vertexBufferData[index + 6] = (float)j / (resolution - 1);	// texcoord.x
			vertexBufferData[index + 7] = (float)(resolution - i - 1) / (resolution - 1);	// texcoord.y
		}
	}

	// Calculate number of triangles
	size_t numberOfTriangles = (resolution - 1) * (resolution - 1) * 2;
	size_t trianglesPerRow = 2 * (resolution - 1);
	// Each triangle has 3 vertices
	size_t triangleBufferSize = numberOfTriangles * 3;
	unsigned int* triangleBufferData = new unsigned int[triangleBufferSize];

	// Generate triangles for the terrain
	for (int i = 0; i < numberOfTriangles; ++i) {
		for (int j = 0; j < trianglesPerRow; ++j) {
			// Prepare index information (offset of number of data per triangle (3))
			int index = i * 3;
			// Calculate row and column indices
			size_t row = i / trianglesPerRow;
			size_t col = (i % trianglesPerRow) / 2;

			// ###########################
			//   top_left     top_right
			//    (0, 1)       (1, 1)
			//       .___________.
			//       | \   top   |
			//       |   \ tris  |
			//       |     \     |
			//       | btm   \   |
			//       | tris    \ |
			//       .___________.
			//    (0, 0)       (1, 0)
			// bottom_left  bottom_right
			// ##########################

			// Calculate top triangle (ccw)
			if (i % 2 == 0) {
				triangleBufferData[index + 0] = static_cast<unsigned int>(row * resolution + (col + 1));	// (0, 1) - top_left
				triangleBufferData[index + 1] = static_cast<unsigned int>((row + 1) * resolution + col);	// (1, 0) - bottom_right
				triangleBufferData[index + 2] = static_cast<unsigned int>((row + 1) * resolution + (col + 1));	// (1, 1) - top_right
			}
			// Calculate bottom triangle (ccw)
			else {
				triangleBufferData[index + 0] = static_cast<unsigned int>(row * resolution + col);	// (0, 0) - bottom_left
				triangleBufferData[index + 1] = static_cast<unsigned int>((row + 1) * resolution + col);	// (1, 0) - bottom_right
				triangleBufferData[index + 2] = static_cast<unsigned int>(row * resolution + (col + 1));	// (0, 1) - top_left
			}
		}
	}

	// Bind VAO
	glBindVertexArray(terrainVAO);

	// Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize * sizeof(float), vertexBufferData, GL_STATIC_DRAW);

	// Bind EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleBufferSize * sizeof(unsigned int), triangleBufferData, GL_STATIC_DRAW);

	// Set data organization
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind to default buffer
	glBindVertexArray(0);

	// Delete buffers data
	delete[] vertexBufferData;
	delete[] triangleBufferData;

	// ================================================
	// Position and LOD generation
	// ================================================

	// Generate tile grid
	glm::vec2 I = glm::vec2(scale, 0);
	glm::vec2 J = glm::vec2(0, scale);
	for (int i = 0; i < data->tileSize; ++i) {
		for (int j = 0; j < data->tileSize; ++j) {
			positionBufferData[j + i * data->tileSize] = (j - data->tileSize / 2.f) * I + (i - data->tileSize / 2.f) * J;
		}
	}

	// Update position data with new positions
	updatePositionData();
}

void Terrain::updatePositionData()
{
	// Bind position buffer and forward its data
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, positionBufferData.size() * sizeof(glm::vec2), &positionBufferData[0], GL_STATIC_DRAW);

	// Bind VAO for location 3 (position)
	glBindVertexArray(terrainVAO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glVertexAttribDivisor(3, 1);

	// Unbind to default buffer
	glBindVertexArray(0);
}
