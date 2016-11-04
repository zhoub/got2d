#pragma once
#include <g2dconfig.h>

namespace g2d
{
	class RenderSystem;
	class Scene;
	class Texture;

	class G2DAPI Engine
	{
	public:
		struct Config
		{
			void* nativeWindow;
			const char* resourceFolderPath;
		};

		static bool Initialize(const Config& config);
		static void Uninitialize();
		static Engine* Instance();

		virtual ~Engine();
		virtual bool Update(unsigned long elapsedTime) = 0;
		virtual void Render() = 0;
		virtual RenderSystem* GetRenderSystem() = 0;
		virtual Scene* GetCurrentScene() = 0;
	};

	inline Engine* GetEngine() { return g2d::Engine::Instance(); }
}


