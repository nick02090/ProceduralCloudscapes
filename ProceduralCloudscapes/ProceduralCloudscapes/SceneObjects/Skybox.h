#ifndef SKYBOX_H
#define SKYBOX_H

#include <glm/glm.hpp>

#include "../Engine/SceneObject.h"

class Shader;
class ScreenShader;
class FrameBufferObject;

class Skybox : public SceneObject {
public:
	Skybox(Window* _window);
	~Skybox();

	void update() override;

    // SETTERS
    void setSunDirection(glm::vec3 _sunDirection) { sunDirection = _sunDirection; }
    void setEarthRadius(float _earthRadius) { earthRadius = _earthRadius; }
    void setAtmosphereRadius(float _atmosphereRadius) { atmosphereRadius = _atmosphereRadius; }
    void setRayleighScaleHeight(float _scaleHeight) { Hr = _scaleHeight; }
    void setMieScaleHeight(float _scaleHeight) { Hm = _scaleHeight; }
    void setRayleighScatteringCoeff(glm::vec3 _scatteringCoeff) { betaR = _scatteringCoeff; }
    void setMieScatteringCoeff(glm::vec3 _scatteringCoeff) { betaM = _scatteringCoeff; }

    // GETTERS
    glm::vec3 getSunDirection() const { return sunDirection; }
    float getEarthRadius() const { return earthRadius; }
    float getAtmosphereRadius() const { return atmosphereRadius; }
    float getRayleighScaleHeight() const { return Hr; }
    float getMieScaleHeight() const { return Hm; }
    glm::vec3 getRayleighScatteringCoeff() const { return betaR; }
    glm::vec3 getMieScatteringCoeff() const { return betaM; }
private:
    // PROPERTIES

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


    // DRAWING

    ScreenShader* skyboxShader;

    FrameBufferObject* framebuffer;

    void configureData();
};

#endif // !SKYBOX_H
