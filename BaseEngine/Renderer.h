#pragma once
#include "RenderPass.h"
#include <memory>

class EngineContext;
class Renderer
{
public:

	virtual void Init() = 0;
	virtual void BeginFrame(Scene& aScene) = 0;
	virtual void RenderFrame(Scene& aScene) = 0;
	virtual void EndFrame(Scene& aScene) = 0;
	EngineContext* context;
	
protected:
	std::vector<std::unique_ptr<RenderPass>> myPasses;
};

