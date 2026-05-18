#pragma once
#include <string>
#include <vector>
#include "glm/glm/glm.hpp"

class Scene;
class Object;

class PrefabManager
{
public:
	PrefabManager();
	~PrefabManager();

	Object* InstantiateRagdollPrefab(
		const std::string& filepath
		,glm::vec3 spawnWorldPos
		,Scene* scene
		,std::vector<Object*>& masterList
		, const std::string& instanceName);
};

