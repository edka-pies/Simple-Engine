#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <memory>
#include <vector>
#include "EngineState.h"

class Object;
class AssetViewer;
class Scene;

class EditorBase
{
public:
	EditorBase();
	~EditorBase();

	void Init(GLFWwindow* window, Scene* scene, std::vector<Object*>* masterList);

	void UpdateAssetViewerList(std::vector<Object*> newList);

	void FrameRun(GLFWwindow* win);

	void RenderEditor(GLFWwindow* window);

	AssetViewer& GetAssetViewer() { return *assetViewer; }

	EngineState currentState = EngineState::Editor;

private:
	float mainScale;
	ImGuiIO* ioPtr;
	std::unique_ptr<AssetViewer> assetViewer;
	Scene* scene;
};
