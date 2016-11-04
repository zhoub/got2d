#include "engine.h"

Engine* Engine::Instance = nullptr;

g2d::Engine::~Engine() { }

bool g2d::Engine::Initialize(const Config& config)
{
	auto& inst = ::Engine::Instance;

	if (inst) return false;
	inst = new ::Engine();
	if (!inst) return false;

	do
	{
		inst->SetResourceRoot(config.resourceFolderPath);

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

void g2d::Engine::Uninitialize()
{
	if (::Engine::Instance)
	{
		delete ::Engine::Instance;
		::Engine::Instance = nullptr;
	}
}

g2d::Engine* g2d::Engine::Instance()
{
	return ::Engine::Instance;
}

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
	nativeWindow = nativeWindow;
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

void Engine::SetResourceRoot(const char* resPath)
{
	if (resPath == nullptr || strlen(resPath) == 0)
		return;

	m_resourceRoot = resPath;
	char endWith = m_resourceRoot[m_resourceRoot.length() - 1];
	if (endWith != '/' && endWith != '\\')
	{
		m_resourceRoot += '\\';
	}
}