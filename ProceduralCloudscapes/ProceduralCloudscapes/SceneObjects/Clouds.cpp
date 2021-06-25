#include "Clouds.h"

#include "../Engine/ScreenShader.h"
#include "../Engine/Scene.h"
#include "../Engine/Environment/SkyboxEnvironment.h"
#include "../Engine/Environment/ColorEnvironment.h"
#include "../Engine/FrameBufferObject.h"
#include "../Engine/GUI/ImGUIExpansions.h"

static const char* cloudTypes[] = { "Cumulus", "Stratus", "Stratocumulus", "Cumulonimbus", "Mix" };

Clouds::Clouds(Window* _window) : SceneObject(_window)
{
	// Initialize member variables
	data = new CloudsData();

	// Initialize clouds shape properties
	data->globalCoverage = 0.3f;
	data->globalDensity = 0.5f;
	data->anvilAmount = 0.f;
	data->cloudsType = CloudsType::Cumulus;
	data->isBaseShape = false;

	// Initialize clouds animation properties
	data->windDirection = glm::vec3(0.5f, 1.0f, 0.1f);
	data->cloudSpeed = 350.f;
	data->edgesSpeedMultiplier = 1.f;

	// Initialize clouds lighting properties
	data->beerCoeff = 1.0f;
	data->enablePowder = true;
	data->powderCoeff = 10.0f;
	data->csi = 2.5f;
	data->color = Color(1.f, 1.f, 1.f);

	// framebuffer configuration
	framebuffer = new FrameBufferObject();
	// create a color attachment texture
	framebuffer->attachColorTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	framebuffer->attachDepthTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (!framebuffer->checkStatus())
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	// Generate textures for shader program
	generateNoiseTextures();

	// Create weather map shader
	weatherMapShader = new Shader();
	weatherMapShader->attachShader("Shaders/Clouds/weatherMap.comp", ShaderInfo(ShaderType::kCompute));
	weatherMapShader->linkProgram();

	// Create weather map texture
	weatherMapTex = new Texture(TextureType::twoDimensional, glm::vec3(1024.f, 1024.f, 0.f), 4, true);

	// Generate weather map
	generateWeatherMap();

	// Build and compile the shader program
	cloudsShader = new ScreenShader("Shaders/Clouds/clouds.frag");

	// Subscribe to GUI
	window->getGUI()->subscribe(this);

	// Subscribe to key reaction
	window->subscribeToKeyReaction(this);
}

Clouds::~Clouds()
{
	// delete noise textures
	delete perlinWorleyTex;
	delete worleyTex;
	delete curlTex;
	// delete weather map items
	delete weatherMapTex;
	delete weatherMapShader;
	// delete clouds shader
	delete cloudsShader;
	// delete framebuffer
	delete framebuffer;
}

void Clouds::update()
{
	Camera* camera = window->getCamera();

	// wait for all the memory stores, loads, textures fetches, vertex fetches
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// bind the framebuffer
	framebuffer->bind();
	// clear its data before any rendering
	framebuffer->clear();

	// configure shader data
	Shader* shader = cloudsShader->getShader();
	shader->use();

	// set shaders camera info
	shader->setVec3("cameraPosition", camera->getPosition());
	shader->setMat4("inverseProjection", glm::inverse(window->getProjectionMatrix()));
	shader->setMat4("inverseView", glm::inverse(camera->getViewMatrix()));
	shader->setVec2("resolution", window->getSize());

	// set shaders sky info
	SkyboxEnvironment* env = getScene()->getEnvironment<SkyboxEnvironment>();
	if (env != nullptr) {
		shader->setFloat("sunAltitude", env->getSunAltitude());
		shader->setFloat("sunAzimuth", env->getSunAzimuth());
		shader->setFloat("sunIntensity", env->getSunIntensity());
		shader->setVec3("sunColorDay", env->getSunColorDay().getf());
		shader->setVec3("sunColorSunset", env->getSunColorSunset().getf());
	}
	else {
		std::cout << "ERROR::CLOUDS::update() Clouds should be rendered only using Skybox environment!" << std::endl;
	}

	// set 2D textures
	shader->setSampler("weatherMapTex", *weatherMapTex, 0);
	shader->setSampler("environmentTex", *getScene()->getEnvironmentTexture(), 3);

	// set 3D textures
	shader->setSampler("perlinWorleyTex", *perlinWorleyTex, 1);
	shader->setSampler("worleyTex", *worleyTex, 2);

	// set clouds shape info
	shader->setFloat("globalCloudsCoverage", data->globalCoverage);
	shader->setFloat("globalCloudsDensity", data->globalDensity);
	shader->setFloat("anvilAmount", data->anvilAmount);
	shader->setBool("isBaseShape", data->isBaseShape);

	// set clouds animation info
	shader->setFloat("time", static_cast<float>(glfwGetTime()));
	shader->setVec3("windDirection", data->windDirection);
	shader->setFloat("cloudSpeed", data->cloudSpeed);
	shader->setFloat("edgesSpeedMultiplier", data->edgesSpeedMultiplier);

	// set clouds lighting info
	shader->setVec3("cloudsColor", data->color.getf());
	shader->setFloat("beerCoeff", data->beerCoeff);
	shader->setBool("isPowder", data->enablePowder);
	shader->setFloat("powderCoeff", data->powderCoeff);
	shader->setFloat("csi", data->csi);

	FrameBufferObject::unbind();

	// disable depth test so screen-space quad isn't discarded due to depth test
	glDisable(GL_DEPTH_TEST);
	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_SRC_ALPHA);
	// draw the screen shader with the buffer texture
	cloudsShader->draw(*framebuffer->getColorTexture(0));
	// enable back depth test
	glEnable(GL_DEPTH_TEST);
	// disable back blending
	glDisable(GL_BLEND);
}

void Clouds::buildGUI()
{
	// Create the clouds control window
	ImGui::Begin("Clouds");

	// Create clouds shape header
	if (ImGui::CollapsingHeader("Shape", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Base shape switch
		bool isBaseShape = getBaseShape();
		imgui_exp::ToggleButton("Base shape", &isBaseShape);
		setBaseShape(isBaseShape);

		// Global coverage
		float globalCoverage = getGlobalCoverage();
		ImGui::SliderFloat("Global coverage", &globalCoverage, 0.0f, 1.0f);
		setGlobalCoverage(globalCoverage);

		// Global density
		float globalDensity = getGlobalDensity();
		ImGui::SliderFloat("Global density", &globalDensity, 0.0f, 1.0f);
		setGlobalDensity(globalDensity);

		// Anvil amount
		float anvilAmount = getAnvilAmount();
		ImGui::SliderFloat("Anvil amount", &anvilAmount, 0.0f, 1.0f);
		setAnvilAmount(anvilAmount);

		// Cloud type
		int cloudsType = static_cast<int>(getCloudsType());
		ImGui::Combo("Cloud type", &cloudsType, cloudTypes, IM_ARRAYSIZE(cloudTypes));
		setCloudsType(static_cast<CloudsType>(cloudsType));
	}

	// Create clouds animation header
	if (ImGui::CollapsingHeader("Animation"), ImGuiTreeNodeFlags_DefaultOpen) 
	{
		// Wind direction
		glm::vec3 windDirection = getWindDirection();
		float input[3] = { windDirection.x, windDirection.y, windDirection.z };
		ImGui::InputFloat3("Wind direction", input);
		windDirection.x = input[0];
		windDirection.y = input[1];
		windDirection.z = input[2];
		setWindDirection(windDirection);

		// Cloud speed
		float cloudSpeed = getCloudSpeed();
		ImGui::SliderFloat("Cloud speed", &cloudSpeed, 0.f, 1000.f);
		setCloudSpeed(cloudSpeed);

		// Edges speed multiplier
		float edgesSpeedMultiplier = getEdgesSpeedMultiplier();
		ImGui::SliderFloat("Edges speed multiplier", &edgesSpeedMultiplier, 0.f, 100.f);
		setEdgesSpeedMultiplier(edgesSpeedMultiplier);
	}

	// Create clouds lighting header
	if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Beer extinction coefficient
		float beerCoeff = getBeerCoeff();
		ImGui::SliderFloat("Beer coefficient", &beerCoeff, 0.0f, 5.0f);
		setBeerCoeff(beerCoeff);

		// Powder
		bool enablePowder = getPowder();
		imgui_exp::ToggleButton("Enable powder effect", &enablePowder);
		setPowder(enablePowder);

		// Powder coefficient
		float powderCoeff = getPowderCoeff();
		ImGui::SliderFloat("Powder coefficient", &powderCoeff, 1.0f, 10.0f);
		setPowderCoeff(powderCoeff);

		// Extra sun intensity
		float extraSunIntensity = getCSI();
		ImGui::SliderFloat("Extra sun intensity", &extraSunIntensity, 0.0f, 100.0f);
		setCSI(extraSunIntensity);

		// Color
		ImVec4 color = getColor().toIMGUI();
		ImGui::ColorEdit3("Color", (float*)&color);
		setColor(Color::fromIMGUI(color));
	}

	// Show cloud type changes
	cloudTypePopup();

	// Finish the window
	ImGui::End();
}


void Clouds::react(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	// Set current clouds to CUMULUS [1]
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		if (getCloudsType() != CloudsType::Cumulus)
			timeSinceLastKeyboardUpdate = static_cast<float>(glfwGetTime());
		setCloudsType(CloudsType::Cumulus);
	}

	// Set current clouds to STRATUS [2]
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		if (getCloudsType() != CloudsType::Stratus)
			timeSinceLastKeyboardUpdate = static_cast<float>(glfwGetTime());
		setCloudsType(CloudsType::Stratus);
	}

	// Set current clouds to STRATOCUMULUS [3]
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		if (getCloudsType() != CloudsType::Stratocumulus)
			timeSinceLastKeyboardUpdate = static_cast<float>(glfwGetTime());
		setCloudsType(CloudsType::Stratocumulus);
	}

	// Set current clouds to CUMULONIMBUS [4]
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		if (getCloudsType() != CloudsType::Cumulonimbus)
			timeSinceLastKeyboardUpdate = static_cast<float>(glfwGetTime());
		setCloudsType(CloudsType::Cumulonimbus);
	}

	// Set current clouds to MIX [5]
	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		if (getCloudsType() != CloudsType::Mix)
			timeSinceLastKeyboardUpdate = static_cast<float>(glfwGetTime());
		setCloudsType(CloudsType::Mix);
	}
}

void Clouds::buildHiddenGUI()
{
	// When GUI is hidden, show only cloud type changes
	cloudTypePopup();
}

void Clouds::generateNoiseTextures()
{
	// =============================================
	// 1st 3D texture (Perlin-Worley) (128^3) RGBA
	// =============================================

	// create shader
	Shader* perlinWorleyShader = new Shader();
	perlinWorleyShader->attachShader("Shaders/Noise/perlinWorley.comp", ShaderInfo(ShaderType::kCompute));
	perlinWorleyShader->linkProgram();

	// create texture
	perlinWorleyTex = new Texture(TextureType::threeDimensional, glm::vec3(128), 4, true);

	// configure shader
	perlinWorleyShader->use();
	glActiveTexture(GL_TEXTURE0);
	perlinWorleyShader->setInt("perlinWorleyTex", 0);
	glBindTexture(GL_TEXTURE_3D, perlinWorleyTex->ID);
	glBindImageTexture(0, perlinWorleyTex->ID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));

	// delete shader
	delete perlinWorleyShader;

	// =============================================
	// 2nd 3D texture (Worley) (32^3) RGB
	// =============================================

	// create shader
	Shader* worleyShader = new Shader();
	worleyShader->attachShader("Shaders/Noise/worley.comp", ShaderInfo(ShaderType::kCompute));
	worleyShader->linkProgram();

	// create texture
	worleyTex = new Texture(TextureType::threeDimensional, glm::vec3(32), 4, true);

	// configure shader
	worleyShader->use();
	glActiveTexture(GL_TEXTURE0);
	worleyShader->setInt("worleyTex", 0);
	glBindTexture(GL_TEXTURE_3D, worleyTex->ID);
	glBindImageTexture(0, worleyTex->ID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	glDispatchCompute(INT_CEIL(32, 4), INT_CEIL(32, 4), INT_CEIL(32, 4));

	// delete shader
	delete worleyShader;
}

void Clouds::generateWeatherMap()
{
	weatherMapShader->use();
	glActiveTexture(GL_TEXTURE0);
	weatherMapShader->setInt("weatherMapTex", 0);
	weatherMapShader->setInt("cloudsType", static_cast<int>(getCloudsType()));
	glBindTexture(GL_TEXTURE_2D, weatherMapTex->ID);
	glBindImageTexture(0, weatherMapTex->ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
	glDispatchCompute(INT_CEIL(1024, 8), INT_CEIL(1024, 8), 1);
}

void Clouds::cloudTypePopup()
{
	// Show notification upon cloud type change from the keyboard
	bool showPopup = static_cast<float>(glfwGetTime()) - timeSinceLastKeyboardUpdate < 0.5f;
	if (showPopup)
		ImGui::OpenPopup("Cloud");
	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Cloud", &showPopup, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(cloudTypes[static_cast<int>(getCloudsType())]);
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}
