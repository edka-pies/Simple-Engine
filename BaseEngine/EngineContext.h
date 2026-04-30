#pragma once

#include "Scene.h"
#include "Renderer.h"
#include "EditorBase.h"

class EngineContext
{
public:
	EngineContext(Scene* aScene, Renderer* aRenderer, EditorBase* aEditor);

	void Init();

	void Draw();

private:
	Scene* scene;
	EditorBase* editor;
	Renderer* renderer;
};

