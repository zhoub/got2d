#pragma once
#include <memory>

#define RTTI_INNER_IMPL \
public:\
	uint32_t GetClassID() const\
	{\
		static uint32_t ClassID = NextClassID();\
		return ClassID;\
	}\
private:

template<typename T1, typename T2>
bool same_type(T1* a, T2* b)
{
	return (a->GetClassID() == b->GetClassID());
}

namespace g2d
{
	class Engine;
	class RenderSystem;

	class SceneNode;
	class Scene;

	class Component;
	class Camera;
	class Quad;
}
