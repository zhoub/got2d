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

struct CID { static unsigned Next() { static unsigned i = 0x1000; return i++; } };

#define DECL_CLASSID virtual unsigned GetClassID() const = 0;
#define IMPL_CLASSID public:\
	inline unsigned GetClassID() const { static unsigned ClassID = CID::Next(); return ClassID; }