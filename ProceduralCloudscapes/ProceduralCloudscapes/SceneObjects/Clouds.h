#ifndef CLOUDS_H
#define CLOUDS_H

#define INT_CEIL(n,d) (int)ceil((float)n/d)

#include "../Engine/SceneObject.h"
#include "../Engine/Texture.h"
#include "../Engine/Shader.h"
#include "../Engine/Color.h"

class PlaneTexture;
class ScreenShader;
class FrameBufferObject;

struct CloudsData {
	// global coverage of clouds from range [0.0, 1.0] where 0.0 is clear sky and 1.0 is cloudy weather
	float globalCoverage;
	// global density of clouds from range [0.0, inf] that determines the clouds global opacity
	float globalDensity;
	// clouds base color
	Color color;
};

class Clouds : public SceneObject, public GUIBuilder {
public:
	Clouds(Window* _window);
	~Clouds();

	void update() override;
	void buildGUI() override;

	// SETTERS

	inline void setGlobalCoverage(float _globalCoverage) { data->globalCoverage = _globalCoverage; }
	inline void setGlobalDensity(float _globalDensity) { data->globalDensity = _globalDensity; }
	inline void setColor(Color _color) { data->color = _color; }
	inline void setColor(int r, int g, int b) { data->color.set(r, g, b); }
	inline void setColor(float r, float g, float b) { data->color.setf(r, g, b); }

	// GETTERS

	inline float getGlobalCoverage() const { return data->globalCoverage; }
	inline float getGlobalDensity() const { return data->globalDensity; }

private:
	void generateNoiseTextures();
	void generateWeatherMap();

	Texture* perlinWorleyTex = nullptr;
	Texture* worleyTex = nullptr;
	Texture* curlTex = nullptr;

	Texture* weatherMapTex = nullptr;
	Shader* weatherMapShader = nullptr;

	// TODO: Remove upon finished debugging
	PlaneTexture* perlinWorleyPlaneTexture = nullptr;
	PlaneTexture* worleyPlaneTexture = nullptr;
	PlaneTexture* weatherMapPlaneTexture = nullptr;

	ScreenShader* cloudsShader = nullptr;

	CloudsData* data = nullptr;

	FrameBufferObject* framebuffer = nullptr;
};

#endif // !CLOUDS_H
