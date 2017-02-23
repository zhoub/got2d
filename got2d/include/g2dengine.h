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
			float defaultSceneBounding = 2 << 8;
		};

		static bool Initialize(const Config& config);
		static void Uninitialize();
		static Engine* Instance();

		DECL_CLASSID;
		virtual ~Engine();
		virtual bool Update(unsigned long elapsedTime) = 0;
		virtual void Render() = 0;
		virtual RenderSystem* GetRenderSystem() = 0;
		virtual Scene* CreateNewScene(float boundSize) = 0;
		virtual Scene* GetCurrentScene() = 0;
		//return lastActiveScene
		virtual Scene* SetActiveScene(Scene* activeScene) = 0;
		virtual void ReleaseScene(Scene* deletedScene) = 0;
	};

	inline Engine* GetEngine() { return g2d::Engine::Instance(); }
}


