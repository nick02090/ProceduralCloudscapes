#include "Clouds.h"

#include "../Engine/ScreenShader.h"
#include "../Engine/Scene.h"
#include "PlaneTexture.h"
#include "../Engine/Environment/SkyboxEnvironment.h"
#include "../Engine/Environment/ColorEnvironment.h"
#include "../Engine/FrameBufferObject.h"
#include "../Engine/GUI/ImGUIExpansions.h"
#include "../Engine/Utilities.h"

static const char* cloudTypes[] = { "Cumulus", "Stratocumulus", "Stratus", "Cumulonimbus" };

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
	//generateWeatherMap();

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

	// set clouds lighting info
	shader->setVec3("cloudsColor", data->color.getf());
	shader->setFloat("beerCoeff", data->beerCoeff);
	shader->setBool("isPowder", data->enablePowder);
	shader->setFloat("powderCoeff", data->powderCoeff);
	shader->setFloat("csi", data->csi);

	FrameBufferObject::unbind();

	// create a screen shader for rendering the buffer on the screen
	ScreenShader* screenShader = new ScreenShader("Shaders/Default/textureShader2D.frag");
	// disable depth test so screen-space quad isn't discarded due to depth test
	glDisable(GL_DEPTH_TEST);
	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, GL_SRC_ALPHA);
	// draw the screen shader with the buffer texture
	screenShader->draw(*framebuffer->getColorTexture(0));
	// delete the created screen shader
	delete screenShader;
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

		// Anvil amount
		float anvilAmount = getAnvilAmount();
		ImGui::SliderFloat("Anvil amount", &anvilAmount, 0.0f, 1.0f);
		setAnvilAmount(anvilAmount);

		// Global density
		float globalDensity = getGlobalDensity();
		ImGui::SliderFloat("Global density", &globalDensity, 0.0f, 1.0f);
		setGlobalDensity(globalDensity);

		// Cloud type
		int cloudsType = static_cast<int>(getCloudsType());
		ImGui::Combo("Cloud type", &cloudsType, cloudTypes, IM_ARRAYSIZE(cloudTypes));
		// update the weather map if clouds type has changed
		if (static_cast<CloudsType>(cloudsType) != getCloudsType()) {
			setCloudsType(static_cast<CloudsType>(cloudsType));
			if (getCloudsType() == CloudsType::Cumulonimbus)
				setAnvilAmount(util::random());
			generateWeatherMap();
		}
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

	// Finish the window
	ImGui::End();
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

	// =============================================
	// 1st 2D texture (Curl) (128^2) RGB
	// =============================================
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
