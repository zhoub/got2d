#pragma once
#include <g2dengine.h>
#include "render_system.h"
#include "scene.h"

class Engine : public g2d::Engine
{
	RTTI_IMPL;
public:
	static Engine* Instance;

	~Engine();

	bool CreateRenderSystem(void* nativeWindow);

	void SetResourceRoot(const std::string& resPath);

	const std::string& GetResourceRoot() const { return m_resourceRoot; }

public: //g2d::engine 
	virtual g2d::RenderSystem* GetRenderSystem() override { return &m_renderSystem; }

	virtual g2d::Scene* CreateNewScene(float boundSize) override;
	
private:
	void* nativeWindow = nullptr;
	RenderSystem m_renderSystem;
	std::string m_resourceRoot;
};

inline Engine* GetEngineImpl()
{
	return ::Engine::Instance; 
}