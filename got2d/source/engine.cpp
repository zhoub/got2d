#include "engine.h"

g2d::Engine::~Engine() { }

bool g2d::InitEngine(const EngineConfig& config)
{
	auto& inst = ::Engine::Instance;

	if (inst) return false;
	inst = new ::Engine();
	if (!inst) return false;

	do
	{
		if (!inst->CreateRenderSystem(config.nativeWindow))
		{
			break;
		}

		inst->CreateNewScene();
		return true;
	} while (false);

	delete inst;
	inst = nullptr;
	return false;
}

void g2d::UninitEngine()
{
	if (::Engine::Instance)
	{
		delete ::Engine::Instance;
		::Engine::Instance = nullptr;
	}
}

g2d::Engine* g2d::GetEngine()
{
	return ::Engine::Instance;
}

Engine* Engine::Instance = nullptr;

Engine::~Engine()
{
	SD(m_currentScene);
	m_renderSystem.Destroy();
}

bool Engine::Update(unsigned long elapsedTime)
{
	m_currentScene->Update(elapsedTime);
	return true;
}
void Engine::Render()
{
	m_currentScene->Render();
}

bool Engine::CreateRenderSystem(void* nativeWindow)
{
	if (!m_renderSystem.Create(nativeWindow))
	{
		return false;
	}
	return true;
}

void Engine::CreateNewScene()
{
	SD(m_currentScene);
	m_currentScene = new Scene();
}