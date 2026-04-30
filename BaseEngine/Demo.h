#pragma once
#include "BaseEngineCore.h"
#include "Application.h"
#include "Mesh.h"
#include "MemoryCheck.h"

class Demo : public Application
{
	
public:
	Demo();

private:
	virtual void OnInit() override;
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnCleanUp() override;

	MemoryCheck* MemoryCheckInstance;
};

