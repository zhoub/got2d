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
template<typename T> using r_ptr = std::unique_ptr<T, PointerReleaser<T>>;
