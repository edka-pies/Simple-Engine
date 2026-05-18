#include "AssetViewer.h"
#include "EditorCore.h"
#include "Object.h"
#include "MessageBus.h"
#include "Scene.h"
#include "Light.h"
#include "MeshManager.h"
#include "Primitives.h"
#include "UITools.h"
#include "Camera.h"
#include "Texture.h"
#include "TextureManager.h"	
#include "MeshComponent.h"
#include <set>
#include <iostream>
#include <string>
#include <cstring>

AssetViewer::AssetViewer()
	: selectedObject(nullptr), currentData(nullptr), activeScene(nullptr), masterObjectList(nullptr)
{
	memset(texturePath, 0, sizeof(texturePath));
}

void AssetViewer::SetMasterObjectList(std::vector<Object*>* masterList)
{
	masterObjectList = masterList;
}

void AssetViewer::SetActiveScene(Scene* scene)
{
	activeScene = scene;
}	

void AssetViewer::Init()
{
	if (!masterObjectList) return;

	objectDataList.clear();
	for (size_t i = 0; i < masterObjectList->size(); ++i)
	{
		Object* obj = (*masterObjectList)[i];
		if (!obj) continue;

		ObjectData data;
		data.objetName = obj->GetName();

		auto meshComp = obj->GetComponent<MeshComponent>();
		if (meshComp && meshComp->mesh) {
			data.position = obj->GetTransform().position;
			data.eulerRotation = obj->GetTransform().rotation;
			data.scale = obj->GetTransform().scale;
		}
		data.isInitialized = true;

		objectDataList.push_back(data); 
	}
}

void AssetViewer::Draw()
{
	if (masterObjectList == nullptr)
	{
		ImGui::Begin("Asset Viewer");
		ImGui::Text("Error: Master Object List not linked!");
		ImGui::End();
		return;
	}

	if (objectDataList.size() != masterObjectList->size())
	{
		Init();

		bool stillExists = false;
		for (Object* obj : *masterObjectList)
		{
			if (obj == selectedObject) {
				stillExists = true;
				break;
			}
		}

		if (!stillExists) {
			selectedObject = nullptr;
			currentData = nullptr;
		}
	}

	ImGui::Begin("Asset Viewer");

	static char modelPath[256] = "";
	ImGui::InputText("Model File Path", modelPath, IM_ARRAYSIZE(modelPath));

	objectDataList.clear();
	objectDataList.reserve(masterObjectList->size());

	for (int i = 0; i < (int)masterObjectList->size(); ++i)
	{
		Object* object = (*masterObjectList)[i];
		std::string buttonLabel = object->GetName() + "##" + std::to_string(i);

		if (ImGui::Button(buttonLabel.c_str()))
		{
			if (objectDataList.size() != masterObjectList->size())
			{
				Init();
			}

			selectedObject = object;

			if (i < (int)objectDataList.size())
			{
				currentData = &objectDataList[i];
			}
			else
			{
				currentData = nullptr; // Prevent pointing to garbage
			}

			memset(texturePath, 0, sizeof(texturePath));
		}
	}

	if (selectedObject != nullptr && currentData != nullptr)
	{
		ImGui::NewLine();
		char nameBuff[256];
		strncpy_s(nameBuff, sizeof(nameBuff), selectedObject->GetName().c_str(), _TRUNCATE);

		if (ImGui::InputText("##name", nameBuff, sizeof(nameBuff)))
		{
			ScopedIndent indent;
			selectedObject->SetName(nameBuff);
		}

		if (ImGui::CollapsingHeader("Transform"))
		{
			ScopedIndent transformIndent;
			ImGui::NewLine();

			if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ScopedIndent indent;
				glm::vec3& newPosition = currentData->position;
				if (ImGui::InputFloat3("Position", &newPosition.x))
				{
					selectedObject->SetPosition(newPosition);
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
			if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ScopedIndent indent;
				glm::vec3& newRotation = currentData->eulerRotation;
				if (ImGui::InputFloat3("Rotation", &newRotation.x, "%.3f"))
				{
					selectedObject->SetRotation(newRotation);
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
			if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ScopedIndent indent;
				glm::vec3& newScale = currentData->scale;
				if (ImGui::InputFloat3("Scale", &newScale.x))
				{
					selectedObject->SetScale(newScale);
				}
				ImGui::SameLine();
			}
		}

		// Texture assignment
		ImGui::Separator();
		ImGui::Text("Texture");
		ImGui::InputText("Texture Path", texturePath, sizeof(texturePath));
		ImGui::NewLine();
		if (ImGui::Button("Load Texture"))
		{
			if (strlen(texturePath) > 0)
			{
				auto tex = TextureManager::GetInstance().GetTexture(texturePath);

				auto meshComp = selectedObject->GetComponent<MeshComponent>();
				if (meshComp && meshComp->mesh)
				{
					meshComp->mesh->SetTexture(tex);
				}
			}
		}
		ImGui::Separator();
		ImGui::Text("Mesh Data");

		static char meshPathBuffer[256] = "Assets/Models/plane.obj";
		ImGui::InputText("New Mesh Path", meshPathBuffer, sizeof(meshPathBuffer));

		if (ImGui::Button("Change Mesh"))
		{
			std::string newPath(meshPathBuffer);

			auto msg = std::make_unique<ChangeMeshMessage>(selectedObject, newPath);
			MessageBus::GetInstance().EnqueueMessage(std::move(msg)); 
		}
	}
	if (ImGui::Button("Spawn Cube"))
	{
		auto msg = std::make_unique<PrimitiveSpawnedMessage>(PrimitiveShape::Cube);
		MessageBus::GetInstance().EnqueueMessage(std::move(msg));
	}
	if (ImGui::Button("Spawn Quad"))
	{
		auto msg = std::make_unique<PrimitiveSpawnedMessage>(PrimitiveShape::Quad);
		MessageBus::GetInstance().EnqueueMessage(std::move(msg));
	}
	if (ImGui::Button("Spawn Plane"))
	{
		auto msg = std::make_unique<PrimitiveSpawnedMessage>(PrimitiveShape::Plane);
		MessageBus::GetInstance().EnqueueMessage(std::move(msg));
	}
	if (ImGui::Button("Spawn Sphere"))
	{
		auto msg = std::make_unique<PrimitiveSpawnedMessage>(PrimitiveShape::Sphere);
		MessageBus::GetInstance().EnqueueMessage(std::move(msg));
	}
	if (ImGui::Button("Spawn Cylinder"))
	{
		auto msg = std::make_unique<PrimitiveSpawnedMessage>(PrimitiveShape::Cylinder);
		MessageBus::GetInstance().EnqueueMessage(std::move(msg));
	}
	if (ImGui::Button("Spawn Capsule"))
	{
		auto msg = std::make_unique<PrimitiveSpawnedMessage>(PrimitiveShape::Capsule);
		MessageBus::GetInstance().EnqueueMessage(std::move(msg));
	}

	if (ImGui::Button("Spawn New Model"))
	{
		if (strlen(modelPath) > 0)
		{
			auto msg = std::make_unique<ObjectSpawnedMessage>(modelPath);
			MessageBus::GetInstance().EnqueueMessage(std::move(msg));
		}
	}

	if (ImGui::Button("Delete Selected Object"))
	{
		if (selectedObject && activeScene)
		{
			auto msg = std::make_unique<ObjectDeletedMessage>(selectedObject);
			MessageBus::GetInstance().EnqueueMessage(std::move(msg));

			selectedObject = nullptr;
			currentData = nullptr;
		}
	}
	if (ImGui::CollapsingHeader("Scene Lights")) {
		// Add Buttons
		if (ImGui::Button("Add Point Light"))
			MessageBus::GetInstance().EnqueueMessage(std::make_unique<CreateLightMessage>(LightType::Point));
		ImGui::SameLine();
		if (ImGui::Button("Add Directional"))
			MessageBus::GetInstance().EnqueueMessage(std::make_unique<CreateLightMessage>(LightType::Directional));
		ImGui::SameLine();
		if (ImGui::Button("Add Spot Light"))
			MessageBus::GetInstance().EnqueueMessage(std::make_unique<CreateLightMessage>(LightType::Spot));
		ImGui::Separator();

		if (activeScene != nullptr)
		{
			for (size_t i = 0; i < activeScene->lights.size(); i++) {
				Light* l = activeScene->lights[i];

				if (l == nullptr) continue;

				if (ImGui::TreeNode((void*)(intptr_t)i, "Light %d", (int)i)) {
					ImGui::ColorEdit3("Color", &l->color.x);
					ImGui::DragFloat("Strength", &l->strenght, 0.1f, 0.0f, 20.0f);

					if (l->type != LightType::Directional)
						ImGui::DragFloat3("Position", &l->position.x, 0.1f);

					if (l->type != LightType::Point)
						ImGui::DragFloat3("Direction", &l->direction.x, 0.01f, -1.0f, 1.0f);

					if (ImGui::Button("Delete Light")) {
						MessageBus::GetInstance().EnqueueMessage(std::make_unique<DeleteLightMessage>(l));
					}
					ImGui::TreePop();
				}
			}
		}
		else
		{
			// Helpful feedback for you in the editor
			ImGui::Text("No Active Scene Loaded.");
		}
	}
	if (ImGui::CollapsingHeader("Texture Settings")) {
		const char* filters[] = { "Point (Nearest)", "Bilinear", "Trilinear (MipMaps)" };
		static int current_filter = 2;
		if (selectedObject != nullptr)
		{
			if (ImGui::Combo("Filtering Mode", &current_filter, filters, IM_ARRAYSIZE(filters))) {
				GLint glFilter;
				if (current_filter == 0) glFilter = GL_NEAREST;
				else if (current_filter == 1) glFilter = GL_LINEAR;
				else glFilter = GL_LINEAR_MIPMAP_LINEAR;

				auto meshComp = selectedObject->GetComponent<MeshComponent>();
				if (meshComp && meshComp->mesh && meshComp->mesh->GetTexture()) {
					meshComp->mesh->GetTexture()->SetFiltering(glFilter);
				}
			}
		}
	}
	ImGui::End();
}

void AssetViewer::AssignSelectedObject(Object* otherObject)
{
	selectedObject = otherObject;
}
