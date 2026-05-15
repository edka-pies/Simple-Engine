#include "Application.h"
#include <glad/glad/glad.h>
#include "ForwardRenderer.h"
#include "EngineContext.h"
#include "MeshManager.h"
#include "Primitives.h"
#include "AssetViewer.h"
#include "RagdollEntity.h"
#include "Scene.h"
#include "Object.h"
#include "Window.h"
#include "Light.h"
#include "Platform.h"
#include "MessageBus.h"
#include <glm/glm/glm.hpp>
#include <GLFW/Include/glfw3.h>
#include "imgui/imgui.h"
#include <nlohmann/json.hpp>
#include <fstream>

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

			if (editor->currentState == EngineState::Playing)
			{
				scene->levelTime += deltaTime;

				glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				cursorEnabled = false;

				glm::vec3 inputDir(0.0f);

				if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) inputDir.z -= 1.0f;
				if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) inputDir.z += 1.0f;
				if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) inputDir.x -= 1.0f;
				if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) inputDir.x += 1.0f;

				if (glm::length(inputDir) > 0.01f) {
					inputDir = glm::normalize(inputDir);
				}

				static bool eWasPressed = false;
				bool eIsPressed = (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);

				if (eIsPressed && !eWasPressed) {
					// 1. Create the Ray from the Camera
					Ray lookRay;
					lookRay.origin = scene->player.position + glm::vec3(0.0f, 1.0f, 0.0f);
					lookRay.direction = scene->mainCamera->GetFront();

					Object* closestHitObject = nullptr;
					float closestHitDistance = 9999.0f;
					float maxReach = 5.0f; // You can't press a button 100 meters away

					// 2. Loop through the world to find what we are looking at
					for (Object* obj : scene->objects) {
						if (!obj || !obj->GetMesh()) continue;
						if (obj == scene->player.visualObject) continue; // Don't click yourself!

						// Get the World AABB
						AABB localBox = obj->GetMesh()->GetLocalAABB();
						AABB worldBox = GetWorldAABB(localBox, obj->GetModelMatrix()); // Assuming you have this helper

						float hitDist = 0.0f;
						if (TestRayAABB(lookRay, worldBox, hitDist)) {
							// Did we hit it, is it within arm's reach, and is it the closest thing?
							if (hitDist > 0.0f && hitDist < maxReach && hitDist < closestHitDistance) {
								closestHitDistance = hitDist;
								closestHitObject = obj;
							}
						}
					}
					// 3. DO THE INTERACTION
					if (closestHitObject != nullptr) {
						std::cout << "Interacted with: " << closestHitObject->GetName() << "\n";

						// --- THE "FINISH LINE" LOGIC ---
						if (closestHitObject->GetName() == "FinishLine") {
							std::cout << "Level Finished! Time: " << scene->levelTime << "\n";

							// Did we beat the best time?
							if (scene->levelTime < scene->bestTime) {
								scene->bestTime = scene->levelTime;
								std::cout << "NEW RECORD!\n";

								// Immediately save the JSON so the record is permanent
								SaveLevel("Level_1.json"); // Assuming you know the current level name!
							}

							// Kick them back to the Main Menu to celebrate
							editor->currentState = EngineState::MainMenu;
						}
					}
				}
				eWasPressed = eIsPressed;

				static bool rKeyPressed = false; // Prevent holding the key down from firing 60 times a second

				if (glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS) {
					if (!rKeyPressed) {
						// Loop through every ragdoll in the scene and shatter them!
						for (auto* ragdoll : scene->ragdolls) {
							if (ragdoll) {
								ragdoll->BreakAllJoints();
							}
						}
						rKeyPressed = true;
					}
				}
				else {
					rKeyPressed = false;
				}

				// --- JUMP DEBOUNCER ---
				static bool spaceWasPressed = false;
				bool spaceIsPressed = (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS);
				bool wantJump = (spaceIsPressed && !spaceWasPressed); // Only true on the exact frame it's pressed!
				spaceWasPressed = spaceIsPressed;

				// --- DASH DEBOUNCER (Using Left Shift) ---
				static bool shiftWasPressed = false;
				bool shiftIsPressed = (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
				bool wantDash = (shiftIsPressed && !shiftWasPressed);
				shiftWasPressed = shiftIsPressed;

				glm::vec3 camForward = scene->mainCamera->GetFront();

				// Pass the new wantDash parameter to your Update function!
				if (scene->activeTerrain) {
					scene->player.Update(deltaTime, inputDir, wantJump, wantDash, camForward, *scene->activeTerrain, scene->objects);
				}

				for (Enemy* enemy : scene->enemies) {
					if (scene->activeTerrain) {
						enemy->Update(deltaTime, scene->player.position, scene->player.visualObject, *scene->activeTerrain, scene->objects);
					}
				}

				for (Platform* plat : scene->platforms) {
					plat->Update(deltaTime, scene->player);
				}

				for (auto* ragdoll : scene->ragdolls) {
					if (ragdoll) {
						ragdoll->UpdatePhysics(deltaTime);
					}
				}

				glm::vec3 cameraOffset = glm::vec3(2.5f, 5.0f, 10.0f);
				activeCamera->SetPosition(scene->player.position + cameraOffset);
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
	levelData["player"]["posX"] = scene->player.position.x;
	levelData["player"]["posY"] = scene->player.position.y;
	levelData["player"]["posZ"] = scene->player.position.z;

	// 2. Save Objects
	levelData["objects"] = json::array();
	for (Object* obj : scene->objects) {
		if (obj == scene->player.visualObject) continue; // Don't save the player as a static object!

		json jObj;
		jObj["name"] = obj->GetName();
		jObj["meshPath"] = obj->GetMeshFilePath(); // You need to add this getter!

		jObj["posX"] = obj->GetTransform().position.x;
		jObj["posY"] = obj->GetTransform().position.y;
		jObj["posZ"] = obj->GetTransform().position.z;

		// Rotation
		jObj["rotX"] = obj->GetTransform().rotation.x;
		jObj["rotY"] = obj->GetTransform().rotation.y;
		jObj["rotZ"] = obj->GetTransform().rotation.z;

		// ADD THIS: Scale
		jObj["scaleX"] = obj->GetTransform().scale.x;
		jObj["scaleY"] = obj->GetTransform().scale.y;
		jObj["scaleZ"] = obj->GetTransform().scale.z;

		levelData["objects"].push_back(jObj);
	}

	// Write to file
	std::ofstream file("Assets/Levels/" + filename);
	file << levelData.dump(4); // 4 spaces indentation for readable JSON
	std::cout << "Level Saved to " << filename << "\n";
}

void Application::LoadLevel(const std::string& filename) {
	std::cout << "Loading Level: " << filename << "...\n";

	// 1. WIPE THE CURRENT SCENE
	for (Object* obj : masterObjectList) {
		delete obj;
	}
	masterObjectList.clear();
	scene->objects.clear();
	scene->enemies.clear();
	scene->lights.clear();
	scene->platforms.clear();

	// 2. READ THE JSON
	std::ifstream file("Assets/Levels/" + filename);
	if (!file.is_open()) {
		std::cout << "Failed to find level file!\n";
		return;
	}

	json levelData;
	file >> levelData;

	if (levelData.contains("level") && levelData["level"].contains("bestTime")) {
		scene->bestTime = levelData["level"]["bestTime"];
	}
	else {
		scene->bestTime = 9999.0f; // Default for new levels
	}

	// 3. RESTORE PLAYER
	scene->player.position.x = levelData["player"]["posX"];
	scene->player.position.y = levelData["player"]["posY"];
	scene->player.position.z = levelData["player"]["posZ"];

	std::vector<Object*> tempRagdollParts;

	// 4. RESTORE OBJECTS
	for (auto& jObj : levelData["objects"]) {
		std::string path = jObj.value("meshPath", "");
		std::string objName = jObj.value("name", "UnknownObject");
		std::shared_ptr<Mesh> mesh = nullptr;

		// --- FIX 1: HANDLE PRIMITIVES VS FILES ---
		if (path == "Primitive_Cube") mesh = Primitives::CreateCube();
		else if (path == "Primitive_Sphere") mesh = Primitives::CreateSphere();
		else if (path == "Primitive_Plane") mesh = Primitives::CreatePlane();
		else if (path == "Primitive_Quad") mesh = Primitives::CreateQuad();
		else if (path == "Primitive_Cylinder") mesh = Primitives::CreateCylinder();
		else if (path == "Primitive_Capsule") mesh = Primitives::CreateCapsule(0.5f, 1.0f);
		else if (!path.empty()) {
			// It's a real 3D model file
			mesh = MeshManager::GetInstance().GetMesh(path);
		}

		if (mesh) {
			Object* newObj = new Object();
			newObj->SetName(objName);
			newObj->SetMesh(mesh);

			// Re-assign the path so it survives the NEXT time you save!
			newObj->SetMeshFilePath(path);

			// --- FIX 2: USE SETTERS TO RECALCULATE MATRICES ---
			glm::vec3 pos(
				jObj.value("posX", 0.0f),
				jObj.value("posY", 0.0f),
				jObj.value("posZ", 0.0f)
			);

			glm::vec3 rot(
				jObj.value("rotX", 0.0f),
				jObj.value("rotY", 0.0f),
				jObj.value("rotZ", 0.0f)
			);

			// Note: Scale defaults to 1.0f! If it defaulted to 0.0f, your objects would be invisible!
			glm::vec3 scale(
				jObj.value("scaleX", 1.0f),
				jObj.value("scaleY", 1.0f),
				jObj.value("scaleZ", 1.0f)
			);

			newObj->SetPosition(pos);
			newObj->SetRotation(rot);
			newObj->SetScale(scale);

			if (newObj->GetName() == "Enemy") {
				Enemy* newEnemy = new Enemy();
				newEnemy->visualObject = newObj;
				//newEnemy->AddComponent(std::make_shared<AIComponent>(playerBrain));
				scene->enemies.push_back(newEnemy);
			}

			if (newObj->GetName() == "Platform") {
				Platform* newPlat = new Platform();
				newPlat->visualObject = newObj;

				// Set the start and end points
				newPlat->startPos = newObj->GetTransform().position;
				// For now, let's just make it float 15 units forward on the Z axis
				newPlat->endPos = newPlat->startPos + glm::vec3(0.0f, 0.0f, 15.0f);

				scene->platforms.push_back(newPlat);
			}
			if (newObj->GetName() == "Ragdoll_Part") {
				// Add to a temporary list. Once we have 5 parts (Head, Torso, L-Arm, R-Arm, Legs),
				// we initialize the RagdollEntity.
				tempRagdollParts.push_back(newObj);

				if (tempRagdollParts.size() == 5) {
					RagdollEntity* newRagdoll = new RagdollEntity();
					newRagdoll->bodyParts = tempRagdollParts;

					// Remember the order based on how you spawned them:
					// [0] Head, [1] Torso, [2] Left Arm, [3] Right Arm, [4] Legs

					// 1. Connect Head to Torso (Rest length ~1.0)
					newRagdoll->joints.push_back(SpringConstraint{ tempRagdollParts[0], tempRagdollParts[1], 1.2f, 800.0f, 50.0f });

					// 2. Connect Left Arm to Torso (Rest length ~1.5)
					newRagdoll->joints.push_back(SpringConstraint{ tempRagdollParts[2], tempRagdollParts[1], 1.5f, 800.0f, 50.0f });

					// 3. Connect Right Arm to Torso (Rest length ~1.5)
					newRagdoll->joints.push_back(SpringConstraint{ tempRagdollParts[3], tempRagdollParts[1], 1.5f, 800.0f, 50.0f });

					// 4. Connect Legs to Torso (Rest length ~1.5)
					newRagdoll->joints.push_back(SpringConstraint{ tempRagdollParts[4], tempRagdollParts[1], 1.5f, 800.0f, 50.0f });

					scene->ragdolls.push_back(newRagdoll);
					tempRagdollParts.clear();
				}
			}

			scene->AddObjects(newObj);
			masterObjectList.push_back(newObj);
		}
		else {
			std::cout << "WARNING: Failed to load mesh for object: " << jObj["name"] << " (Path: " << path << ")\n";
		}
	}
	scene->levelTime = 0.0f;

	// 5. START PLAYING
	//editor->currentState = EngineState::Playing;
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
				newObject->SetMesh(mesh);

				newObject->SetMeshFilePath(spawnMsg->filePath);

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

void Application::HandleLoadLevel()
{
	MessageBus::GetInstance().Subscribe(MessageType::LoadLevel, [this](Message* msg)
		{
			auto* loadMsg = static_cast<LoadLevelMessage*>(msg);
			this->LoadLevel(loadMsg->levelName);
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
				newObject->SetName(objectName);
				newObject->SetMesh(generatedMesh);

				if (spawnMsg->shapeType == PrimitiveShape::Cube) newObject->SetMeshFilePath("Primitive_Cube");
				else if (spawnMsg->shapeType == PrimitiveShape::Sphere) newObject->SetMeshFilePath("Primitive_Sphere");
				else if (spawnMsg->shapeType == PrimitiveShape::Plane) newObject->SetMeshFilePath("Primitive_Plane");
				else if (spawnMsg->shapeType == PrimitiveShape::Quad) newObject->SetMeshFilePath("Primitive_Quad");
				else if (spawnMsg->shapeType == PrimitiveShape::Cylinder) newObject->SetMeshFilePath("Primitive_Cylinder");
				else if (spawnMsg->shapeType == PrimitiveShape::Capsule) newObject->SetMeshFilePath("Primitive_Capsule");

				this->scene->AddObjects(newObject);

				this->masterObjectList.push_back(newObject);

				std::cout << "Successfully spawned: " << objectName << "\n";
			}
		});
}
