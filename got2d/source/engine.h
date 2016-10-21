#pragma once
#include <g2dengine.h>
#include "render_system.h"

class Engine : public g2d::Engine
{
public:
	bool CreateRenderSystem(void* nativeWindow);
public:
	virtual bool Update(unsigned long elapsedTime) override;
	virtual void Release() override;

private:
	RenderSystem m_renderSystem;
};