#pragma once
#include "Transform.h"
#include "Renderable.h"
#include <glm/glm/glm.hpp>

class Camera
{
public:
    Camera();

    void Init();

    void SetPosition(const glm::vec3& aPos);
    void LookAt(const glm::vec3& aTarget, const glm::vec3& aUp);

    // Return references
    inline const glm::mat4& GetView() const { return viewMatrix; }
    inline const glm::mat4& GetProjection() const { return projectionMatrix; }
    inline const glm::vec3& GetPosition() const { return localTransform.position; }

    void SetFov(float aFov);
    void SetAspectRatio(float aAspect);
    void SetAspectRation(float aWidth, float aHeight);
    void SetNearFar(float aNearPlane, float aFarPlane);

    // Getters for UI
    float GetFov() const;
    float GetNearPlane() const;
    float GetFarPlane() const;

    inline const glm::mat4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }

    void Update();

    Transform localTransform;

    // Movement API (FPS-style)
    enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
    void ProcessKeyboard(Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

private:
    void UpdateCameraVectors();

    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    // FPS-style camera vectors & angles
    glm::vec3 front;
    glm::vec3 right;
    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;

    float fov;
    float aspectRation;
    float nearPlane;
    float farPlane;

    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);

    bool transformDirtyFlag = true;
};

