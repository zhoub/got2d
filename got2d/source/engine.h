#pragma once
#include "../include/g2dengine.h"
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

	void RemoveScene(::Scene& scene);

public: //g2d::engine 
	virtual g2d::RenderSystem* GetRenderSystem() override { return &m_renderSystem; }

	virtual g2d::Scene* CreateNewScene(float boundSize) override;

	virtual void Update(uint32_t deltaTime) override;

	virtual void OnMessage(const g2d::Message& message) override;

	virtual bool OnResize(uint32_t width, uint32_t height) override;
	
private:
	uint32_t m_elapsedTime = 0;

	void* nativeWindow = nullptr;
	RenderSystem m_renderSystem;
	std::string m_resourceRoot;
	std::vector<::Scene*> m_scenes;
};

inline Engine* GetEngineImpl()
{
	return ::Engine::Instance; 
}