#pragma once
#include <vector>
#include "glm/glm/glm.hpp"
#include <string>

class Mesh;
class Object;
class Texture;
class Scene;

class AssetViewer
{

	struct ObjectData
	{
		glm::vec3 position;
		glm::vec3 eulerRotation;
		glm::vec3 scale;
		std::string objetName;
		bool isInitialized = false;
	};

public:
    void SetActiveScene(Scene* scene);
    // link the master list
    void SetMasterObjectList(std::vector<Object*>* masterList);

    AssetViewer();
    void Init();
    void Draw();
    void AssignSelectedObject(Object* otherMesh);

private:
    Scene* activeScene;
    Object* selectedObject;
    ObjectData* currentData;

    std::vector<Object*>* masterObjectList;
    std::vector<ObjectData> objectDataList;
    char texturePath[260];
};
