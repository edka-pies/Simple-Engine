#include "PrefabManager.h"
#include "Object.h"
#include "Scene.h"
#include "ActiveRagdollComponent.h"
#include "MeshComponent.h"
#include "Primitives.h"
#include <fstream>
#include "CollisionComponent.h"
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

PrefabManager::PrefabManager()
{
}

PrefabManager::~PrefabManager()
{
}

Object* PrefabManager::InstantiateRagdollPrefab(
    const std::string& filepath
    ,glm::vec3 spawnWorldPos
    ,Scene* scene
    ,std::vector<Object*>& masterList
    , const std::string& instanceName)
{ 
    std::ifstream file(filepath);
    if (!file.is_open()) return nullptr;

    json jObj;
    file >> jObj;

    // Create the "Brain" Object (This has no mesh, it's just a logic container!)
    Object* brain = new Object();
    brain->GetTransform().position = spawnWorldPos;
    brain->SetName(instanceName + "_Brain");

    // Use your engine's Add method
    scene->AddObjects(brain);
    masterList.push_back(brain);

    // Get a mesh for the parts so they aren't invisible!
    std::shared_ptr<Mesh> cubeMesh = Primitives::CreateCube();

    std::vector<Object*> parts;
    for (const auto& partData : jObj["parts"]) {
        Object* part = new Object();
        part->SetName(instanceName + "_" + std::string(partData["id"]));

        // GIVE IT A BODY AND SCALE
        part->serialize = false;
        part->AddComponent(std::make_shared<MeshComponent>(cubeMesh));
		part->AddComponent(std::make_shared<CollisionComponent>(cubeMesh->localAABB, false)); // Dynamic collider
        if (partData.contains("scale")) {
            part->SetScale(glm::vec3(partData["scale"][0], partData["scale"][1], partData["scale"][2]));
        }

        // POSITION IT
        glm::vec3 offset(partData["offset"][0], partData["offset"][1], partData["offset"][2]);
        part->SetPosition(spawnWorldPos + offset);

        scene->AddObjects(part);
        masterList.push_back(part);
        parts.push_back(part);
    }

    auto ragdollComp = std::make_shared<ActiveRagdollComponent>();
    brain->AddComponent(ragdollComp);
    ragdollComp->BuildRagdoll(parts);

    return brain;
}