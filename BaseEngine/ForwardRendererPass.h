#pragma once
#include "RenderPass.h"
#include "Shader.h"

class ForwardRendererPass : public RenderPass
{
public:
	ForwardRendererPass();
	void Init() override;
	void Execute(Scene& aScene) override;
private:
	Shader* myShader;
};

