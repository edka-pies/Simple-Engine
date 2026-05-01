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

	std::shared_ptr<Mesh> GetMesh();
private:

	std::string name;
	std::shared_ptr<Mesh> mesh;
	std::vector<std::shared_ptr<Mesh>> renderables;
};
