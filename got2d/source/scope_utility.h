#pragma once
#include <functional>
#include <memory>

template <typename FUNC>
class scope_fallback
{
	bool canceled = false;
	FUNC callback;
public:
	scope_fallback(FUNC&& f) : callback(std::move(f)) { }

	scope_fallback(scope_fallback&& other)
		: callback(other.callback)
		, canceled(other.canceled)
	{
		other.canceled = true;
	}

	~scope_fallback()
	{
		if (!canceled) callback();
	}

	void cancel() { canceled = true; }
private:
	scope_fallback& operator=(const scope_fallback&) = delete;
	scope_fallback& operator=(const scope_fallback&&) = delete;
	scope_fallback(const scope_fallback&) = delete;
};

template<typename FUNC> 
scope_fallback<FUNC> create_fallback(FUNC&& f)
{
	return scope_fallback<FUNC>(std::move(f));
}

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

template<typename T> using autor = auto_kill_ptr<T, PointerReleaser<T>>;
template<typename T> using autod = auto_kill_ptr<T, PointerDeleter<T>>;
