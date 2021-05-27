#ifndef SKYBOX_ENVIRONMENT_H
#define SKYBOX_ENVIRONMENT_H

#include "Environment.h"
#include "../../Engine/Color.h"
#include <glm/glm.hpp>

class ScreenShader;
class FrameBufferObject;

struct SkyboxEnvironmentData : EnvironmentData {

    // =============================================
    // SUN POSITION
    // =============================================

    // altitude of the sun from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
    float sunAltitude;
    // azimuth of the sun from range [-1.0, 1.0] where 0.0 is in front and -1.0 and 1.0 is behind (but coming from different side)
    float sunAzimuth;

    // =============================================
    // COLOR
    // =============================================

    // sun intensity (default is 20.f) from range [10.0, 30.0]
    float sunIntensity;
    // center part sun scale
    float sunScale;
    // light that sun emits at daytime
    Color sunColorDay;
    // light that sun emits at sunset
    Color sunColorSunset;

    // =============================================
    // POST PROCESSING
    // =============================================

    bool isGammaAndContrast;
    bool isVignette;
};

class SkyboxEnvironment : public Environment {
public:
	SkyboxEnvironment(Window* _window);
	~SkyboxEnvironment();

	void update() override;
    void extendGUI() override;

    // SETTERS

    void setSunAltitude(float _sunAltitude) { static_cast<SkyboxEnvironmentData*>(data)->sunAltitude = _sunAltitude; }
    void setSunAzimuth(float _sunAzimuth) { static_cast<SkyboxEnvironmentData*>(data)->sunAzimuth = _sunAzimuth; }
    void setSunIntensity(float _sunIntensity) { static_cast<SkyboxEnvironmentData*>(data)->sunIntensity = _sunIntensity; }
    void setSunScale(float _sunScale) { static_cast<SkyboxEnvironmentData*>(data)->sunScale = _sunScale; }
    void setGammaAndContrast(bool _isGammaAndContrast) { static_cast<SkyboxEnvironmentData*>(data)->isGammaAndContrast = _isGammaAndContrast; }
    void setVignette(bool _isVignette) { static_cast<SkyboxEnvironmentData*>(data)->isVignette = _isVignette; }
    void setSunColorDay(Color _sunColorDay) { static_cast<SkyboxEnvironmentData*>(data)->sunColorDay = _sunColorDay; }
    void setSunColorSunset(Color _sunColorSunset) { static_cast<SkyboxEnvironmentData*>(data)->sunColorSunset = _sunColorSunset; }

    // GETTERS

    inline float getSunAltitude() const { return static_cast<SkyboxEnvironmentData*>(data)->sunAltitude; }
    inline float getSunAzimuth() const { return static_cast<SkyboxEnvironmentData*>(data)->sunAzimuth; }
    inline float getSunIntensity() const { return static_cast<SkyboxEnvironmentData*>(data)->sunIntensity; }
    inline float getSunScale() const { return static_cast<SkyboxEnvironmentData*>(data)->sunScale; }
    inline bool getIsGammaAndContrast() const { return static_cast<SkyboxEnvironmentData*>(data)->isGammaAndContrast; }
    inline bool getIsVignette() const { return static_cast<SkyboxEnvironmentData*>(data)->isVignette; }
    inline Color getSunColorDay() const { return static_cast<SkyboxEnvironmentData*>(data)->sunColorDay; }
    inline Color getSunColorSunset() const { return static_cast<SkyboxEnvironmentData*>(data)->sunColorSunset; }

    // DRAWING

    ScreenShader* skyboxShader;
};

#endif // !SKYBOX_ENVIRONMENT_H
