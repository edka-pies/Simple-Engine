#pragma once
#include "EditorBase.h"
#include <iostream>
#include "AssetViewer.h"
#include "Object.h"
#include "Scene.h"
#include "MessageBus.h"
#include "Primitives.h"
#include "Camera.h"
#include <GLFW/Include/glfw3.h>

EditorBase::EditorBase() : ioPtr(nullptr), mainScale(0), assetViewer(nullptr), scene(nullptr)
{
}

EditorBase::~EditorBase()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorBase::Init(GLFWwindow* window, Scene* scene, std::vector<Object*>* masterList)
{
    this->scene = scene;

    mainScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ioPtr = &io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    assetViewer = std::make_unique<AssetViewer>();

    // 2. Pass the Scene and the Master List
    assetViewer->SetMasterObjectList(masterList);
    assetViewer->SetActiveScene(scene);

    // 3. Initialize UI data
    assetViewer->Init();
}

void EditorBase::UpdateAssetViewerList(std::vector<Object*> newList)
{
    assetViewer->Init();
}

void EditorBase::FrameRun(GLFWwindow* win)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
    if(currentState == EngineState::Editor)
    {
        if (assetViewer)
        {
            assetViewer.get()->Draw();
        }

        ImGui::Begin("Camera");
        if (scene && scene->mainCamera)
        {
            Camera* cam = scene->mainCamera;

            float fov = cam->GetFov();
            if (ImGui::SliderFloat("FOV", &fov, 1.0f, 179.0f))
            {
                cam->SetFov(fov);
            }

            float nearP = cam->GetNearPlane();
            float farP = cam->GetFarPlane();
            if (ImGui::InputFloat("Near Plane", &nearP))
            {
                if (nearP < 0.001f) nearP = 0.001f;
                cam->SetNearFar(nearP, farP);
            }
            if (ImGui::InputFloat("Far Plane", &farP))
            {
                if (farP <= nearP) farP = nearP + 0.1f;
                cam->SetNearFar(nearP, farP);
            }

            glm::vec3 pos = cam->GetPosition();
            ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
        }
        else
        {
            ImGui::Text("No active camera in scene");
        }
        ImGui::End();
    }

    if (scene != nullptr)
    {
        ImGui::Begin("Editor UI");
        if(currentState == EngineState::Editor)
        {
            if (ImGui::CollapsingHeader("File Operations", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // A static buffer to hold the text you type in the editor
                static char levelNameBuffer[256] = "Level_1.json";
                ImGui::InputText("Level File Name", levelNameBuffer, sizeof(levelNameBuffer));

                // SAVE BUTTON
                if (ImGui::Button("Save Current Scene", ImVec2(ImGui::GetContentRegionAvail().x, 30)))
                {
                    std::string fileName(levelNameBuffer);
                    // Ensure it ends in .json so you don't accidentally save extensionless files!
                    if (fileName.find(".json") == std::string::npos) fileName += ".json";

                    auto msg = std::make_unique<SaveLevelMessage>(fileName);
                    MessageBus::GetInstance().EnqueueMessage(std::move(msg));
                }

                ImGui::Separator();

                // LOAD BUTTON (For Editor Use)
                if (ImGui::Button("Load Scene into Editor", ImVec2(ImGui::GetContentRegionAvail().x, 30)))
                {
                    std::string fileName(levelNameBuffer);
                    if (fileName.find(".json") == std::string::npos) fileName += ".json";

                    auto msg = std::make_unique<LoadLevelMessage>(fileName, true);
                    MessageBus::GetInstance().EnqueueMessage(std::move(msg));

                    // Ensure we stay in Editor mode when loading from here!
                    currentState = EngineState::Editor;
                }
            }
            ImGui::Separator();
        }
        if (scene->isPlaying) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Red
            if (ImGui::Button("STOP MODE", ImVec2(ImGui::GetContentRegionAvail().x, 30))) {
                scene->isPlaying = false;
                currentState = EngineState::Editor;

                // --- PURE ECS RESET PLAYER ---
                if (scene->playerObject) {
                    scene->playerObject->SetPosition(glm::vec3(30.0f, 10.0f, 10.0f));
                    auto ragdoll = scene->playerObject->GetComponent<ActiveRagdollComponent>();
                    if (ragdoll) {
                        for (auto part : ragdoll->bodyParts) {
                            if (part) part->velocity = glm::vec3(0.0f);
                        }
                    }
                }
            }
            ImGui::PopStyleColor();
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green
            if (ImGui::Button("PLAY MODE", ImVec2(ImGui::GetContentRegionAvail().x, 30))) {
                scene->isPlaying = true;
                currentState = EngineState::MainMenu;
            }
            ImGui::PopStyleColor();
        }
        ImGui::Separator();
        if (currentState == EngineState::MainMenu) {
            // Unlock Cursor
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            ImGui::SetNextWindowPos(ImVec2(1920 / 2 - 100, 1080 / 2 - 100)); // Center screen
            ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("GET GOOD");
            ImGui::Separator();
            if (ImGui::Button("Play Level 1", ImVec2(200, 40))) {
                currentLoadedLevel = "Level_1.json";
                MessageBus::GetInstance().EnqueueMessage(std::make_unique<LoadLevelMessage>(currentLoadedLevel, false));
				currentState = EngineState::Playing;
            }
            if (ImGui::Button("Play Level 2", ImVec2(200, 40))) {
                currentLoadedLevel = "Level_2.json";
                MessageBus::GetInstance().EnqueueMessage(std::make_unique<LoadLevelMessage>(currentLoadedLevel, false));
                currentState = EngineState::Playing;
            }
            //if (ImGui::Button("Play Level 3", ImVec2(200, 40))) { LoadLevel("Level_3.json"); }
            if (ImGui::Button("Back to Editor", ImVec2(200, 40))) { 
                if (!currentLoadedLevel.empty()) {
                    // Notice the 'true' here! This tells your LoadLevel function to spawn Editor dummies.
                    MessageBus::GetInstance().EnqueueMessage(std::make_unique<LoadLevelMessage>(currentLoadedLevel, true));
                }
                currentState = EngineState::Editor;
                scene->isPlaying = false;
            }

            ImGui::End();
        }
        else if (currentState == EngineState::Playing) {
            // Lock Cursor for FPS controls
            scene->isPlaying = true;
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // Press Escape to Pause!
            static bool escWasPressed = false;
            bool escPressed = (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS);
            if (escPressed && !escWasPressed) {
                currentState = EngineState::Paused;
            }
            escWasPressed = escPressed;

            ImGui::SetNextWindowPos(ImVec2(10, 10)); // Top left corner
            ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
            ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs);
            
            ImGui::Text("Time: %.2f", scene->levelTime);
            
            if (scene->bestTime < 9999.0f) {
                ImGui::Text("Best: %.2f", scene->bestTime);
            }
            ImGui::End();
        }
        else if (currentState == EngineState::Paused) {
            // Unlock Cursor for UI
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            ImGui::SetNextWindowPos(ImVec2(1920 / 2 - 100, 1080 / 2 - 100));
            ImGui::Begin("Paused", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("PAUSED");
            ImGui::Separator();

            if (ImGui::Button("Resume", ImVec2(200, 40))) { currentState = EngineState::Playing; }
            if (ImGui::Button("Quit to Menu", ImVec2(200, 40))) { currentState = EngineState::MainMenu; }
            ImGui::End();
        }
        ImGui::End();
    }
}

void EditorBase::RenderEditor(GLFWwindow* window)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
