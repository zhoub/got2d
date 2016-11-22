#pragma once

#ifdef GOT2D_EXPORTS
#define G2DAPI __declspec(dllexport)
#else
#define G2DAPI __declspec(dllimport)
#endif

unsigned G2DAPI NextClassID();
#define DECL_CLASSID virtual unsigned GetClassID() const = 0;
#define IMPL_CLASSID public:\
	inline unsigned GetClassID() const { static unsigned ClassID = NextClassID(); return ClassID; }