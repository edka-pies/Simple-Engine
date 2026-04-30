#include "EditorBase.h"
#include <GLFW/Include/glfw3.h>
#include <iostream>
#include "AssetViewer.h"
#include "Object.h"
#include "Scene.h"
#include "Camera.h"

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

void EditorBase::FrameRun()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
    if (assetViewer)
    {
        assetViewer.get()->Draw();
    }

    // Camera inspector
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
            // clamp a bit
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

void EditorBase::RenderEditor(GLFWwindow* window)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
