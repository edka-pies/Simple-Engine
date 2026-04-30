#include "EngineContext.h"
#include "Renderer.h"
#include "Scene.h"
#include "EditorBase.h"
#include "Object.h"

EngineContext::EngineContext(Scene* aScene, Renderer* aRenderer, EditorBase* aEditor)
	: scene(aScene)
	, renderer(aRenderer)
	, editor(aEditor)
{
}

void EngineContext::Init()
{
}

void EngineContext::Draw()
{
	renderer->BeginFrame(*scene);
	renderer->RenderFrame(*scene);
	renderer->EndFrame(*scene);
}
