#include "Application.h"
#include <glad/glad/glad.h>
#include "ForwardRenderer.h"
#include "EngineContext.h"
#include "MeshManager.h"
#include "Primitives.h"
#include "AssetViewer.h"
#include "ActiveRagdollComponent.h"
#include "Component.h"
#include "Scene.h"
#include "Object.h"
#include "Window.h"
#include "Light.h"
#include "MessageBus.h"
#include <glm/glm/glm.hpp>
#include <GLFW/Include/glfw3.h>
#include "imgui/imgui.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include "PrefabManager.h"
#include "AiComponent.h"
#include "PlayerInputComponent.h"
#include "MeshComponent.h"
#include "GroundSensorComponent.h"
#include "PlatformComponent.h"
#include "ExplosiveComponent.h"

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
		deltaTime *= scene->timeScale;

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

			if (editor->currentState == EngineState::Playing)
			{
				scene->levelTime += deltaTime;

				glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				cursorEnabled = false;

				physicsSystem.Update(deltaTime, masterObjectList);

				static bool eWasPressed = false;
				bool eIsPressed = (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);

				if (eIsPressed && !eWasPressed) {
					// 1. Create the Ray from the Camera
					// Safely get player position if it exists
					glm::vec3 playerPos = scene->playerObject ? scene->playerObject->GetTransform().position : glm::vec3(0.0f);
					Ray lookRay;
					lookRay.origin = playerPos + glm::vec3(0.0f, 1.0f, 0.0f);
					lookRay.direction = scene->mainCamera->GetFront();

					Object* closestHitObject = nullptr;
					float closestHitDistance = 9999.0f;
					float maxReach = 5.0f;

					for (Object* obj : scene->objects) {
						auto meshComp = obj->GetComponent<MeshComponent>();
						if (!meshComp || !meshComp->mesh) continue;
						if (obj == scene->playerObject) continue; // ECS: Don't click yourself!

						AABB localBox = meshComp->mesh->GetLocalAABB();
						AABB worldBox = GetWorldAABB(localBox, obj->GetModelMatrix());

						float hitDist = 0.0f;
						if (TestRayAABB(lookRay, worldBox, hitDist)) {
							if (hitDist > 0.0f && hitDist < maxReach && hitDist < closestHitDistance) {
								closestHitDistance = hitDist;
								closestHitObject = obj;
							}
						}
					}

					if (closestHitObject != nullptr) {
						std::cout << "Interacted with: " << closestHitObject->GetName() << "\n";
						if (closestHitObject->GetName() == "FinishLine") {
							std::cout << "Level Finished! Time: " << scene->levelTime << "\n";
							if (scene->levelTime < scene->bestTime) {
								scene->bestTime = scene->levelTime;
								std::cout << "NEW RECORD!\n";
								SaveLevel("Level_1.json");
							}
							editor->currentState = EngineState::MainMenu;
						}
					}
				}
				eWasPressed = eIsPressed;

				// --- DELETED OLD ENEMY LOOP ---
				// masterObjectList will automatically call Update() on your AIComponents!

				// TODO: Refactor Platform to use ECS later. Commented out to prevent crashes for now!
				// for (Platform* plat : scene->platforms) {
				// 	plat->Update(deltaTime, scene->playerObject);
				// }

				// --- PURE ECS CAMERA TRACKING ---
				glm::vec3 camTarget = glm::vec3(0.0f);
				if (scene->playerObject) {
					auto ragdoll = scene->playerObject->GetComponent<ActiveRagdollComponent>();
					if (ragdoll && ragdoll->bodyParts.size() > 1 && ragdoll->bodyParts[1]) {
						camTarget = ragdoll->bodyParts[1]->GetTransform().position;
					}
					else {
						camTarget = scene->playerObject->GetTransform().position;
					}
				}

				glm::vec3 cameraOffset = glm::vec3(2.5f, 5.0f, 10.0f);
				activeCamera->SetPosition(camTarget + cameraOffset);
			}
			else if (editor->currentState == EngineState::MainMenu || editor->currentState == EngineState::Paused)
			{
				glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				cursorEnabled = true;

			}
			else if (editor->currentState == EngineState::Editor) 
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

				static bool rWasPressed = false;
				bool rPressed = (glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS);

				if (rPressed && !rWasPressed) {
					std::cout << "R Key Pressed: Reloading...\n";
				}
				rWasPressed = rPressed;

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
			}
		}

		MessageBus::GetInstance().DispatchMessages();

		OnUpdate(deltaTime);

		GetScene().Update(deltaTime);

		engineContext->Draw();
		
		GLFWwindow* win = &GetWindow().GetWindow();
		
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

		editor->FrameRun(&window->GetWindow());
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

using json = nlohmann::json;

void Application::SaveLevel(const std::string& filename) {
	json levelData;

	levelData["level"]["bestTime"] = scene->bestTime;

	// 1. Save Player Position
	if (scene->playerObject) {
		levelData["player"]["posX"] = scene->playerObject->GetTransform().position.x;
		levelData["player"]["posY"] = scene->playerObject->GetTransform().position.y;
		levelData["player"]["posZ"] = scene->playerObject->GetTransform().position.z;
	}
	// 2. Save Objects
	levelData["objects"] = json::array();

	// Note: It is highly recommended to loop through masterObjectList here instead of scene->objects 
	// to ensure you don't miss anything!
	for (Object* obj : masterObjectList) {
		if (!obj->serialize) continue;
		if (obj == scene->playerObject) continue; // Don't save the active player avatar

		json jObj;
		jObj["name"] = obj->GetName();

		// Transforms
		jObj["posX"] = obj->GetTransform().position.x;
		jObj["posY"] = obj->GetTransform().position.y;
		jObj["posZ"] = obj->GetTransform().position.z;

		jObj["rotX"] = obj->GetTransform().rotation.x;
		jObj["rotY"] = obj->GetTransform().rotation.y;
		jObj["rotZ"] = obj->GetTransform().rotation.z;

		jObj["scaleX"] = obj->GetTransform().scale.x;
		jObj["scaleY"] = obj->GetTransform().scale.y;
		jObj["scaleZ"] = obj->GetTransform().scale.z;

		// AUTOMATIC COMPONENT SAVING
		jObj["components"] = json::array();
		for (auto& comp : obj->GetComponents()) {
			json jComp;
			jComp["type"] = comp->GetComponentName();
			comp->Serialize(jComp);
			jObj["components"].push_back(jComp);
		}

		levelData["objects"].push_back(jObj);
	}

	// Write to file
	std::ofstream file("Assets/Levels/" + filename);
	file << levelData.dump(4);
	std::cout << "Level Saved to " << filename << "\n";
}

void Application::LoadLevel(const std::string& filename, bool isEditorMode) {
	std::cout << "Loading Level: " << filename << "...\n";

	// 1. WIPE THE CURRENT SCENE
	for (Object* obj : masterObjectList) {
		delete obj;
	}
	masterObjectList.clear();
	scene->objects.clear();
	scene->lights.clear();
	scene->playerObject = nullptr;

	// 2. READ THE JSON
	std::ifstream file("Assets/Levels/" + filename);
	if (!file.is_open()) {
		std::cout << "Failed to find level file!\n";
		return;
	}

	json levelData;
	file >> levelData;

	scene->bestTime = levelData.contains("level") && levelData["level"].contains("bestTime")
		? (float)levelData["level"]["bestTime"]
		: 9999.0f;

	glm::vec3 savedPlayerPos(0.0f, 10.0f, 0.0f); // Default fallback spawn point

	if (levelData.contains("player")) {
		savedPlayerPos.x = levelData["player"].value("posX", 0.0f);
		savedPlayerPos.y = levelData["player"].value("posY", 10.0f);
		savedPlayerPos.z = levelData["player"].value("posZ", 0.0f);
	}

	// 4. RESTORE OBJECTS
	for (auto& jObj : levelData["objects"]) {
		std::string objName = jObj.value("name", "UnknownObject");
		glm::vec3 pos(jObj.value("posX", 0.0f), jObj.value("posY", 0.0f), jObj.value("posZ", 0.0f));

		// --- A. THE SPAWNERS (Check these first!) ---
		if (objName == "Enemy" || objName == "Enemy_Brain") {
			if (isEditorMode) {
				Object* dummy = new Object();
				dummy->SetName("Enemy");
				dummy->AddComponent(std::make_shared<MeshComponent>(Primitives::CreateCube(), "Primitive_Cube"));
				dummy->SetPosition(pos);
				scene->AddObjects(dummy);
				masterObjectList.push_back(dummy);
			}
			else {
				Object* enemyBrain = prefabManager.InstantiateRagdollPrefab("assets/prefabs/ragdoll_basic.json", pos, scene, masterObjectList, "Enemy");
				enemyBrain->SetName("Enemy");
				enemyBrain->AddComponent(std::make_shared<AIComponent>(scene));
				enemyBrain->AddComponent(std::make_shared<GroundSensorComponent>(scene));
			}
			continue; // Skip the rest of the loop for spawners!
		}

		if (objName == "Player" || objName == "PlayerSpawn" || objName == "Player_Brain") {
			if (isEditorMode) {
				// In Editor, put the dummy box where the spawner was placed
				Object* dummy = new Object();
				dummy->SetName("Player");
				dummy->AddComponent(std::make_shared<MeshComponent>(Primitives::CreateCube(), "Primitive_Cube"));
				dummy->SetPosition(pos);
				scene->AddObjects(dummy);
				masterObjectList.push_back(dummy);
			}
			else {
				// IN GAME: Spawn the actual ragdoll at the savedPlayerPos!
				Object* playerBrain = prefabManager.InstantiateRagdollPrefab("assets/prefabs/ragdoll_basic.json", savedPlayerPos, scene, masterObjectList, "Player");
				playerBrain->SetName("PlayerBrain");
				playerBrain->AddComponent(std::make_shared<PlayerInputComponent>(&GetWindow().GetWindow(), scene->mainCamera));
				playerBrain->AddComponent(std::make_shared<GroundSensorComponent>(scene));
				scene->playerObject = playerBrain;
			}
			continue;
		}

		// --- B. STANDARD SCENE PROPS ---
		Object* newObj = new Object();
		newObj->SetName(objName);

		// Load Transforms
		newObj->SetPosition(pos);
		newObj->SetRotation(glm::vec3(jObj.value("rotX", 0.0f), jObj.value("rotY", 0.0f), jObj.value("rotZ", 0.0f)));
		newObj->SetScale(glm::vec3(jObj.value("scaleX", 1.0f), jObj.value("scaleY", 1.0f), jObj.value("scaleZ", 1.0f)));

		// Load Components
		if (jObj.contains("components")) {
			for (auto& jComp : jObj["components"]) {
				std::string compType = jComp.value("type", "");

				if (compType == "MeshComponent") {
					auto meshComp = std::make_shared<MeshComponent>(nullptr, "");
					meshComp->Deserialize(jComp); // The primitive generation happens strictly inside here now!
					newObj->AddComponent(meshComp);
				}
				else if (compType == "CollisionComponent") {
					AABB defaultBox;
					auto colComp = std::make_shared<CollisionComponent>(defaultBox, true);
					colComp->Deserialize(jComp);

					// --- NEW: FORCE IT TO BE A SOLID WALL ---
					// Even if the JSON saved it as a moving object, level props must be static!
					colComp->isStatic = true;

					newObj->AddComponent(colComp);
				}
			}
		}

		// Legacy Platform Setup (To be refactored into a Component later)
		if (newObj->GetName() == "Platform") {
			auto platComp = std::make_shared<PlatformComponent>(scene);
			platComp->startPos = newObj->GetTransform().position;
			platComp->endPos = platComp->startPos + glm::vec3(0.0f, 0.0f, 15.0f);
			newObj->AddComponent(platComp);
		}

		if (newObj->GetName() == "ExplosiveBarrel") {
			newObj->AddComponent(std::make_shared<ExplosiveComponent>(scene));

			// Optionally force it to have a dynamic collider so you can kick it!
			auto colComp = newObj->GetComponent<CollisionComponent>();
			if (colComp) colComp->isStatic = false;
		}

		scene->AddObjects(newObj);
		masterObjectList.push_back(newObj);
	}

	scene->levelTime = 0.0f;
}

void Application::SetupSubscriptions()
{
	HandleObjectSpawned();
	HandleObjectDeleted();
	HandleChangeMesh();
	HandleCreateLight();
	HandleDeleteLight();
	HandlePrimitiveSpawned();
	HandleLoadLevel();
	HandleSaveLevel();
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

				Object* newObject = new Object();
				newObject->SetName("New Mesh Object");
				newObject->AddComponent(std::make_shared<MeshComponent>(mesh, spawnMsg->filePath));

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
				auto meshComp = changeMsg->targetObject->GetComponent<MeshComponent>();

				if (meshComp != nullptr) {
					// 2a. It already has one! Just swap the model and the path inside it.
					meshComp->mesh = newMesh;
					meshComp->meshPath = changeMsg->newFilePath;
				}
			}
			else {
				// 2b. It doesn't have one yet. Add a brand new component!
				changeMsg->targetObject->AddComponent(std::make_shared<MeshComponent>(newMesh, changeMsg->newFilePath));
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

void Application::HandleLoadLevel()
{
	MessageBus::GetInstance().Subscribe(MessageType::LoadLevel, [this](Message* msg)
		{
			auto* loadMsg = static_cast<LoadLevelMessage*>(msg);
			this->LoadLevel(loadMsg->levelName, loadMsg->isEditorMode);
		});
}
void Application::HandleSaveLevel()
{
	MessageBus::GetInstance().Subscribe(MessageType::SaveLevel, [this](Message* msg)
		{
			auto* saveMsg = static_cast<LoadLevelMessage*>(msg);
			this->SaveLevel(saveMsg->levelName);
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
				generatedMesh = Primitives::CreatePlane();
				objectName = "Primitive Plane";
				break;
			case PrimitiveShape::Quad:
				generatedMesh = Primitives::CreateQuad();
				objectName = "Primitive Quad";
				break;
			case PrimitiveShape::Sphere:
				generatedMesh = Primitives::CreateSphere();
				objectName = "Primitive Sphere";
				break;
			case PrimitiveShape::Capsule:
				generatedMesh = Primitives::CreateCapsule(0.5f, 1.0f); // Standard player size
				objectName = "Primitive Capsule";
				break;
			case PrimitiveShape::Cylinder:
				generatedMesh = Primitives::CreateCylinder();
				objectName = "Primitive Cylinder";
				break;
			}

			if (generatedMesh)
			{
				Object* newObject = new Object();
				newObject->AddComponent(std::make_shared<MeshComponent>(generatedMesh, objectName));

				if (spawnMsg->shapeType == PrimitiveShape::Cube) newObject->GetComponent<MeshComponent>()->meshPath = "Primitive_Cube";
				else if (spawnMsg->shapeType == PrimitiveShape::Sphere) newObject->GetComponent<MeshComponent>()->meshPath = "Primitive_Sphere";
				else if (spawnMsg->shapeType == PrimitiveShape::Plane) newObject->GetComponent<MeshComponent>()->meshPath = "Primitive_Plane";
				else if (spawnMsg->shapeType == PrimitiveShape::Quad) newObject->GetComponent<MeshComponent>()->meshPath = "Primitive_Quad";
				else if (spawnMsg->shapeType == PrimitiveShape::Cylinder) newObject->GetComponent<MeshComponent>()->meshPath = "Primitive_Cylinder";
				else if (spawnMsg->shapeType == PrimitiveShape::Capsule) newObject->GetComponent<MeshComponent>()->meshPath = "Primitive_Capsule";

				this->scene->AddObjects(newObject);

				newObject->AddComponent(std::make_shared<CollisionComponent>(generatedMesh->localAABB, true)); // Dynamic collider

				this->masterObjectList.push_back(newObject);

				std::cout << "Successfully spawned: " << objectName << "\n";
			}
		});
}
