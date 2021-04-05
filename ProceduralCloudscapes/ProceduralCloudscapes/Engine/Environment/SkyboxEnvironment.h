#ifndef SKYBOX_ENVIRONMENT_H
#define SKYBOX_ENVIRONMENT_H

#include "Environment.h"
#include <glm/glm.hpp>

class ScreenShader;
class FrameBufferObject;

struct SkyboxEnvironmentData : EnvironmentData {
    glm::vec3 sunDirection;

    float earthRadius;
    float atmosphereRadius;

    // thickness of the atmosphere if density was uniform (Rayleigh scale height)
    float Hr;
    // thickness of the atmosphere if density was uniform (Mie scale height)
    float Hm;

    // Rayleigh scattering coefficient
    glm::vec3 betaR;
    // Mie scattering coefficient
    glm::vec3 betaM;
};

class SkyboxEnvironment : public Environment {
public:
	SkyboxEnvironment(Window* _window);
	~SkyboxEnvironment();

	void update() override;

    // SETTERS

    void setSunDirection(glm::vec3 _sunDirection) { static_cast<SkyboxEnvironmentData*>(data)->sunDirection = _sunDirection; }
    void setEarthRadius(float _earthRadius) { static_cast<SkyboxEnvironmentData*>(data)->earthRadius = _earthRadius; }
    void setAtmosphereRadius(float _atmosphereRadius) { static_cast<SkyboxEnvironmentData*>(data)->atmosphereRadius = _atmosphereRadius; }
    void setRayleighScaleHeight(float _scaleHeight) { static_cast<SkyboxEnvironmentData*>(data)->Hr = _scaleHeight; }
    void setMieScaleHeight(float _scaleHeight) { static_cast<SkyboxEnvironmentData*>(data)->Hm = _scaleHeight; }
    void setRayleighScatteringCoeff(glm::vec3 _scatteringCoeff) { static_cast<SkyboxEnvironmentData*>(data)->betaR = _scatteringCoeff; }
    void setMieScatteringCoeff(glm::vec3 _scatteringCoeff) { static_cast<SkyboxEnvironmentData*>(data)->betaM = _scatteringCoeff; }

    // GETTERS

    glm::vec3 getSunDirection() const { return static_cast<SkyboxEnvironmentData*>(data)->sunDirection; }
    float getEarthRadius() const { return static_cast<SkyboxEnvironmentData*>(data)->earthRadius; }
    float getAtmosphereRadius() const { return static_cast<SkyboxEnvironmentData*>(data)->atmosphereRadius; }
    float getRayleighScaleHeight() const { return static_cast<SkyboxEnvironmentData*>(data)->Hr; }
    float getMieScaleHeight() const { return static_cast<SkyboxEnvironmentData*>(data)->Hm; }
    glm::vec3 getRayleighScatteringCoeff() const { return static_cast<SkyboxEnvironmentData*>(data)->betaR; }
    glm::vec3 getMieScatteringCoeff() const { return static_cast<SkyboxEnvironmentData*>(data)->betaM; }


    // DRAWING

    ScreenShader* skyboxShader;
};

#endif // !SKYBOX_ENVIRONMENT_H
