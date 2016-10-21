#include "engine.h"


g2d::Engine::~Engine() { }

g2d::Engine* g2d::CreateEngine()
{
	return new ::Engine();
}

bool Engine::Update(unsigned long elapsedTime)
{
	return true;
}

void Engine::Release()
{
	delete this;
}