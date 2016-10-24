#pragma once
#include <g2dengine.h>
#include "render_system.h"
#include "scene.h"

class Engine : public g2d::Engine
{
public:
	static Engine* Instance;
	~Engine();
	bool CreateRenderSystem(void* nativeWindow);
	void CreateNewScene();

public:
	virtual bool Update(unsigned long elapsedTime) override;
	virtual void Render() override;
	inline virtual g2d::RenderSystem* GetRenderSystem() override { return &m_renderSystem; }
	inline virtual g2d::Scene* GetCurrentScene() override { return m_currentScene; }

private:
	RenderSystem m_renderSystem;
	Scene* m_currentScene;
};

inline Engine* GetEngine() { return Engine::Instance; }