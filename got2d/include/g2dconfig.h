#pragma once
#include <cinttypes>

#ifdef GOT2D_EXPORTS
#define G2DAPI __declspec(dllexport)
#else
#define G2DAPI __declspec(dllimport)
#endif

class G2DAPI GObject
{
public:
	virtual ~GObject() { }

	virtual uint32_t GetClassID() const = 0;

	bool IsSameType(GObject* other) const
	{
		return GetClassID() == other->GetClassID();
	}

protected:
	GObject() = default;

	GObject(const GObject&) = delete;

	GObject& operator=(const GObject&) = delete;
};

uint32_t G2DAPI NextClassID();
#define RTTI_IMPL \
public:\
	virtual uint32_t GetClassID() const override \
	{ return GetStaticClassID(); }\
	static uint32_t GetStaticClassID()\
	{ static uint32_t s_ClassID = NextClassID(); return s_ClassID; }\
private:

template<typename T> bool Is(GObject* gobj) { return gobj->GetClassID() == T::GetStaticClassID(); }