#pragma once
#include <memory>
#include <vector>
#include <map>
#include "Camera.h"
#include "Object.h"
#include "Enemy.h"
#include "Terrain.h"
#include "Renderable.h"
#include "Mesh.h"
#include "Light.h"
#include "ActiveRagdollComponent.h"

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

	bool isPlaying = false;
	Object* playerObject = nullptr;

	float levelTime = 0.0f;
	float bestTime = 9999.0f;

	float timeScale = 1.0f;

private: 
	void AddRenderable(std::shared_ptr<Mesh> renderable);

};

