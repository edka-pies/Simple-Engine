#pragma once
#include <glm/glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include "Transform.h"
#include "Renderable.h"

class Mesh;

class Object
{

public:
	Object();
	~Object();

	void Init();

	void Update(float deltaTime);

	void CleanUp();

	inline std::string GetName() { return name; }

	void SetName(std::string newName);

	const std::vector<std::shared_ptr<Mesh>>& GetRenderables();

	void CreateMesh();

	void SetMesh(std::shared_ptr<Mesh> aMesh);

	void SetRotation(const glm::vec3& newRotation);

	void SetPosition(const glm::vec3& newPosition);

	void SetScale(const glm::vec3& newScale);

	void SetMeshFilePath(const std::string& path) { meshFilePath = path; }
	std::string GetMeshFilePath() const { return meshFilePath; }

	glm::vec3 GetPosition() const { return localTransform.position; }
	glm::vec3 GetRotation() const { return localTransform.rotation; }
	glm::vec3 GetScale() const { return localTransform.scale; }

	const glm::mat4& GetModelMatrix() const {
		if (transformDirtyFlag) {
			cachedModelMatrix = localTransform.mat4();
			transformDirtyFlag = false;
		}
		return cachedModelMatrix;
	}

	inline Transform& GetTransform() { return localTransform; }

	glm::vec3 velocity = glm::vec3(0.0f);

	std::shared_ptr<Mesh> GetMesh();
private:

	Transform localTransform;
	mutable glm::mat4 cachedModelMatrix = glm::mat4(1.0f);
	mutable bool transformDirtyFlag = true;
	std::string name;
	std::shared_ptr<Mesh> mesh;
	std::vector<std::shared_ptr<Mesh>> renderables;
	std::string meshFilePath = "";
};
