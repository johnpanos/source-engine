//========= Portal 2 port ======================================================//
//
// Purpose: The tier0/mathlib/tier2 link surface ImpTiler.cpp needs, for the
//          headless blobulator suite (blobulator.tiler-mesh).
//
// The linux-headless-core profile links no engine library, so this unit
// defines the few symbols the tiler's object references:
//
//   Warning               counted, so the suite can check when endFrame() warns
//   pfVectorNormalize     the formula of mathlib's _VectorNormalize, which
//                         MathLib_Init selects on every Linux CPU
//   ThreadGetCurrentId,   a nonzero per-thread id and a spinning slow path for
//   CThreadFastMutex      the factory's mutex (only reached under contention)
//   MemAllocOOMError      the aligned allocator's failure report (the suite
//                         builds with NO_MALLOC_OVERRIDE: malloc, not g_pMemAlloc)
//   Generate*IndexBuffer  referenced by the CMeshBuilder draw path, which needs
//                         a render context the suite never provides; they abort
//
// None of these decide the tiler's mesh; the suite checks only CPU mesh output.
//
//=============================================================================//

#include "tier0/dbg.h"
#include "tier0/mem.h"
#include "tier0/threadtools.h"
#include "mathlib/vector.h"
#include "tier2/meshutils.h"

#include <atomic>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace
{
std::atomic<int> s_nWarnings( 0 );
std::atomic<unsigned int> s_nNextThreadId( 1 );

float FASTCALL FakeVectorNormalize( Vector &vec )
{
	float flRadius = sqrtf( vec.x * vec.x + vec.y * vec.y + vec.z * vec.z );
	float flInvRadius = 1.0f / ( flRadius + FLT_EPSILON );
	vec.x *= flInvRadius;
	vec.y *= flInvRadius;
	vec.z *= flInvRadius;
	return flRadius;
}

[[noreturn]] void DrawPathNotExercised( const char *pszName )
{
	std::fprintf( stderr, "blobulatortest: %s called; the draw path is not under test\n", pszName );
	std::abort();
}
} // namespace

// Number of Warning() calls so far.
int BlobTest_WarningCount()
{
	return s_nWarnings.load();
}

float( FASTCALL *pfVectorNormalize )( Vector &v ) = FakeVectorNormalize;

void Warning( const tchar *pMsg, ... )
{
	++s_nWarnings;
	va_list args;
	va_start( args, pMsg );
	std::printf( "warning: " );
	std::vprintf( pMsg, args );
	va_end( args );
}

ThreadId_t ThreadGetCurrentId()
{
	thread_local unsigned int s_nId = s_nNextThreadId++;
	return s_nId;
}

void CThreadFastMutex::Lock( const uint32 threadId, unsigned nSpinSleepTime ) volatile
{
	(void)nSpinSleepTime;
	while ( !TryLock( threadId ) )
		ThreadPause();
}

void MemAllocOOMError( size_t nSize )
{
	std::fprintf( stderr, "blobulatortest: out of memory allocating %zu bytes\n", nSize );
	std::abort();
}

void GenerateSequentialIndexBuffer( unsigned short *, int, int )
{
	DrawPathNotExercised( "GenerateSequentialIndexBuffer" );
}

void GenerateQuadIndexBuffer( unsigned short *, int, int )
{
	DrawPathNotExercised( "GenerateQuadIndexBuffer" );
}

void GeneratePolygonIndexBuffer( unsigned short *, int, int )
{
	DrawPathNotExercised( "GeneratePolygonIndexBuffer" );
}

void GenerateLineStripIndexBuffer( unsigned short *, int, int )
{
	DrawPathNotExercised( "GenerateLineStripIndexBuffer" );
}

void GenerateLineLoopIndexBuffer( unsigned short *, int, int )
{
	DrawPathNotExercised( "GenerateLineLoopIndexBuffer" );
}
