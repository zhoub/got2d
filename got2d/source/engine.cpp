#include "engine.h"
#include "inner_utility.h"

#include <algorithm>

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

		auto defaultScene = inst->CreateNewScene(config.defaultSceneBounding);
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
		delete s;
	}
	for (auto& rs : m_releasedScene)
	{
		delete rs;
	}
	m_renderSystem.Destroy();
}

g2d::Scene* Engine::CreateNewScene(float boundSize)
{
	auto newScenePtr = new Scene(boundSize);
	m_sceneList.push_back(newScenePtr);
	return newScenePtr;
}

g2d::Scene* Engine::SetActiveScene(g2d::Scene* activeScene)
{
	auto rst = m_currentScene;
	if (std::find(m_releasedScene.begin(), m_releasedScene.end(), m_currentScene) != m_releasedScene.end())
	{
		rst = nullptr;
	}
	m_currentScene = dynamic_cast<::Scene*>(activeScene);
	return rst;
}

void Engine::ReleaseScene(g2d::Scene* deletedScene)
{
	::Scene* ds = dynamic_cast<::Scene*>(deletedScene);
	if (m_currentScene != ds)
	{
		std::remove_if(m_sceneList.begin(), m_sceneList.end(), [&ds](const ::Scene* s)->bool {
			if (s == ds)
			{
				delete s;
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	else if (ds != nullptr)
	{
		std::remove(m_sceneList.begin(), m_sceneList.end(), ds);
		m_releasedScene.push_back(ds);
	}
}

bool Engine::Update(unsigned long elapsedTime)
{
	if (m_currentScene)
	{
		m_currentScene->Update(elapsedTime);
	}

	//ensure we delete a scene is not in a loop.
	if (m_releasedScene.size() > 0)
	{
		for (auto& rs : m_releasedScene)
		{
			if (rs == m_currentScene)
			{
				m_currentScene = nullptr;
			}
			delete rs;
		}
		m_releasedScene.clear();
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