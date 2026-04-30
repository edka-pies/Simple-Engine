#include "ForwardRendererPass.h"
#include "Scene.h"
#include "Renderable.h"
#include "Mesh.h"
#include "MeshManager.h"
#include <memory>
#include <glm/glm/glm.hpp>
#include <GLFW/Include/glfw3.h>

ForwardRendererPass::ForwardRendererPass() : myShader(nullptr)
{
    myShader = new Shader("Assets/Shaders/VertexShader.glsl", "Assets/Shaders/FragmentShader.glsl");
}

void ForwardRendererPass::Init()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void ForwardRendererPass::Execute(Scene& aScene)
{
    if (!myShader) return;

    myShader->Use();

    if (aScene.mainCamera) 
    {
        aScene.mainCamera->Update();

        const glm::mat4& view = aScene.mainCamera->GetView();
        const glm::mat4& proj = aScene.mainCamera->GetProjection();
        myShader->SetMatrix(view, "viewMatrix");
        myShader->SetMatrix(proj, "projectionMatrix");

        glm::vec3 camPos = aScene.mainCamera->GetPosition();
        myShader->SetVec4(glm::vec4(camPos, 1.0f), "eyePosition");
    } 
    else 
    {
        myShader->SetMatrix(glm::mat4(1.0f), "viewMatrix");
        myShader->SetMatrix(glm::mat4(1.0f), "projectionMatrix");
        myShader->SetVec4(glm::vec4(0.0f), "eyePosition");
    }

    if (!aScene.lights.empty()) {
        myShader->SetLights(aScene.lights);
    }

    // Provide default material values
    myShader->SetVec4(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), "materialAmbient");
    myShader->SetVec4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "materialDiffuse");
    myShader->SetVec4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "materialSpecular");
    myShader->SetInt(32, "materialShininess"); // reasonable default gloss

	// Check for texture usage
    myShader->SetInt(0, "useTexture");
    myShader->SetFloat(1.0f, "textureMixFactor");
    myShader->SetInt(0, "diffuseTexture");

    for (auto& mesh : aScene.renderables)
    {
        if (mesh) mesh->Render(*myShader, aScene.mainCamera ? aScene.mainCamera->GetViewProjectionMatrix() : glm::mat4(1.0f));
    }

    for (Object* obj : aScene.objects) // Loop through the objects directly
    {
        if (obj)
        {
            // Get the current list of meshes from the object (dynamically!)
            for (auto& mesh : obj->GetRenderables())
            {
                if (mesh)
                {
                    mesh->Render(*myShader, aScene.mainCamera ? aScene.mainCamera->GetViewProjectionMatrix() : glm::mat4(1.0f));
                }
            }
        }
    }

    myShader->Unuse();
}
