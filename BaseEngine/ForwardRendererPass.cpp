#include "ForwardRendererPass.h"
#include "Scene.h"
#include "Renderable.h"
#include "Mesh.h"
#include "MeshManager.h"
#include <memory>
#include <glm/glm/glm.hpp>
#include <GLFW/Include/glfw3.h>

ForwardRendererPass::ForwardRendererPass() : 
    myShader(nullptr)
    ,shadowShader(nullptr),
    depthMapFBO(0), depthMap(0)
{
    myShader = new Shader("Assets/Shaders/VertexShader.glsl", "Assets/Shaders/FragmentShader.glsl");
    shadowShader = new Shader("Assets/Shaders/shadow_vertex.glsl", "Assets/Shaders/shadow_fragment.glsl");
}

void ForwardRendererPass::Init()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);

    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Clamp to border so areas outside the light's view aren't in shadow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); 
    glReadBuffer(GL_NONE); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}
void ForwardRendererPass::Execute(Scene& aScene)
{
    if (!myShader || !shadowShader) return;

    // Calculate Light Space Matrix
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    if (!aScene.lights.empty()) {
        Light* sun = aScene.lights[0];

        glm::vec3 center = glm::vec3(0.0f);
        if (aScene.mainCamera) {
            center = aScene.mainCamera->GetPosition();
            float shadowRes = 2048.0f; 
            float worldSize = 80.0f;  
            float texelSize = worldSize / shadowRes;
            center.x = floor(center.x / texelSize) * texelSize;
            center.z = floor(center.z / texelSize) * texelSize;
        }

        // Create a larger Ortho Box
        float size = 40.0f;
        glm::mat4 lightProjection = glm::ortho(-size, size, -size, size, 0.1f, 100.0f);

        // Position the light
        glm::vec3 lightPos = center + (-sun->GetDirection() * 50.0f);
        glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0.0f, 1.0f, 0.0f));

        lightSpaceMatrix = lightProjection * lightView;
    }

    auto DrawAllGeometry = [&](Shader& shader, bool includeTerrain) {
        // Standalone meshes
        for (auto& mesh : aScene.renderables) {
            if (mesh) mesh->Render(shader, glm::mat4(1.0f));
        }

        // Objects
        for (Object* obj : aScene.objects) {
            if (obj && obj->GetMesh()) {
                glm::mat4 realMatrix = obj->GetModelMatrix();
                for (auto& mesh : obj->GetRenderables()) {
                    if (mesh) mesh->Render(shader, realMatrix);
                }
            }
        }

        // Draw the Player if in Play Mode
        if (aScene.isPlaying && aScene.player.visualObject) {
            aScene.player.visualObject->GetMesh()->Render(shader, aScene.player.GetModelMatrix());
        }

        // Terrain
        if (includeTerrain && aScene.activeTerrain) {
            aScene.activeTerrain->Render(shader, glm::mat4(1.0f));
        }
        };
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowShader->Use();
    shadowShader->SetMatrix(lightSpaceMatrix, "lightSpaceMatrix");
    shadowShader->Unuse();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, 1920, 1080);

    myShader->Use();

    // Set Camera Data
    if (aScene.mainCamera) {
        aScene.mainCamera->Update();
        myShader->SetMatrix(aScene.mainCamera->GetView(), "viewMatrix");
        myShader->SetMatrix(aScene.mainCamera->GetProjection(), "projectionMatrix");
        myShader->SetVec4(glm::vec4(aScene.mainCamera->GetPosition(), 1.0f), "eyePosition");
    }

    // Set Lighting Data
    if (!aScene.lights.empty()) {
        myShader->SetLights(aScene.lights);
    }
    myShader->SetMatrix(lightSpaceMatrix, "lightSpaceMatrix");

    // Bind the Shadow Map
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    myShader->SetInt(2, "shadowMap");

    myShader->SetVec4(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), "materialAmbient");
    myShader->SetVec4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "materialDiffuse");
    myShader->SetVec4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "materialSpecular");
    myShader->SetInt(32, "materialShininess");

    myShader->SetInt(0, "useTexture");
    myShader->SetFloat(1.0f, "textureMixFactor");
    myShader->SetInt(0, "diffuseTexture");

    DrawAllGeometry(*myShader, false);

    if (aScene.activeTerrain && aScene.mainCamera) {
        myShader->SetVec4(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), "materialDiffuse");
        aScene.activeTerrain->Render(*myShader, aScene.mainCamera->GetViewProjectionMatrix());
    }

    myShader->Unuse();
}
