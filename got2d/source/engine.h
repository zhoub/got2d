#pragma once
#include <g2dengine.h>
#include "render_system.h"
#include "scene.h"

class Engine : public g2d::Engine
{
	IMPL_CLASSID;
public:
	static Engine* Instance;
	~Engine();
	bool CreateRenderSystem(void* nativeWindow);
	void SetResourceRoot(const char* resPath);
	inline const std::string& GetResourceRoot() { return m_resourceRoot; }

public:
	virtual g2d::Scene* CreateNewScene() override;
	inline virtual g2d::Scene* GetCurrentScene() override { return m_currentScene; }
	//return lastActiveScene
	virtual g2d::Scene* SetActiveScene(g2d::Scene* activeScene) override;
	virtual bool Update(unsigned long elapsedTime) override;
	virtual void Render() override;
	inline virtual g2d::RenderSystem* GetRenderSystem() override { return &m_renderSystem; }
	

private:
	void* nativeWindow = nullptr;
	RenderSystem m_renderSystem;
	Scene* m_currentScene = nullptr;
	std::vector<Scene*> m_sceneList;
	std::string m_resourceRoot;
};

inline Engine* GetEngine() { return Engine::Instance; }