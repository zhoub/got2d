#pragma once
#include "g2dconfig.h"
#include "g2dmessage.h"

namespace g2d
{
	class RenderSystem;
	class Scene;
	class Texture;

	// Got2D starts from here, this is the main entrance of the entire engine.
	class G2DAPI Engine : public GObject
	{
	public:
		// Fill this sturct to initialize the engine.
		struct Config
		{
			// Native rendering window handle: HWND, EGLWindowType, etc.
			void* nativeWindow;

			// Engine will prefix this path to all relative resource-loading paths
			// using in the engine, turning them to absolute paths.
			const char* resourceFolderPath;
		};

		// CAUSTION, this must be the first Engine function
		// called by user, it initialize the engine, user can 
		// only issues any other engine functions after 
		// successfully calling this function.
		// It can be called when Engine has not be initialized,
		// otherwise engine will raise an exception.
		static bool Initialize(const Config& config);

		// CAUSTION, this must be the last Engine function
		// called by user, it release all resources and
		// destroy the engine. call it before exit.
		// It can be called only engine have been initialized,
		// otherwise engine will raise an exception.
		static void Uninitialize();

		// Determin whether the engine is initialized.
		static bool IsInitialized();

		// The unique instance maintained by the engine,
		// using it after initialized the engine.
		static Engine* Instance();

		virtual RenderSystem* GetRenderSystem() = 0;

		// Create a Scene instance.
		// Engine require a maxinum bounding size to enable quadtree
		// culling technique, dynamic entities or those whom ouside
		// the boundry will be checked each frame when processing 
		// visibility tesing before rendering.
		virtual Scene* CreateNewScene(float boundSize) = 0;

		// Call it each frame manually, to update engine staffs, including 
		// updating entire scene tree, adjusting special graph, 
		// updating spectial effects, and so on.
		// deltaTime is elapsed time between 2 frames.
		virtual void Update(uint32_t deltaTime) = 0;

		// Call it manually when message arrived, to dispatching events 
		// to entire scene tree.
		// Translate system message by using function set TranslateMessage* .
		virtual void OnMessage(const Message& message) = 0;

		// Call it manually when size of native window changes, to 
		// update projection matrix and rendering states in render system.
		virtual bool OnResize(uint32_t width, uint32_t height) = 0;
	};

	inline bool IsEngineInitialized()
	{
		return g2d::Engine::IsInitialized();
	}

	inline Engine* GetEngine()
	{
		return g2d::Engine::Instance();
	}
}


