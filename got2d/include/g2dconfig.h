#pragma once

#ifdef GOT2D_EXPORTS
#define G2DAPI __declspec(dllexport)
#else
#define G2DAPI __declspec(dllimport)
#endif

#ifndef SR
#define SR(x) if(x){ x->Release(); x=nullptr;}
#endif
