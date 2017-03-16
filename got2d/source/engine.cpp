#include "engine.h"
#include "inner_utility.h"

#include <algorithm>

uint32_t G2DAPI NextClassID()
{
	static uint32_t s_nextID = 0;
	return ++s_nextID;
}

Engine* Engine::Instance = nullptr;

bool g2d::Engine::IsInitialized()
{
	return ::Engine::Instance != nullptr;
}

bool g2d::Engine::Initialize(const Config& config)
{
	ENSURE(!IsInitialized());

	auto& instance = ::Engine::Instance;
	instance = new ::Engine();
	if (instance == nullptr)
		return false;

	do
	{
		instance->SetResourceRoot(config.resourceFolderPath);

		if (!instance->CreateRenderSystem(config.nativeWindow))
		{
			break;
		}
		return true;
	} while (false);

	delete instance;
	instance = nullptr;
	return false;
}

void g2d::Engine::Uninitialize()
{
	ENSURE(IsInitialized());
	delete ::Engine::Instance;
	::Engine::Instance = nullptr;
}

g2d::Engine* g2d::Engine::Instance()
{
	ENSURE(IsInitialized());
	return GetEngineImpl();
}

Engine::~Engine()
{
	m_renderSystem.Destroy();
}

g2d::Scene* Engine::CreateNewScene(float boundSize)
{
	return new Scene(boundSize);
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

void Engine::SetResourceRoot(const std::string& resPath)
{
	if (!resPath.empty())
	{
		m_resourceRoot = resPath;
		std::replace(std::begin(m_resourceRoot), std::end(m_resourceRoot), '/', '\\');
		if (m_resourceRoot.back() != '\\')
		{
			m_resourceRoot.push_back('\\');
		}
	}
}