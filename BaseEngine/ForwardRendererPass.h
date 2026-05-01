#pragma once
#include "RenderPass.h"
#include "Shader.h"

class ForwardRendererPass : public RenderPass
{
public:
	ForwardRendererPass();
	void Init() override;
	void Execute(Scene& aScene) override;
    int windowWidth = 1920;
    int windowHeight = 1080;
private:
    Shader* myShader;
    Shader* shadowShader; // NEW: Dedicated shader for the depth pass

    // NEW: Shadow mapping variables
    unsigned int depthMapFBO;
    unsigned int depthMap;
    const unsigned int SHADOW_WIDTH = 2048;
    const unsigned int SHADOW_HEIGHT = 2048;
};

