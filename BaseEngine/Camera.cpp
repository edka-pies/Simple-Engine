#include "Camera.h"
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/constants.hpp>
#include <glm/glm/gtx/transform.hpp>

Camera::Camera()
	: position(glm::vec3(30.0f, 0.0f, 0.0f)),
	target(glm::vec3(0.0f, 0.0f, 0.0f)),
	up(glm::vec3(0.0f, 1.0f, 0.0f)),
	front(glm::normalize(target - position)),
	right(glm::normalize(glm::cross(front, up))),
	yaw(-90.0f), // will be recomputed in Init()
	pitch(0.0f),
	movementSpeed(10.0f),
	mouseSensitivity(0.1f),
	fov(45.0f),
	aspectRation((1280.0f / 720.0f)),
	nearPlane(0.1f),
	farPlane(100.0f),
	viewMatrix(glm::mat4(1.0f)),
	projectionMatrix(glm::mat4(1.0f)),
	viewProjectionMatrix(glm::mat4(1.0f)),
	transformDirtyFlag(true)
{
}

void Camera::Init()
{
	localTransform.position = position;
	// initialize yaw/pitch from the current front vector
	glm::vec3 f = glm::normalize(front);
	yaw = glm::degrees(atan2(f.z, f.x));
	pitch = glm::degrees(asin(glm::clamp(f.y, -1.0f, 1.0f)));
	UpdateCameraVectors();
}

void Camera::SetPosition(const glm::vec3& aPos)
{
	localTransform.position = aPos;
	transformDirtyFlag = true;
}

void Camera::LookAt(const glm::vec3& aTarget, const glm::vec3& aUp)
{
	target = aTarget;
	up = aUp;
	transformDirtyFlag = true;
}

void Camera::SetFov(float aFov)
{
	fov = aFov;
	transformDirtyFlag = true;
}

void Camera::SetAspectRatio(float aAspect)
{
	aspectRation = aAspect;
	transformDirtyFlag = true;
}

void Camera::SetAspectRation(float aWidth, float aHeight)
{
	SetAspectRatio(aWidth / aHeight);
}

void Camera::SetNearFar(float aNearPlane, float aFarPlane)
{
	nearPlane = aNearPlane;
	farPlane = aFarPlane;
	transformDirtyFlag = true;
}

// Getter implementations added to resolve linker errors
float Camera::GetFov() const
{
	return fov;
}

float Camera::GetNearPlane() const
{
	return nearPlane;
}

float Camera::GetFarPlane() const
{
	return farPlane;
}

void Camera::Update()
{
	if (transformDirtyFlag)
	{
		projectionMatrix = glm::perspective(glm::radians(fov), aspectRation, nearPlane, farPlane);
		// use the front vector for an FPS-style camera
		viewMatrix = glm::lookAt(localTransform.position, localTransform.position + front, up);
		viewProjectionMatrix = projectionMatrix * viewMatrix;
		transformDirtyFlag = false;
	}
}

void Camera::ProcessKeyboard(Movement direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	if (direction == FORWARD)
		localTransform.position += front * velocity;
	if (direction == BACKWARD)
		localTransform.position -= front * velocity;
	if (direction == LEFT)
		localTransform.position -= right * velocity;
	if (direction == RIGHT)
		localTransform.position += right * velocity;
	if (direction == UP)
		localTransform.position += up * velocity;
	if (direction == DOWN)
		localTransform.position -= up * velocity;

	transformDirtyFlag = true;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (constrainPitch)
	{
		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;
	}

	UpdateCameraVectors();
	transformDirtyFlag = true;
}

void Camera::UpdateCameraVectors()
{
	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(f);
	right = glm::normalize(glm::cross(front, up));
}

