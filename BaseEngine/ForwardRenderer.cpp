#include "ForwardRenderer.h"
#include "ForwardRendererPass.h"
#include "Scene.h"
#include "MemoryCheck.h"

 #include <GLFW/Include/glfw3.h>

void ForwardRenderer::Init()
{
	std::unique_ptr<ForwardRendererPass> pass = std::make_unique<ForwardRendererPass>();

	myPasses.push_back(std::move(pass));
}

void ForwardRenderer::BeginFrame(Scene& aScene)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 0.6f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ForwardRenderer::RenderFrame(Scene& aScene)
{
	for (size_t i = 0; i < myPasses.size(); i++)
	{
		myPasses[i]->Execute(aScene);
	}
}

void ForwardRenderer::EndFrame(Scene& aScene)
{

}
