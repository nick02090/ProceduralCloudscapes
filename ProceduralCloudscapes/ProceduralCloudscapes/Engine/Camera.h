#ifndef CAMERA_H
#define CAMERA_H

// include glad to get all the required OpenGL headers
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum class CameraMovement
{
	Forward,
	Backward,
	Left,
	Right,
	Up,
	Down
};

// default camera settings
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 60.0f;

class Camera
{
public:
	Camera(glm::vec3 _position = glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3 _up = glm::vec3(0.0f, 1.0f, 0.0f), float _yaw = YAW, float _pitch = PITCH);

	void processKeyboard(CameraMovement direction, float deltaTime);

	glm::mat4 getViewMatrix() const;
	inline float getZoom() const { return zoom; }
	inline float getMovementSpeed() const { return movementSpeed; }
	inline float getMouseSensitivity() const { return mouseSensitivity; }
	inline glm::vec3 getPosition() const { return position; }
	inline glm::vec3 getDirection() const { return glm::normalize(front); }
	inline glm::vec3 getUp() const { return glm::normalize(up); }
	void setMovementSpeed(float value) { movementSpeed = value; };
	void setMouseSensitivity(float value) { mouseSensitivity = value; };

	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void processMouseScroll(float yoffset);

	void reset();
private:
	void updateCameraVectors();

	glm::vec3 initialPosition;
	glm::vec3 initialiUp;
	float initialYaw;
	float initialPitch;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	float yaw;
	float pitch;
	float movementSpeed;
	float mouseSensitivity;
	float zoom;
};

#endif