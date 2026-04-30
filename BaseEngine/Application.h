#pragma once
#include <string>
#include <vector>
#include "Light.h"

class Object;
class Renderer;
class EngineContext;
class Scene;
class EditorBase;
class Window;

class Application
{

public:

	Application();

	~Application();

	virtual void Init(const int width, const int height, const std::string& name);

	void Run();

protected:

	virtual void OnInit() {}
	virtual void OnUpdate(float deltaTime) {};
	virtual void OnCleanUp() {};

	inline Renderer& GetRenderer() { return *renderer; }
	inline Window& GetWindow() { return *window; }
	inline EngineContext& GetEngineContext() { return *engineContext; }
	inline Scene& GetScene() { return *scene; }
	inline EditorBase& GetEditor() { return *editor; }

private:
	void CleanUp();
	void KeyCallback();
	void MouseCallback();

	std::vector<Object*> masterObjectList;
	Window* window;
	Renderer* renderer;
	EngineContext* engineContext;
	Scene* scene;
	EditorBase* editor;
	Light* mainLight;
};

