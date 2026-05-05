#include "Application.h"
#include <glad/glad/glad.h>
#include "ForwardRenderer.h"
#include "EngineContext.h"
#include "EditorBase.h"
#include "MeshManager.h"
#include "Primitives.h"
#include "AssetViewer.h"
#include "Scene.h"
#include "Object.h"
#include "Window.h"
#include "Light.h"
#include "MessageBus.h"
#include <glm/glm/glm.hpp>
#include <GLFW/Include/glfw3.h>
#include "imgui/imgui.h"

Application::Application()
	: window(nullptr)
	, renderer(nullptr)
	, scene(nullptr)
	, engineContext(nullptr)
	, editor(nullptr)
	, mainLight(nullptr)
{
}

Application::~Application()
{
	for (Object* obj : masterObjectList) 
	{
		delete obj;
	}
}

void Application::Init(const int width, const int height, const std::string& name)
{
	if (engineContext == nullptr)
	{

		window = new Window(width, height, name);
		scene = new Scene();
		renderer = new ForwardRenderer();
		editor = new EditorBase();
		engineContext = new EngineContext(scene, renderer, editor);

		GetWindow().Init();

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return;
		}

		GetRenderer().Init();
		GetScene().Init();
		SetupSubscriptions();
		GetEngineContext().Init();
		GetEditor().Init(&GetWindow().GetWindow(), &GetScene(), &masterObjectList);

		auto msg = std::make_unique<ObjectSpawnedMessage>("Assets/Models/FortificationsLevel5.obj");

		MessageBus::GetInstance().EnqueueMessage(std::move(msg));

		glfwSetInputMode(&GetWindow().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		scene->activeTerrain = new Terrain(100, 100, 2.0f);

		auto editorCam = std::make_shared<Camera>();
		editorCam->Init();
		editorCam->SetPosition(glm::vec3(0.0f, 10.0f, 10.0f));

		auto playerCam = std::make_shared<Camera>();
		playerCam->Init();

		cameraManager.AddCamera(editorCam); 
		cameraManager.AddCamera(playerCam); 

		MessageBus::GetInstance().EnqueueMessage(std::make_unique<CreateLightMessage>(LightType::Directional));

		OnInit();
	}
}

void Application::Run()
{
	float lastFrameTime = glfwGetTime();

	// mouse tracking
	bool firstMouse = true;
	double lastX = 0.0, lastY = 0.0;
	{
		GLFWwindow* winInit = &GetWindow().GetWindow();
		double x, y;
		glfwGetCursorPos(winInit, &x, &y);
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	//cursor toggle handling
	bool cursorEnabled = false;
	bool altWasPressed = false;
	bool iWasPressed = false;

	while (!window->ShouldClose())
	{
		float currentFrameTime = glfwGetTime();
		float deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		if (deltaTime > 0.25f) deltaTime = 0.25f;

		glfwPollEvents();

		if (scene->isPlaying) {
			cameraManager.SetActiveCamera(1); // Player Cam
		}
		else {
			cameraManager.SetActiveCamera(0); // Editor Cam
		}

		auto activeCamera = cameraManager.GetActiveCamera();

		if (activeCamera)
		{
			GLFWwindow* win = &GetWindow().GetWindow();

			if (scene->isPlaying)
			{
				glm::vec3 inputDir(0.0f);

				if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) inputDir.z -= 1.0f;
				if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) inputDir.z += 1.0f;
				if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) inputDir.x -= 1.0f;
				if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) inputDir.x += 1.0f;

				if (glm::length(inputDir) > 0.01f) {
					inputDir = glm::normalize(inputDir);
				}

				bool wantJump = (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS);

				if (scene->activeTerrain) {
					scene->player.Update(deltaTime, inputDir, wantJump, *scene->activeTerrain, scene->objects);
				}

				glm::vec3 cameraOffset = glm::vec3(2.5f, 5.0f, 10.0f);
				activeCamera->SetPosition(scene->player.position + cameraOffset);
			}
			else
			{
				if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
					activeCamera->ProcessKeyboard(Camera::FORWARD, deltaTime);
				if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
					activeCamera->ProcessKeyboard(Camera::BACKWARD, deltaTime);
				if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
					activeCamera->ProcessKeyboard(Camera::LEFT, deltaTime);
				if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
					activeCamera->ProcessKeyboard(Camera::RIGHT, deltaTime);
				if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
					activeCamera->ProcessKeyboard(Camera::DOWN, deltaTime);
				if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
					activeCamera->ProcessKeyboard(Camera::UP, deltaTime);
			}
		}

		MessageBus::GetInstance().DispatchMessages();

		OnUpdate(deltaTime);

		GetScene().Update(deltaTime);

		engineContext->Draw();
		
		GLFWwindow* win = &GetWindow().GetWindow();

		bool altPressed = (glfwGetKey(win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) == GLFW_PRESS;
		if (altPressed && !altWasPressed)
		{
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			cursorEnabled = true;
			firstMouse = true;
		}
		altWasPressed = altPressed;

		bool iPressed = (glfwGetKey(win, GLFW_KEY_I) == GLFW_PRESS);
		if (iPressed && !iWasPressed)
		{
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			cursorEnabled = false;
			firstMouse = true;
		}
		iWasPressed = iPressed;
		
		double xpos, ypos;
		glfwGetCursorPos(win, &xpos, &ypos);
		if (activeCamera && !cursorEnabled) 
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				float xoffset = static_cast<float>(xpos - lastX);
				float yoffset = static_cast<float>(lastY - ypos);

				if (firstMouse)
				{
					xoffset = 0.0f;
					yoffset = 0.0f;
					firstMouse = false;
				}

				activeCamera->ProcessMouseMovement(xoffset, yoffset);
			}
		}
		lastX = xpos;
		lastY = ypos;

		// Update the matrix, then pass the pointer to the Scene
		if (activeCamera) {
			activeCamera->Update();
			scene->mainCamera = activeCamera.get();
		}

		editor->FrameRun();
		editor->RenderEditor(&window->GetWindow());
		window->SwapBuffer();

		lastFrameTime = currentFrameTime;

		if (scene->activeTerrain) {
			glm::vec3 camPos = scene->mainCamera->GetPosition();
			float groundHeight = scene->activeTerrain->GetTriangleHeightAt(camPos.x, camPos.z);
		}
	}

	CleanUp();
}


void Application::CleanUp()
{
	OnCleanUp();
	glfwTerminate();
	delete engineContext;
	delete scene;

}

void Application::KeyCallback()
{

}

void Application::MouseCallback()
{
}

void Application::SetupSubscriptions()
{
	HandleObjectSpawned();
	HandleObjectDeleted();
	HandleChangeMesh();
	HandleCreateLight();
	HandleDeleteLight();
	HandlePrimitiveSpawned();
}

void Application::HandleObjectSpawned()
{
	MessageBus::GetInstance().Subscribe(MessageType::ObjectSpawned, [this](Message* msg)
		{
			auto* spawnMsg = static_cast<ObjectSpawnedMessage*>(msg);

			auto mesh = MeshManager::GetInstance().GetMesh(spawnMsg->filePath);
			if (mesh)
			{
				const auto& verts = mesh->GetVertices();

				if (verts.empty()) {
					std::cout << "CRITICAL WARNING: " << spawnMsg->filePath << " has 0 vertices on the CPU! Collisions will fail.\n";
				}
				else {
					mesh->localAABB = Mesh::CalculateLocalAABB(verts);
				}
				// ==========================================

				Object* newObject = new Object();
				newObject->SetName("New Mesh Object");
				newObject->SetMesh(mesh);

				// Add to Scene
				this->scene->AddObjects(newObject);

				// Add to Application's Master List
				this->masterObjectList.push_back(newObject);

				std::cout << "Successfully spawned: " << spawnMsg->filePath << "\n";
			}
			else
			{
				std::cout << "Failed to load mesh at: " << spawnMsg->filePath << "\n";
			}
		});
}

void Application::HandleObjectDeleted()
{
	MessageBus::GetInstance().Subscribe(MessageType::ObjectDeleted, [this](Message* msg)
		{
			auto* deleteMsg = static_cast<ObjectDeletedMessage*>(msg);
			Object* target = deleteMsg->object;

			if (target && this->scene)
			{
				this->scene->RemoveObject(target);

				auto it = std::find(this->masterObjectList.begin(), this->masterObjectList.end(), target);
				if (it != this->masterObjectList.end())
				{
					this->masterObjectList.erase(it);
				}

				delete target;
				std::cout << "Engine successfully deleted object.\n";
			}
		});
}

void Application::HandleChangeMesh()
{
	MessageBus::GetInstance().Subscribe(MessageType::ChangeMesh, [this](Message* msg)
		{
			auto* changeMsg = static_cast<ChangeMeshMessage*>(msg);

			if (!changeMsg->targetObject) {
				std::cout << "Error: Tried to change mesh on a null object!\n";
				return;
			}

			auto newMesh = MeshManager::GetInstance().GetMesh(changeMsg->newFilePath);

			if (newMesh)
			{
				changeMsg->targetObject->SetMesh(newMesh);
				std::cout << "Successfully changed mesh to: " << changeMsg->newFilePath << "\n";
			}
			else
			{
				std::cout << "Failed to load new mesh at: " << changeMsg->newFilePath << "\n";
			}
		});
}

void Application::HandleCreateLight()
{
	MessageBus::GetInstance().Subscribe(MessageType::CreateLight, [this](Message* msg) {
		auto* lightMsg = static_cast<CreateLightMessage*>(msg);

		LightCreateInfo info;
		info.type = lightMsg->typeToCreate;

		if (info.type == LightType::Point) {
			info.position = glm::vec3(0.0f, 2.0f, 0.0f);
			info.color = glm::vec3(1.0f, 0.5f, 0.2f);
		}
		else if (info.type == LightType::Directional) {
			info.direction = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else if (info.type == LightType::Spot) {
			info.position = glm::vec3(0.0f, 5.0f, 0.0f);
			info.direction = glm::vec3(0.0f, -1.0f, 0.0f);
		}

		this->scene->lights.push_back(new Light(&info));
		std::cout << "Engine spawned a new light via UI request.\n";
		});
}

void Application::HandleDeleteLight()
{
	MessageBus::GetInstance().Subscribe(MessageType::DeleteLight, [this](Message* msg) {
		auto* deleteMsg = static_cast<DeleteLightMessage*>(msg);
		auto& lights = this->scene->lights;

		auto it = std::find(lights.begin(), lights.end(), deleteMsg->lightToDelete);
		if (it != lights.end()) {
			delete* it; 
			lights.erase(it); 
		}
		});
}

void Application::HandlePrimitiveSpawned()
{
	MessageBus::GetInstance().Subscribe(MessageType::PrimitiveSpawned, [this](Message* msg)
		{
			auto* spawnMsg = static_cast<PrimitiveSpawnedMessage*>(msg);

			std::shared_ptr<Mesh> generatedMesh = nullptr;
			std::string objectName = "New Primitive";

			switch (spawnMsg->shapeType) {
			case PrimitiveShape::Cube:
				generatedMesh = Primitives::CreateCube();
				objectName = "Primitive Cube";
				break;
			case PrimitiveShape::Plane:
				break;
			}

			if (generatedMesh)
			{
				Object* newObject = new Object();
				newObject->SetName(objectName);
				newObject->SetMesh(generatedMesh);

				this->scene->AddObjects(newObject);

				this->masterObjectList.push_back(newObject);

				std::cout << "Successfully spawned: " << objectName << "\n";
			}
		});
}
