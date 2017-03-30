#include <algorithm>
#include "engine.h"

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

	auto fb = create_fallback([&]
	{
		delete instance;
		instance = nullptr;
	});


	instance->SetResourceRoot(config.resourceFolderPath);
	if (!instance->CreateRenderSystem(config.nativeWindow))
	{
		return false;
	}

	fb.cancel();
	return true;
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

void Engine::RemoveScene(::Scene& scene)
{
	auto oldEnd = std::end(m_scenes);
	auto newEnd = std::remove(std::begin(m_scenes), oldEnd, &scene);
	m_scenes.erase(newEnd, oldEnd);
}

g2d::Scene* Engine::CreateNewScene(float boundSize)
{
	auto scene = new Scene(boundSize);
	m_scenes.push_back(scene);
	return scene;
}

void Engine::Update(uint32_t deltaTime)
{
	m_elapsedTime += deltaTime;

	GetKeyboard().Update(m_elapsedTime);
	GetMouse().Update(m_elapsedTime);

	for (auto& scene : m_scenes)
	{
		scene->Update(m_elapsedTime, deltaTime);
	}
}

void Engine::OnMessage(const g2d::Message& message)
{
	GetKeyboard().OnMessage(message, m_elapsedTime);
	GetMouse().OnMessage(message, m_elapsedTime);

	for (auto& scene : m_scenes)
	{
		scene->OnMessage(message, m_elapsedTime);
	}
}

bool Engine::OnResize(uint32_t width, uint32_t height)
{
	if (m_renderSystem.OnResize(width, height))
	{
		for (auto& scene : m_scenes)
		{
			scene->OnResize();
		}
		return true;
	}
	else
	{
		return false;
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
