#pragma once
#include <memory>
#include <vector>
#include <map>
#include "Camera.h"
#include "Object.h"
#include "Terrain.h"
#include "Renderable.h"
#include "Light.h"

class Scene
{
public:

	Scene();

	void Init();

	void Update(float deltaTime);

	void AddObjects(Object* aObject);

	void RemoveObject(Object* aObject);	

	std::vector<Object*> objects;
	std::vector<std::shared_ptr<Mesh>> renderables;
	std::vector<Light*> lights;

	Camera* mainCamera;

	Terrain* activeTerrain = nullptr;

private:
	void AddRenderable(std::shared_ptr<Mesh> renderable);

	//std::map<Object*, std::vector<std::shared_ptr<Mesh>>> objectMap;

};

