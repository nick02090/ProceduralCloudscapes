#include "Camera.h"

Camera::Camera(glm::vec3 _position, glm::vec3 _up, float _yaw, float _pitch)
{
	initialPosition = _position;
	initialiUp = _up;
	initialYaw = _yaw;
	initialPitch = _pitch;

	position = _position;
	worldUp = _up;
	yaw = _yaw;
	pitch = _pitch;

	front = glm::vec3(0.0f, 0.0f, -1.0f);
	movementSpeed = SPEED;
	mouseSensitivity = SENSITIVITY;
	zoom = ZOOM;

	updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	switch (direction)
	{
	case CameraMovement::Forward:
		position += front * velocity;
		break;
	case CameraMovement::Backward:
		position -= front * velocity;
		break;
	case CameraMovement::Left:
		position -= right * velocity;
		break;
	case CameraMovement::Right:
		position += right * velocity;
		break;
	case CameraMovement::Up:
		position += up * velocity;
		break;
	case CameraMovement::Down:
		position -= up * velocity;
		break;
	default:
		break;
	}

	// camera can't go below 0!
	if (position.y < 10.0) position.y = 10.0;
}

void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (constrainPitch)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset)
{
	zoom -= yoffset;
	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;
}

void Camera::reset()
{
	position = initialPosition;
	up = initialiUp;
	yaw = initialYaw;
	pitch = initialPitch;

	updateCameraVectors();
}

void Camera::updateCameraVectors()
{
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}
