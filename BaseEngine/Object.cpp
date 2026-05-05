#include "Object.h"
#include "Mesh.h"
#include "GLFW/Include/glfw3.h"
#include "Application.h"

Object::Object() : mesh(nullptr), name("Object")
{
}

void Object::Init()
{
    if (!mesh)
    {
        CreateMesh();
    }

    if (mesh)
    {
        mesh->Init();

        renderables.clear(); // Clear old references
        renderables.push_back(mesh);
    }
}

void Object::Update(float deltaTime)
{

}

void Object::CleanUp()
{

}

void Object::SetName(std::string newName)
{
	name = newName;
}

const std::vector<std::shared_ptr<Mesh>>& Object::GetRenderables()
{
	return renderables;
}

void Object::CreateMesh()
{
    mesh = std::make_shared<Mesh>();
}

void Object::SetMesh(std::shared_ptr<Mesh> aMesh)
{
    mesh = aMesh;

    renderables.clear();
    if (mesh)
    {
        renderables.push_back(mesh);
    }
}

std::shared_ptr<Mesh> Object::GetMesh()
{
	return mesh;
}

void Object::SetRotation(const glm::vec3& newRotation)
{
    localTransform.rotation = newRotation;
    transformDirtyFlag = true;
}

void Object::SetPosition(const glm::vec3& newPosition)
{
    localTransform.position = newPosition;
    transformDirtyFlag = true;
}

void Object::SetScale(const glm::vec3& newScale)
{
    localTransform.scale = newScale;
    transformDirtyFlag = true;
}

AABB Mesh::CalculateLocalAABB(const std::vector<Vertex>& vertices) {
    AABB box;
    box.minBounds = glm::vec3(FLT_MAX);
    box.maxBounds = glm::vec3(-FLT_MAX);

    for (const auto& v : vertices) {
        // Use the parentheses trick to block Windows macros from ruining std::min
        box.minBounds.x = (std::min)(box.minBounds.x, v.position.x);
        box.minBounds.y = (std::min)(box.minBounds.y, v.position.y);
        box.minBounds.z = (std::min)(box.minBounds.z, v.position.z);

        box.maxBounds.x = (std::max)(box.maxBounds.x, v.position.x);
        box.maxBounds.y = (std::max)(box.maxBounds.y, v.position.y);
        box.maxBounds.z = (std::max)(box.maxBounds.z, v.position.z);
    }
    return box;
}

Object::~Object()
{
}   