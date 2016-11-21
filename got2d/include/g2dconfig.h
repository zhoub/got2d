#pragma once

#ifdef GOT2D_EXPORTS
#define G2DAPI __declspec(dllexport)
#else
#define G2DAPI __declspec(dllimport)
#endif

#ifndef SR
#define SR(x) if(x){ x->Release(); x=nullptr;}
#endif

#ifndef SD
#define SD(x) if(x){ delete x; x=nullptr;}
#endif

#ifndef SDA
#define SDA(x) if(x){ delete[] x; x=nullptr;}
#endif

unsigned G2DAPI NextClassID();
#define DECL_CLASSID virtual unsigned GetClassID() const = 0;
#define IMPL_CLASSID public:\
	inline unsigned GetClassID() const { static unsigned ClassID = NextClassID(); return ClassID; }