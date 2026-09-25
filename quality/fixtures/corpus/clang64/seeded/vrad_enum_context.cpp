// Seeded negative control for corpus.clang64.seeded-truncation (RFC 0005 gap
// corpus). Reproduces vrad's enumeration-context defect from before ad7120f3
// (utils/vrad/vraddisps.cpp, CVRadDispMgr::ClipRayToDisp): the address of a
// stack EnumContext_t is cast to `int` and passed through an `intp` context
// parameter, so on LP64 the callee receives a pointer whose high 32 bits are
// gone. GCC accepts the cast under -fpermissive (as a warning that the
// products' -w hides); Clang rejects it. This file is never built into a
// product; the sweep must report its sites as new.
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef intptr_t intp; // as public/tier0/platform.h with PLATFORM_64BITS

struct Ray_t;
struct DispTested_t;

struct EnumContext_t
{
	Ray_t const *m_pRay;
	DispTested_t *m_pDispTested;
};

class IBSPTreeDataEnumerator
{
public:
	virtual bool EnumerateElement( int userId, intp context ) = 0;
};

class ISpatialQuery
{
public:
	virtual bool EnumerateLeavesAlongRay( Ray_t const &ray, IBSPTreeDataEnumerator *pEnum, intp context ) = 0;
};

bool ClipRayToDisp( ISpatialQuery *pTree, IBSPTreeDataEnumerator *pEnum, DispTested_t &dispTested, Ray_t const &ray )
{
	EnumContext_t ctx;
	ctx.m_pRay = &ray;
	ctx.m_pDispTested = &dispTested;
	return !pTree->EnumerateLeavesAlongRay( ray, pEnum, ( int )&ctx );
}

// The reverse direction: a 32-bit handle widened into a pointer.
void *HandleToPointer( int handle )
{
	return ( void * )handle;
}

// A printf width hazard: %d reads 32 of the 64 bits of a size_t.
void PrintCount( size_t count )
{
	printf( "%d elements\n", count );
}
