#pragma once
#include <g2dengine.h>
#include "render_system.h"

class Engine : public g2d::Engine
{
public:
	static Engine* Instance;
	~Engine();
	bool CreateRenderSystem(void* nativeWindow);

public:
	virtual bool Update(unsigned long elapsedTime) override;
	inline virtual g2d::RenderSystem* GetRenderSystem() override { return &m_renderSystem; }

private:
	RenderSystem m_renderSystem;
};

inline Engine* GetEngine() { return Engine::Instance; }