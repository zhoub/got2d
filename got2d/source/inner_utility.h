#pragma once
#include <memory>

#define SR(x)  if(x) { x->Release(); x=nullptr; }
#define SD(x)  if(x) { delete x; x=nullptr; }
#define SDA(x) if(x) { delete[] x; x=nullptr; }

#define RTTI_INNER_IMPL \
public:\
	uint32_t GetClassID() const\
	{\
		static uint32_t ClassID = NextClassID();\
		return ClassID;\
	}\
private:

template<typename T> struct PointerReleaser { void operator()(T* pointer) { pointer->Release(); } };
template<typename T> struct PointerDeleter { void operator()(T* pointer) { delete pointer; } };
template<typename T> struct ArrayDeleter { void operator()(T* pointer) { delete[] pointer; } };
template<typename T> using uptr_d = std::unique_ptr<T>;
template<typename T> using uptr_r = std::unique_ptr<T, PointerReleaser<T>>;
template<typename T> using uptr_a = std::unique_ptr<T, ArrayDeleter<T>>;

template<typename Pointer, typename Killer>
struct auto_kill_ptr
{
	Pointer* pointer;

	auto_kill_ptr() : pointer(nullptr) { }

	auto_kill_ptr(Pointer* ptr) : pointer(ptr) {}

	auto_kill_ptr(const auto_kill_ptr&) = delete;

	auto_kill_ptr& operator=(Pointer* ptr)
	{
		if (ptr != pointer)
			release();
		pointer = ptr;
		return *this;
	}

	~auto_kill_ptr() { release(); }

	void release()
	{
		if (pointer != nullptr)
		{
			Killer k;
			k(pointer);
			pointer = nullptr;
		}
	}

	operator Pointer* &() { return pointer; }
	Pointer* operator->() { return pointer; }
	const Pointer* operator->() const { return pointer; }
};

template<typename T> using ptr_autor = auto_kill_ptr<T, PointerReleaser<T>>;
template<typename T> using ptr_autod = auto_kill_ptr<T, PointerDeleter<T>>;

#include <cassert>
#define ENSURE(b) { assert(b); if (b); else throw; }

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

	class Entity;
	class Camera;
	class Quad;
}