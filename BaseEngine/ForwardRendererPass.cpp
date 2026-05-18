#include "ForwardRendererPass.h"
#include "Scene.h"
#include "Renderable.h"
#include "MeshManager.h"
#include "Frustum.h"
#include "MeshComponent.h"
#include <memory>
#include <glm/glm/glm.hpp>
#include "CollisionComponent.h"
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

	// Update the Frustum for Culling
    Frustum cameraFrustum;
    if (aScene.mainCamera) {
        cameraFrustum.Update(aScene.mainCamera->GetViewProjectionMatrix());
    }

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

    auto DrawAllGeometry = [&](Shader& shader, bool includeTerrain, bool performCulling) {

        // Standalone meshes (If you are still using aScene.renderables)
        // NOTE: If you deleted the Renderable class, you might need to change aScene.renderables to store Mesh* instead.
        for (auto& mesh : aScene.renderables) {
            if (mesh) {
                shader.SetMatrix(glm::mat4(1.0f), "modelMatrix");
                mesh->Draw(shader); // Changed from Render to Draw!
            }
        }

        // Objects 
        for (Object* obj : aScene.objects) {

            auto meshComp = obj->GetComponent<MeshComponent>();
            if (obj && meshComp && meshComp->mesh) {

                // Assuming you renamed this to GetTransformMatrix() based on our previous step!
                // If it is still GetModelMatrix() in your Object class, leave it as GetModelMatrix().
                glm::mat4 realMatrix = obj->GetModelMatrix();

                // Frustum Culling
                if (performCulling) {
                    glm::vec3 minExt = meshComp->mesh->localAABB.minBounds;
                    glm::vec3 maxExt = meshComp->mesh->localAABB.maxBounds;

                    // Transform local bounding box to world space
                    glm::vec3 worldMin = realMatrix * glm::vec4(minExt, 1.0f);
                    glm::vec3 worldMax = realMatrix * glm::vec4(maxExt, 1.0f);

                    // Re-align min/max in case rotation flipped them
                    glm::vec3 actualMin = (glm::min)(worldMin, worldMax);
                    glm::vec3 actualMax = (glm::max)(worldMin, worldMax);

                    if (!cameraFrustum.IsBoxVisible(actualMin, actualMax)) {
                        continue;
                    }
                }

                // --- THE ECS FIX ---
                // 1. Tell the shader where the object is
                shader.SetMatrix(realMatrix, "modelMatrix");
                // 2. Tell the mesh to draw itself
                meshComp->mesh->Draw(shader);
            }
        }

        // Terrain
        if (includeTerrain && aScene.activeTerrain) {
            // If you haven't refactored Terrain yet, leave this as Render. 
            // If you have refactored it, change it to follow the same pattern!
            aScene.activeTerrain->Render(shader, glm::mat4(1.0f));
        }
        };
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowShader->Use();
    shadowShader->SetMatrix(lightSpaceMatrix, "lightSpaceMatrix");
    DrawAllGeometry(*shadowShader, false, false);
    shadowShader->Unuse();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, 1920, 1080);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    DrawAllGeometry(*myShader, false, true);

    if (aScene.activeTerrain && aScene.mainCamera) {
        myShader->SetVec4(glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), "materialDiffuse");
        aScene.activeTerrain->Render(*myShader, aScene.mainCamera->GetViewProjectionMatrix());
    }

    myShader->Unuse();
}
