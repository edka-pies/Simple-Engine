#include "AssetViewer.h"
#include "Mesh.h"
#include "EditorCore.h"
#include "Object.h"
#include "MessageBus.h"
#include "Scene.h"
#include "Light.h"
#include "UITools.h"
#include "Camera.h"
#include "Texture.h"
#include "TextureManager.h"	
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
	// We use a standard for loop to ensure we aren't hitting nulls
	for (size_t i = 0; i < masterObjectList->size(); ++i)
	{
		Object* obj = (*masterObjectList)[i];
		if (!obj) continue;

		ObjectData data;
		data.objetName = obj->GetName();

		if (obj->GetMesh())
		{
			data.position = obj->GetMesh()->GetTransform().position;
			data.eulerRotation = obj->GetMesh()->GetTransform().rotation;
			data.scale = obj->GetMesh()->GetTransform().scale;
		}
		data.isInitialized = true;

		objectDataList.push_back(data); // This populates the list
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

		// Check if our selected object is still in the new master list
		bool stillExists = false;
		for (Object* obj : *masterObjectList)
		{
			if (obj == selectedObject) {
				stillExists = true;
				break;
			}
		}

		// Only collapse the UI if the object was deleted
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
			// SYNC CHECK: If for some reason the data list is smaller than the object list, fix it
			if (objectDataList.size() != masterObjectList->size())
			{
				Init();
			}

			selectedObject = object;

			// SAFE ACCESS: Only assign if the index is valid
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
					selectedObject->GetMesh()->SetPosition(newPosition);
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
					selectedObject->GetMesh()->SetRotation(newRotation);
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
					selectedObject->GetMesh()->SetScale(newScale);
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
				// No local initialization needed!
				auto tex = TextureManager::GetInstance().GetTexture(texturePath);

				if (selectedObject->GetMesh())
				{
					selectedObject->GetMesh()->SetTexture(tex);
				}
			}
		}
	}
	if (ImGui::Button("Spawn New Model"))
	{
		if (strlen(modelPath) > 0)
		{
			// 2. Package the request into a message
			auto msg = std::make_unique<ObjectSpawnedMessage>(modelPath);

			// 3. Drop it in the post box
			MessageBus::GetInstance().EnqueueMessage(std::move(msg));
		}
	}

	if (ImGui::Button("Delete Selected Object"))
	{
		if (selectedObject && activeScene)
		{
			// 1. Send the message
			auto msg = std::make_unique<ObjectDeletedMessage>(selectedObject);
			MessageBus::GetInstance().EnqueueMessage(std::move(msg));

			// 2. Immediately clear the UI pointers so we don't try to draw a ghost
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

		ImGui::Separator();

		for (size_t i = 0; i < activeScene->lights.size(); i++) {
			Light* l = activeScene->lights[i];
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
	if (ImGui::CollapsingHeader("Texture Settings")) {
		const char* filters[] = { "Point (Nearest)", "Bilinear", "Trilinear (MipMaps)" };
		static int current_filter = 2;

		if (ImGui::Combo("Filtering Mode", &current_filter, filters, IM_ARRAYSIZE(filters))) {
			GLint glFilter;
			if (current_filter == 0) glFilter = GL_NEAREST;
			else if (current_filter == 1) glFilter = GL_LINEAR;
			else glFilter = GL_LINEAR_MIPMAP_LINEAR;

			// Apply to the current object's texture
			if (selectedObject->GetMesh()->GetTexture()) {
				selectedObject->GetMesh()->GetTexture()->SetFiltering(glFilter);
			}
		}
	}
	ImGui::End();
}

void AssetViewer::AssignSelectedObject(Object* otherObject)
{
	selectedObject = otherObject;
}
