#ifndef CLOUDS_H
#define CLOUDS_H

#define INT_CEIL(n,d) (int)ceil((float)n/d)

#include "../Engine/SceneObject.h"
#include "../Engine/Texture.h"
#include "../Engine/Shader.h"
#include "../Engine/Color.h"
#include "../Engine/Utilities.h"
#include "../Engine/Window.h"

class ScreenShader;
class FrameBufferObject;

enum class CloudsType {
	Cumulus = 0,
	Stratus = 1,
	Stratocumulus = 2,
	Cumulonimbus = 3,
	Mix = 4
};

struct CloudsData {
	// =============================================
	// CLOUDS SHAPE
	// =============================================
	
	// global coverage of clouds from range [0.0, 1.0] where 0.0 is clear sky and 1.0 is cloudy weather
	float globalCoverage;
	// global density of clouds from range [0.0, inf] that determines the clouds global opacity
	float globalDensity;
	// flag to show clouds base shape
	bool isBaseShape;
	// amount of the anvil that will clouds have
	float anvilAmount;
	// type of the clouds
	CloudsType cloudsType;

	// =============================================
	// CLOUDS ANIMATION
	// =============================================

	// direction in which the clouds will be animated
	glm::vec3 windDirection;
	// animation speed (clouds movement speed)
	float cloudSpeed;
	// animation speed multiplier for the edge parts of the clouds
	float edgesSpeedMultiplier;

	// =============================================
	// CLOUDS LIGHTING
	// =============================================

	// extinction coefficient (Beer-Lambert law)
	float beerCoeff;
	// flag for powder
	bool enablePowder;
	// powder coefficient
	float powderCoeff;
	// extra sun intensity (increases HG effect)
	float csi;
	// clouds base color
	Color color;
};

class Clouds : public SceneObject, public GUIBuilder, public KeyReactor {
public:
	Clouds(Window* _window);
	~Clouds();

	void update() override;
	void buildGUI() override;
	void buildHiddenGUI() override;
	void react(GLFWwindow* window, int key, int scancode, int action, int mods) override;

	// SETTERS

	inline void setGlobalCoverage(float _globalCoverage) { data->globalCoverage = _globalCoverage; }
	inline void setGlobalDensity(float _globalDensity) { data->globalDensity = _globalDensity; }
	inline void setAnvilAmount(float _anvilAmount) { data->anvilAmount = _anvilAmount; }
	void setCloudsType(CloudsType _cloudsType) 
	{
		bool hasChanged = false;

		// check if clouds type has changed
		if (data->cloudsType != _cloudsType) {

			hasChanged = true;

			// change global coverage based on cloud type
			if (_cloudsType == CloudsType::Cumulus)
				setGlobalCoverage(util::random(0.1f, 0.3f));
			else if (_cloudsType == CloudsType::Stratus)
				setGlobalCoverage(util::random(0.7f, 1.0f));
			else if (_cloudsType == CloudsType::Stratocumulus)
				setGlobalCoverage(util::random(0.6f, 0.8f));
			else if (_cloudsType == CloudsType::Cumulonimbus)
				setGlobalCoverage(0.5f);
			else
				setGlobalCoverage(0.3f);

			// change global density based on cloud type
			if (_cloudsType == CloudsType::Stratus)
				setGlobalDensity(util::random(0.1f, 0.3f));
			else if (_cloudsType == CloudsType::Stratocumulus)
				setGlobalDensity(util::random(0.2f, 0.5f));
			else
				setGlobalDensity(0.5f);

			// change anvil amount based on cloud type
			if (_cloudsType == CloudsType::Cumulonimbus)
				setAnvilAmount(util::random());
			else if (_cloudsType == CloudsType::Mix)
				setAnvilAmount(util::random(0.f, 0.5f));
			else
				setAnvilAmount(0.f);
		}

		// update the clouds type
		data->cloudsType = _cloudsType;

		// NOTE: Weather map has to be generated once the data->cloudsType has been updated!!!
		if (hasChanged)
			// generate weather map when cloud type changes
			generateWeatherMap();
	}
	inline void setBaseShape(bool _isBaseShape) { data->isBaseShape = _isBaseShape; }

	inline void setWindDirection(glm::vec3 _windDirection) { data->windDirection = _windDirection; }
	inline void setCloudSpeed(float _cloudSpeed) { data->cloudSpeed = _cloudSpeed; }
	inline void setEdgesSpeedMultiplier(float _edgesSpeedMultiplier) { data->edgesSpeedMultiplier = _edgesSpeedMultiplier; }

	inline void setBeerCoeff(float _beerCoeff) { data->beerCoeff = _beerCoeff; }
	inline void setPowder(bool _isPowder) { data->enablePowder = _isPowder; }
	inline void setPowderCoeff(float _powderCoeff) { data->powderCoeff = _powderCoeff; }
	inline void setCSI(float _csi) { data->csi = _csi; }

	inline void setColor(Color _color) { data->color = _color; }
	inline void setColor(int r, int g, int b) { data->color.set(r, g, b); }
	inline void setColor(float r, float g, float b) { data->color.setf(r, g, b); }

	// GETTERS

	inline float getGlobalCoverage() const { return data->globalCoverage; }
	inline float getGlobalDensity() const { return data->globalDensity; }
	inline float getAnvilAmount() const { return data->anvilAmount; }
	inline CloudsType getCloudsType() const { return data->cloudsType; }
	inline bool getBaseShape() const { return data->isBaseShape; }

	inline glm::vec3 getWindDirection() const { return data->windDirection; }
	inline float getCloudSpeed() const { return data->cloudSpeed; }
	inline float getEdgesSpeedMultiplier() const { return data->edgesSpeedMultiplier; }

	inline float getBeerCoeff() const { return data->beerCoeff; }
	inline float getPowder() const { return data->enablePowder; }
	inline float getPowderCoeff() const { return data->powderCoeff; }
	inline float getCSI() const { return data->csi; }
	inline Color getColor() const { return data->color; }

private:
	void generateNoiseTextures();
	void generateWeatherMap();
	void cloudTypePopup();

	float timeSinceLastKeyboardUpdate = 0.f;

	Texture* perlinWorleyTex = nullptr;
	Texture* worleyTex = nullptr;
	Texture* curlTex = nullptr;

	Texture* weatherMapTex = nullptr;
	Shader* weatherMapShader = nullptr;

	ScreenShader* cloudsShader = nullptr;

	CloudsData* data = nullptr;

	FrameBufferObject* framebuffer = nullptr;
};

#endif // !CLOUDS_H
