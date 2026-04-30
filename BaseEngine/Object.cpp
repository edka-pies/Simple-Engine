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
        renderables.push_back(mesh); // This now works because types match!
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
    // This assigns a new mesh to the member variable 'mesh'
    mesh = std::make_shared<Mesh>();
}

void Object::SetMesh(std::shared_ptr<Mesh> aMesh)
{
    mesh = aMesh;

    // Critical for runtime swapping:
    // When you change the mesh, you must update the render list 
    // so the renderer knows to draw the NEW mesh, not the old one.
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

Object::~Object()
{
}
