#pragma once
#include "Scene.h"

class RenderPass
{
public:
	virtual void Init() = 0;
	virtual void Execute(Scene& aScene) = 0;
};

