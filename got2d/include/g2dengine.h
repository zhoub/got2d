#pragma once

#include <g2dconfig.h>

namespace g2d
{
	class G2DAPI Engine
	{
	public:
		virtual ~Engine();

		virtual bool Update(unsigned long elapsedTime) = 0;

		virtual void Release() = 0;
	};

	struct G2DAPI EngineConfig
	{
		void* nativeWindow;
	};
	extern "C" G2DAPI Engine* CreateEngine(const EngineConfig& config);
}


