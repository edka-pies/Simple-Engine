#include "Scene.h"
#include "Object.h"
#include "Renderable.h"

Scene::Scene() : mainCamera(nullptr)
{

}

void Scene::Init()
{
	mainCamera = new Camera();
	mainCamera->Init();
}

void Scene::Update(float deltaTime)
{
	for (Object* objectsInScene : objects)
	{
		objectsInScene->Update(deltaTime);
	}
}

void Scene::AddObjects(Object* aObject)
{
    if (!aObject) return;
    auto it = std::find(objects.begin(), objects.end(), aObject);
    if (it == objects.end())
    {
        objects.push_back(aObject);
        aObject->Init();
    }
}

void Scene::RemoveObject(Object* aObject)
{
    if (!aObject) return;
    auto it = std::find(objects.begin(), objects.end(), aObject);
    if (it != objects.end())
    {
        objects.erase(it);
    }
}

void Scene::AddRenderable(std::shared_ptr<Mesh> renderable)
{
    if (!renderable) return;
    auto it = std::find(renderables.begin(), renderables.end(), renderable);
    if (it == renderables.end())
    {
        renderables.push_back(renderable);
    }
}
