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
    // NEW: Load the shadow shaders (we will write these in Phase 3)
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
    glDrawBuffer(GL_NONE); // We don't draw colors
    glReadBuffer(GL_NONE); // We don't read colors
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Return to default screen
}
void ForwardRendererPass::Execute(Scene& aScene)
{
    if (!myShader || !shadowShader) return;

    // 1. Calculate Light Space Matrix (Assuming light[0] is your Directional Sunlight)
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    if (!aScene.lights.empty()) {
        Light* sun = aScene.lights[0];
        // The "Camera" projection for the sun
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 50.0f);

        // Push the light position artificially backward along its direction line
        glm::vec3 lightPos = -sun->GetDirection() * 20.0f;
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        lightSpaceMatrix = lightProjection * lightView;
    }

    // Helper Lambda function to draw all geometry (Saves us from writing loops twice)
    auto DrawAllGeometry = [&](Shader& shader) {
        for (auto& mesh : aScene.renderables) {
            if (mesh) mesh->Render(shader, glm::mat4(1.0f)); // Assuming Render applies the transform
        }
        for (Object* obj : aScene.objects) {
            if (obj) {
                for (auto& mesh : obj->GetRenderables()) {
                    if (mesh) mesh->Render(shader, glm::mat4(1.0f));
                }
            }
        }
        };

    // ==========================================
    // PASS 1: Render Shadows
    // ==========================================
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT); // Only clear depth!

    shadowShader->Use();
    shadowShader->SetMatrix(lightSpaceMatrix, "lightSpaceMatrix");
    DrawAllGeometry(*shadowShader);
    shadowShader->Unuse();

    // ==========================================
    // PASS 2: Render Scene Normally
    // ==========================================
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Back to screen

    // IMPORTANT: Change 1920, 1080 to your actual Window Width/Height!
    glViewport(0, 0, 1920, 1080);

    // Don't call glClear here if Application.cpp / EngineContext is already doing it!
    // But if you must: glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    // Bind the Shadow Map to Texture Slot 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    myShader->SetInt(2, "shadowMap");

    // Default Material Setters...
    myShader->SetVec4(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), "materialAmbient");
    myShader->SetVec4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "materialDiffuse");
    myShader->SetVec4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "materialSpecular");
    myShader->SetInt(32, "materialShininess");

    myShader->SetInt(0, "useTexture");
    myShader->SetFloat(1.0f, "textureMixFactor");
    myShader->SetInt(0, "diffuseTexture");

    // Draw everything again with the main shader
    DrawAllGeometry(*myShader);

    myShader->Unuse();
}
