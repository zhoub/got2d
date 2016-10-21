#include "engine.h"

g2d::Engine::~Engine() { }

g2d::Engine* g2d::CreateEngine(const EngineConfig& config)
{
	auto rst = new ::Engine();
	if (rst)
	{
		do
		{
			if (!rst->CreateRenderSystem(config.nativeWindow))
			{
				break;
			}
			
			return rst;
		} while (false);
		delete rst;
	}
	return nullptr;
}

bool Engine::Update(unsigned long elapsedTime)
{
	//render
	m_renderSystem.Clear();
	m_renderSystem.Render();
	m_renderSystem.Present();
	return true;
}

void Engine::Release()
{
	m_renderSystem.Destroy();
	delete this;
}

bool Engine::CreateRenderSystem(void* nativeWindow)
{
	if (!m_renderSystem.Create(nativeWindow))
	{
		return false;
	}
	return true;
}