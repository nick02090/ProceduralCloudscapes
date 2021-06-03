#include "Terrain.h"

#include <glm/gtx/string_cast.hpp>
#include <imgui.h>

#include "../Engine/Shader.h"
#include "../Engine/GUI/ImGUIExpansions.h"
#include "../Engine/Utilities.h"
#include "../Engine/Environment/SkyboxEnvironment.h"
#include "../Engine/Scene.h"
#include "../Engine/Texture.h"
#include "../Engine/PBRMaterial.h"

Terrain::Terrain(Window* _window) : SceneObject(_window)
{
	// Initialize member variables
	data = new TerrainData();
	data->subdivision = 2;
	data->scale = 2000.f;
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
	data->wireframe = false;
	data->tileSize = 100;
	data->grassCoverage = 0.5f;
	data->snowCoverage = 0.7f;
	data->grassColor = Color(0.06f, 0.25f, 0.03f);
	data->rockColor = Color(0.4f, 0.38f, 0.29f);
	data->snowColor = Color(0.9f, 0.9f, 0.9f);
	data->grassScale = 1.f;
	data->rockScale = 1.f;
	data->snowScale = 1.f;
	data->fogFalloff = 15.f;
	data->fogColor = Color(0.43f, 0.53f, 0.68f);
	data->isRealFog = true;

	// Subscribe to GUI
	window->getGUI()->subscribe(this);

	// Create terrain shader
	shader = new Shader();
	shader->attachShader("Shaders/Terrain/terrain.vert", ShaderInfo(ShaderType::kVertex));
	shader->attachShader("Shaders/Terrain/terrain.tesc", ShaderInfo(ShaderType::kTessControl));
	shader->attachShader("Shaders/Terrain/terrain.tese", ShaderInfo(ShaderType::kTessEvaluation));
	shader->attachShader("Shaders/Terrain/terrain.frag", ShaderInfo(ShaderType::kFragment));
	shader->linkProgram();

	// load and create PBR materials
	grassMaterial = new PBRMaterial("Textures/grass/");
	rockMaterial = new PBRMaterial("Textures/rock/");
	snowMaterial = new PBRMaterial("Textures/snow/");

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
	// Delete materials
	delete grassMaterial;
	delete rockMaterial;
	delete snowMaterial;
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
	shader->setMat4("inverseProjection", glm::inverse(window->getProjectionMatrix()));
	shader->setMat4("inverseView", glm::inverse(camera->getViewMatrix()));

	// Set terrain noise params
	shader->setFloat("terrainNoise.amplitude", data->terrainNoise.amplitude);
	shader->setFloat("terrainNoise.frequency", data->terrainNoise.frequency * data->terrainNoise.frequencyMultiplier);
	shader->setInt("terrainNoise.octaves", data->terrainNoise.octaves);
	shader->setFloat("terrainNoise.lacunarity", data->terrainNoise.lacunarity);
	shader->setFloat("terrainNoise.gain", data->terrainNoise.gain);
	shader->setVec2("terrainNoise.seed", data->terrainNoise.seed);
	shader->setFloat("terrainNoise.power", data->terrainNoise.power);

	// Set terrain grass material
	shader->setSampler("grassAlbedo", *grassMaterial->getAlbedo(), 0);
	shader->setSampler("grassNormal", *grassMaterial->getNormal(), 1);
	shader->setSampler("grassMetallic", *grassMaterial->getMetallic(), 2);
	shader->setSampler("grassRoughness", *grassMaterial->getRoughness(), 3);
	shader->setSampler("grassAO", *grassMaterial->getAO(), 4);
	shader->setVec3("grassBaseColor", data->grassColor.getf());
	shader->setFloat("grassScale", data->grassScale * (data->scale / 1000.f));

	// Set terrain rock material
	shader->setSampler("rockAlbedo", *rockMaterial->getAlbedo(), 5);
	shader->setSampler("rockNormal", *rockMaterial->getNormal(), 6);
	shader->setSampler("rockMetallic", *rockMaterial->getMetallic(), 7);
	shader->setSampler("rockRoughness", *rockMaterial->getRoughness(), 8);
	shader->setSampler("rockAO", *rockMaterial->getAO(), 9);
	shader->setVec3("rockBaseColor", data->rockColor.getf());
	shader->setFloat("rockScale", data->rockScale * (data->scale / 1000.f));

	// Set terrain snow material
	shader->setSampler("snowAlbedo", *snowMaterial->getAlbedo(), 10);
	shader->setSampler("snowNormal", *snowMaterial->getNormal(), 11);
	shader->setSampler("snowMetallic", *snowMaterial->getMetallic(), 12);
	shader->setSampler("snowRoughness", *snowMaterial->getRoughness(), 13);
	shader->setSampler("snowAO", *snowMaterial->getAO(), 14);
	shader->setVec3("snowBaseColor", data->snowColor.getf());
	shader->setFloat("snowScale", data->snowScale * (data->scale / 1000.f));

	// Set terrain coverage values
	shader->setFloat("grassCoverage", data->grassCoverage);
	shader->setFloat("snowCoverage", data->snowCoverage);

	// Set terrain fog values
	shader->setFloat("fogFalloff", data->fogFalloff);
	shader->setVec3("fogColor", data->fogColor.getf());
	shader->setBool("isRealFog", data->isRealFog);

	// Set sky info
	SkyboxEnvironment* env = getScene()->getEnvironment<SkyboxEnvironment>();
	if (env != nullptr) {
		shader->setFloat("sunAltitude", env->getSunAltitude());
		shader->setFloat("sunAzimuth", env->getSunAzimuth());
		shader->setFloat("sunIntensity", env->getSunIntensity());
	}
	else {
		std::cout << "ERROR::CLOUDS::update() Clouds should be rendered only using Skybox environment!" << std::endl;
	}

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
		ImGui::SliderFloat("Scale", &scale, 100.f, 5000.f);
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

	// Crete terrain post-processing header
	if (ImGui::CollapsingHeader("Post-processing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Fog falloff
		float fogFalloff = getFogFalloff();
		ImGui::SliderFloat("Fog falloff", &fogFalloff, 0.f, 100.f);
		setFogFalloff(fogFalloff);

		// Fog color
		ImVec4 fogColor = getFogColor().toIMGUI();
		ImGui::ColorEdit3("Fog color", (float*)&fogColor);
		setFogColor(Color::fromIMGUI(fogColor));

		// Realistic fog
		bool isRealFog = getIsRealFog();
		imgui_exp::ToggleButton("Realistic fog", &isRealFog);
		setIsRealFog(isRealFog);
	}

	// Create terrain color header
	if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Grass coverage
		float grassCoverage = getGrassCoverage();
		ImGui::SliderFloat("Grass coverage", &grassCoverage, 0.f, 1.f);
		setGrassCoverage(grassCoverage);

		// Snow coverage
		float snowCoverage = getSnowCoverage();
		ImGui::SliderFloat("Snow coverage", &snowCoverage, 0.f, 1.f);
		setSnowCoverage(snowCoverage);

		// Grass scale
		float grassScale = getGrassScale();
		ImGui::SliderFloat("Grass scale", &grassScale, 0.f, 100.f);
		setGrassScale(grassScale);

		// Rock scale
		float rockScale = getRockScale();
		ImGui::SliderFloat("Rock scale", &rockScale, 0.f, 100.f);
		setRockScale(rockScale);

		// Snow scale
		float snowScale = getSnowScale();
		ImGui::SliderFloat("Snow scale", &snowScale, 0.f, 100.f);
		setSnowScale(snowScale);

		// Grass color
		ImVec4 grassColor = getGrassColor().toIMGUI();
		ImGui::ColorEdit3("Grass color", (float*)&grassColor);
		setGrassColor(Color::fromIMGUI(grassColor));

		// Rock color
		ImVec4 rockColor = getRockColor().toIMGUI();
		ImGui::ColorEdit3("Rock color", (float*)&rockColor);
		setRockColor(Color::fromIMGUI(rockColor));

		// Snow color
		ImVec4 snowColor = getSnowColor().toIMGUI();
		ImGui::ColorEdit3("Snow color", (float*)&snowColor);
		setSnowColor(Color::fromIMGUI(snowColor));
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
		ImGui::SliderFloat("Frequency multiplier", &frequencyMultiplier, .1f, 10.0f);
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

		// New seed
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.3f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.3f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.3f, 0.8f, 0.8f));
		if (ImGui::Button("New seed")) {
			data->terrainNoise.seed = glm::vec2(util::random(), util::random());
		}
		ImGui::PopStyleColor(3);

		// Default seed (0.0)
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.6f, 0.8f, 0.8f));
		ImGui::SameLine();
		if (ImGui::Button("Default seed")) {
			data->terrainNoise.seed = glm::vec2(0.0);
		}
		ImGui::PopStyleColor(3);
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
	// Initial grid and position generation
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
	// Calculate current tile (based on the camera position)
	glm::vec2 currentTile = calculateCurrentCameraTile();

	// Recalculate grid position data based on current tile (results in infinite terrain)
	glm::vec2 center = positionBufferData[static_cast<unsigned int>(data->tileSize / 2.f + (data->tileSize / 2.f) * data->tileSize)];
	for (glm::vec2& position : positionBufferData) {
		position += currentTile - center;
	}

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

glm::vec2 Terrain::calculateCurrentCameraTile()
{
	// Get camera
	Camera* camera = window->getCamera();
	glm::vec3 cameraPosition = camera->getPosition();

	// Prepare final result
	glm::vec2 cameraTile = glm::vec2(0.f);

	// Iterate over every position
	for (glm::vec2 position : positionBufferData) {

		// Set initial values as false
		bool inX = false;	// true if camera's x position corresponds to this tile's x position
		bool inY = false;	// true if camera's y position corresponds to this tile's y position

		// Check for camera's x position
		if (cameraPosition.x <= position.x + data->scale &&
			cameraPosition.x >= position.x - data->scale) {
			inX = true;
		}

		// Check for camera's y position
		// NOTE: When projecting 3D to 2D: x[3D] represents x[2D], whilst z[3D] represents y[2D]
		if (cameraPosition.z <= position.y + data->scale &&
			cameraPosition.z >= position.y - data->scale) {
			inY = true;
		}

		// If camera's x and y coordinates correspond to this tile then quit the search, we have found our tile!
		if (inX && inY) {
			cameraTile = position;
			break;
		}
	}

	// Return final result
	return cameraTile;
}
