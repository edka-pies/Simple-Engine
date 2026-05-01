#include "Application.h"
#include <glad/glad/glad.h>
#include "ForwardRenderer.h"
#include "EngineContext.h"
#include "EditorBase.h"
#include "MeshManager.h"
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
		
		LightCreateInfo dirInfo;
		dirInfo.type = LightType::Directional;
		dirInfo.direction = glm::normalize(glm::vec3(0.2f, -1.0f, 0.3f));
		dirInfo.color = glm::vec3(1.0f, 1.0f, 1.0f); // White sunlight
		dirInfo.strenght = 1.0f;
		scene->lights.push_back(new Light(&dirInfo));

		// 2. Subscribe to Light Creation
		MessageBus::GetInstance().Subscribe(MessageType::CreateLight, [this](Message* msg) {
			auto* lightMsg = static_cast<CreateLightMessage*>(msg);

			LightCreateInfo info;
			info.type = lightMsg->typeToCreate;

			// Set some defaults based on type
			if (info.type == LightType::Point) {
				info.position = glm::vec3(0.0f, 2.0f, 0.0f);
				info.color = glm::vec3(1.0f, 0.5f, 0.2f); // Warm orange
			}
			else if (info.type == LightType::Directional) {
				info.direction = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
			}

			// Add to the scene's light list
			this->scene->lights.push_back(new Light(&info));
			std::cout << "Engine spawned a new light via UI request.\n";
			});
		
		MessageBus::GetInstance().Subscribe(MessageType::ObjectSpawned, [this](Message* msg)
			{
				// 1. Cast the generic message back to our specific spawn message
				auto* spawnMsg = static_cast<ObjectSpawnedMessage*>(msg);

				// 2. THIS IS YOUR EXACT LOGIC, MOVED HERE
				auto mesh = MeshManager::GetInstance().GetMesh(spawnMsg->filePath);
				if (mesh)
				{
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
		MessageBus::GetInstance().Subscribe(MessageType::ObjectDeleted, [this](Message* msg)
			{
				auto* deleteMsg = static_cast<ObjectDeletedMessage*>(msg);
				Object* target = deleteMsg->object;

				if (target && this->scene)
				{
					// 1. Remove from Scene
					this->scene->RemoveObject(target);

					// 2. Remove from Master List
					auto it = std::find(this->masterObjectList.begin(), this->masterObjectList.end(), target);
					if (it != this->masterObjectList.end())
					{
						this->masterObjectList.erase(it);
					}

					// 3. FINALLY delete the memory
					delete target;
					std::cout << "Engine successfully deleted object.\n";
				}
			});
		MessageBus::GetInstance().Subscribe(MessageType::DeleteLight, [this](Message* msg) {
			auto* deleteMsg = static_cast<DeleteLightMessage*>(msg);
			auto& lights = this->scene->lights;

			auto it = std::find(lights.begin(), lights.end(), deleteMsg->lightToDelete);
			if (it != lights.end()) {
				delete* it;      // Free the memory
				lights.erase(it); // Remove from the list
			}
			});

		GetEngineContext().Init();
		GetEditor().Init(&GetWindow().GetWindow(), &GetScene(), &masterObjectList);

		Object* startObject = new Object();
		startObject->SetName("Initial Fortification");
		auto startMesh = MeshManager::GetInstance().GetMesh("Assets/Models/FortificationsLevel5.obj");
		startObject->SetMesh(startMesh);
		scene->AddObjects(startObject);
		masterObjectList.push_back(startObject);

		glfwSetInputMode(&GetWindow().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		scene->activeTerrain = new Terrain(100, 100, 2.0f);

		OnInit();
	}
}

void Application::Run()
{
	float lastFrameTime = glfwGetTime();

	// mouse tracking
	bool firstMouse = true;
	double lastX = 0.0, lastY = 0.0;
	// initialize lastX/lastY from current cursor position
	{
		GLFWwindow* winInit = &GetWindow().GetWindow();
		double x, y;
		glfwGetCursorPos(winInit, &x, &y);
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	// state for cursor toggle handling
	bool cursorEnabled = false;
	bool altWasPressed = false;
	bool iWasPressed = false;

	while (!window->ShouldClose())
	{
		float currentFrameTime = glfwGetTime();
		float deltaTime = currentFrameTime - lastFrameTime;

		glfwPollEvents();

		// Simple keyboard-controlled camera (WSAD + Q/E for up/down)
		if (scene && scene->mainCamera)
		{
			GLFWwindow* win = &GetWindow().GetWindow();
			if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
				scene->mainCamera->ProcessKeyboard(Camera::FORWARD, deltaTime);
			if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
				scene->mainCamera->ProcessKeyboard(Camera::BACKWARD, deltaTime);
			if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
				scene->mainCamera->ProcessKeyboard(Camera::LEFT, deltaTime);
			if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
				scene->mainCamera->ProcessKeyboard(Camera::RIGHT, deltaTime);
			if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
				scene->mainCamera->ProcessKeyboard(Camera::DOWN, deltaTime);
			if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
				scene->mainCamera->ProcessKeyboard(Camera::UP, deltaTime);
		}

		MessageBus::GetInstance().DispatchMessages();

		OnUpdate(deltaTime);

		GetScene().Update(deltaTime);

		engineContext->Draw();

		// Poll events before reading input so we get updated states

		// Toggle cursor with Alt (show) and 'I' (hide / resume)
		{
			GLFWwindow* win = &GetWindow().GetWindow();

			bool altPressed = (glfwGetKey(win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) == GLFW_PRESS;
			if (altPressed && !altWasPressed)
			{
				// show cursor so the user can interact with OS / UI
				glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				cursorEnabled = true;
				// avoid a large jump when re-locking later
				firstMouse = true;
			}
			altWasPressed = altPressed;

			bool iPressed = (glfwGetKey(win, GLFW_KEY_I) == GLFW_PRESS);
			if (iPressed && !iWasPressed)
			{
				// hide and lock cursor back to the engine (resume mouse-look)
				glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				cursorEnabled = false;
				firstMouse = true;
			}
			iWasPressed = iPressed;
		}

		// Mouse-look: read cursor delta and forward to camera, but only when cursor is locked
		double xpos, ypos;
		GLFWwindow* win = &GetWindow().GetWindow();
		glfwGetCursorPos(win, &xpos, &ypos);
		if (scene && scene->mainCamera && !cursorEnabled)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{

				// compute deltas; invert y because window coords have origin at top-left
				float xoffset = static_cast<float>(xpos - lastX);
				float yoffset = static_cast<float>(lastY - ypos);

				// prevent a large jump on first re-lock
				if (firstMouse)
				{
					xoffset = 0.0f;
					yoffset = 0.0f;
					firstMouse = false;
				}

				scene->mainCamera->ProcessMouseMovement(xoffset, yoffset);
			}
		}
		lastX = xpos;
		lastY = ypos;

		editor->FrameRun();
		editor->RenderEditor(&window->GetWindow());
		window->SwapBuffer();

		lastFrameTime = currentFrameTime;

		if (scene->activeTerrain) {
			glm::vec3 camPos = scene->mainCamera->GetPosition();
			float groundHeight = scene->activeTerrain->GetHeightAt(camPos.x, camPos.z);

			// Uncomment to test: 
			// std::cout << "Cam Y: " << camPos.y << " | Ground Y: " << groundHeight << "\n";
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
