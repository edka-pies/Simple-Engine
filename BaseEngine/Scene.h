#pragma once
#include <memory>
#include <vector>
#include <map>
#include "Camera.h"
#include "Object.h"
#include "Player.h"
#include "Enemy.h"
#include "Terrain.h"
#include "Renderable.h"
#include "RagdollEntity.h"
#include "Light.h"
#include "Platform.h"

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
	std::vector<RagdollEntity*> ragdolls;

	Camera* mainCamera;

	Terrain* activeTerrain = nullptr;

	bool isPlaying = false;
	Player player;
	std::vector<Enemy*> enemies;
	std::vector<Platform*> platforms;

	float levelTime = 0.0f;
	float bestTime = 9999.0f;

private:
	void AddRenderable(std::shared_ptr<Mesh> renderable);

};

