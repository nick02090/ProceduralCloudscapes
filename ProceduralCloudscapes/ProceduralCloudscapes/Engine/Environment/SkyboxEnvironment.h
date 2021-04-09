#ifndef SKYBOX_ENVIRONMENT_H
#define SKYBOX_ENVIRONMENT_H

#include "Environment.h"
#include <glm/glm.hpp>

class ScreenShader;
class FrameBufferObject;

struct SkyboxEnvironmentData : EnvironmentData {
    // altitude of the sun from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
    float sunAltitude;
    // azimuth of the sun from range [-1.0, 1.0] where 0.0 is in front and -1.0 and 1.0 is behind (but coming from different side)
    float sunAzimuth;
    // sun intensity (default is 20.f) from range [10.0, 30.0]
    float sunIntensity;
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

    // GETTERS

    float getSunAltitude() const { return static_cast<SkyboxEnvironmentData*>(data)->sunAltitude; }
    float getSunAzimuth() const { return static_cast<SkyboxEnvironmentData*>(data)->sunAzimuth; }
    float getSunIntensity() const { return static_cast<SkyboxEnvironmentData*>(data)->sunIntensity; }

    // DRAWING

    ScreenShader* skyboxShader;
};

#endif // !SKYBOX_ENVIRONMENT_H
