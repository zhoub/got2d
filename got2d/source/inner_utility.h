#pragma once
#include <memory>


#ifndef SR
#define SR(x) if(x){ x->Release(); x=nullptr;}
#endif

#ifndef SD
#define SD(x) if(x){ delete x; x=nullptr;}
#endif

#ifndef SDA
#define SDA(x) if(x){ delete[] x; x=nullptr;}
#endif

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

	inline auto_kill_ptr() : pointer(nullptr) { }

	inline auto_kill_ptr(Pointer* ptr) : pointer(ptr) {}

	inline auto_kill_ptr(const auto_kill_ptr&) = delete;

	inline auto_kill_ptr& operator=(Pointer* ptr)
	{
		if (ptr != pointer)
			release();
		pointer = ptr;
		return *this;
	}

	inline ~auto_kill_ptr() { release(); }

	inline void release()
	{
		if (pointer != nullptr)
		{
			Killer k;
			k(pointer);
			pointer = nullptr;
		}
	}

	inline operator Pointer* &() { return pointer; }
	inline Pointer* operator->() { return pointer; }
	inline const Pointer* operator->() const { return pointer; }
};

template<typename T> using ptr_autor = auto_kill_ptr<T, PointerReleaser<T>>;
template<typename T> using ptr_autod = auto_kill_ptr<T, PointerDeleter<T>>;


