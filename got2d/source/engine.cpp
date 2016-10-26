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

g2d::Texture* Engine::LoadTexture(const char* path)
{


	std::string resourcePath = m_resourceRoot + path;
	return m_renderSystem.CreateTextureFromFile(resourcePath.c_str());
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