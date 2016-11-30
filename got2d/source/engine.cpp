#include "engine.h"
#include "inner_utility.h"

struct ClassIDGenerator
{
	static unsigned Next()
	{
		static unsigned i = 0x1000;
		return i++;
	}
};
unsigned G2DAPI NextClassID()
{
	return ClassIDGenerator::Next();
}


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

		auto defaultScene = inst->CreateNewScene();
		inst->SetActiveScene(defaultScene);

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
	for (auto& s : m_sceneList)
	{
		SD(s);
	}
	m_renderSystem.Destroy();
}

g2d::Scene* Engine::CreateNewScene()
{
	auto newScenePtr = new Scene();
	m_sceneList.push_back(newScenePtr);
	return newScenePtr;
}

g2d::Scene* Engine::SetActiveScene(g2d::Scene* activeScene)
{
	auto rst = m_currentScene;
	m_currentScene = dynamic_cast<::Scene*>(activeScene);
	return rst;
}

bool Engine::Update(unsigned long elapsedTime)
{
	if (m_currentScene)
	{
		m_currentScene->Update(elapsedTime);
	}
	return true;
}

void Engine::Render()
{
	if (m_currentScene)
	{
		m_currentScene->Render();
	}
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