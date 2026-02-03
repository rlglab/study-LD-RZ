#ifndef __TYPES_H__
#define __TYPES_H__

#if defined(_WIN64) || defined(__amd64__)
#ifndef SYSTEM64
#define SYSTEM64
#endif
#else
#ifndef SYSTEM32
#define SYSTEM32
#endif
#endif

typedef unsigned int uint ;
typedef unsigned long long ull ;
typedef unsigned char uchar ;

#include <cassert>

/************************************************************************/
/* limitation of this framework                                         */
/************************************************************************/
const int MAX_NUM_THREADS = 128;
const int MAX_TREE_DEPTH = 800;
const int MAX_NUM_CHILDREN = 362 ;
const int MAX_NUM_WORKERS = 512;

/************************************************************************/
/* special implementation                                               */
/************************************************************************/
#include "Random.h"

/************************************************************************/
/* debugging options                                                    */
/************************************************************************/

#define NATIVE_PTR_PATH 0
#define DUMP_TREE_AFTER_REUSE 0

/************************************************************************/
/* debugging options                                                    */
/************************************************************************/
#if defined(unix) && defined(NDEBUG)
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wreorder"
#pragma clang diagnostic ignored "-Winvalid-source-encoding"
#pragma clang diagnostic ignored "-Wparentheses"
#endif

#endif // __TYPES_H__ 
