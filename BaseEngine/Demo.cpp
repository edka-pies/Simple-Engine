#include "Demo.h"
#include "ForwardRenderer.h"
#include "EngineContext.h"
#include "EditorBase.h"
#include "MeshManager.h"
#include "Scene.h"
#include "Object.h"
#include "Window.h"

Demo::Demo()
{

}

void Demo::OnInit()
{
	MemoryCheckInstance = new MemoryCheck();
	MemoryCheckInstance->CalculateMemoryUsage();
}

void Demo::OnUpdate(float deltaTime)
{
}

void Demo::OnCleanUp()
{
}