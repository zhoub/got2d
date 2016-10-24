#pragma once
#include <g2dconfig.h>

namespace g2d
{
	class RenderSystem;
	class Scene;
	class G2DAPI Engine
	{
	public:
		virtual ~Engine();
		virtual bool Update(unsigned long elapsedTime) = 0;
		virtual void Render() = 0;
		virtual RenderSystem* GetRenderSystem() = 0;
		virtual Scene* GetCurrentScene() = 0;
	};

	struct G2DAPI EngineConfig
	{
		void* nativeWindow;
	};

	extern "C" G2DAPI bool InitEngine(const EngineConfig& config);
	extern "C" G2DAPI void UninitEngine();
	extern "C" G2DAPI Engine* GetEngine();
}


