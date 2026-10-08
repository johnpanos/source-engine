//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A mod-style module for R103 (RFC 0001 "Tier 0 facade over the
//			foundation providers"). It is compiled once, against the Tier 0
//			headers of its recorded revision, with SDK-2013-style flags
//			(C++11, the old libstdc++ ABI), and the binary is kept. Later Tier 0
//			builds must load it and pass ModFixture_Run unchanged.
//
//			Every cohort's exports are called: time, threads, the command line
//			and debugger state, spew, minidump user streams and the allocator.
//
//=============================================================================//

#include "tier0/platform.h"
#include "tier0/dbg.h"
#include "tier0/threadtools.h"
#include "tier0/icommandline.h"
#include "tier0/minidump.h"
#include "tier0/memalloc.h"
#include "tier0/systeminformation.h"

#include <stdio.h>
#include <string.h>

namespace
{

char g_captured[512];
SpewType_t g_capturedType = SPEW_TYPE_COUNT;

SpewRetval_t Capture( SpewType_t type, const tchar *message )
{
	g_capturedType = type;
	snprintf( g_captured, sizeof( g_captured ), "%s", message );
	return SPEW_CONTINUE;
}

struct Report
{
	char *text;
	int size;
	int length;
	int checks;
	int failures;

	void Check( bool ok, const char *what )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( length < size )
			{
				length += snprintf( text + length, size - length, "FAIL %s\n", what );
			}
		}
	}
};

} // namespace

// Writes "FAIL ..." lines into `report` and returns checks * 1000 + failures.
#if defined( _WIN32 )
#define MOD_FIXTURE_EXPORT extern "C" __declspec( dllexport )
#else
#define MOD_FIXTURE_EXPORT extern "C" __attribute__( ( visibility( "default" ) ) )
#endif
MOD_FIXTURE_EXPORT int ModFixture_Run( char *report, int size )
{
	Report r = { report, size, 0, 0, 0 };
	if ( size > 0 )
	{
		report[0] = 0;
	}

	// Time: near zero at load, monotonic, units agree.
	const double t0 = Plat_FloatTime();
	r.Check( t0 >= 0.0 && t0 < 3600.0, "Plat_FloatTime starts near zero" );
	const uint64 us0 = Plat_USTime();
	const uint32 ms0 = Plat_MSTime();
	ThreadSleep( 20 ); // inline in the mod; the delay is measured by Tier 0
	const double t1 = Plat_FloatTime();
	const uint64 us1 = Plat_USTime();
	const uint32 ms1 = Plat_MSTime();
	r.Check( t1 - t0 >= 0.019, "Plat_FloatTime measures a 20 ms sleep" );
	r.Check( t1 - t0 < 2.0, "Plat_FloatTime does not run away" );
	r.Check( us1 - us0 >= 19000, "Plat_USTime measures the sleep" );
	r.Check( ms1 - ms0 >= 19, "Plat_MSTime measures the sleep" );

	// Threads.
	const ThreadId_t id = ThreadGetCurrentId();
	r.Check( id != 0, "ThreadGetCurrentId is nonzero" );
	r.Check( ThreadGetCurrentId() == id, "ThreadGetCurrentId is stable" );
	ThreadSetDebugName( "modfixture" );
	r.Check( ThreadInMainThread(), "the fixture runs on the main thread" );

	// Command line and debugger state (the host passes -modfixture 42).
	ICommandLine *cl = CommandLine_Tier0();
	r.Check( cl != NULL, "CommandLine_Tier0" );
	if ( cl != NULL )
	{
		r.Check( cl->FindParm( "-modfixture" ) != 0, "FindParm" );
		r.Check( cl->ParmValue( "-modfixture", 0 ) == 42, "ParmValue" );
		r.Check( cl->FindParm( "-absent" ) == 0, "FindParm of an absent parameter" );
	}
	const char *line = Plat_GetCommandLineA();
	r.Check( line != NULL && strstr( line, "-modfixture" ) != NULL, "Plat_GetCommandLineA" );
	r.Check( !Plat_IsInDebugSession(), "Plat_IsInDebugSession" );
	Plat_DebugString( "modfixture debug string\n" );

	// Spew: the hook sees the formatted message and its type.
	const SpewOutputFunc_t previous = GetSpewOutputFunc();
	SpewOutputFunc( Capture );
	Msg( "mod %d says hi\n", 7 );
	r.Check( g_capturedType == SPEW_MESSAGE && strcmp( g_captured, "mod 7 says hi\n" ) == 0, "Msg" );
	Warning( "mod warns %s\n", "loudly" );
	r.Check( g_capturedType == SPEW_WARNING && strcmp( g_captured, "mod warns loudly\n" ) == 0, "Warning" );
	DevMsg( 0, "dev %d\n", 1 );
	SpewOutputFunc( previous );

	// Minidump user streams round-trip.
	MinidumpUserStreamInfoSetHeader( "modfixture header %d", 3 );
	MinidumpUserStreamInfoAppend( "modfixture entry %d", 4 );
	const char *header = MinidumpUserStreamInfoGet( 0 );
	r.Check( header != NULL && strstr( header, "modfixture header 3" ) != NULL, "minidump header" );

	// Memory.
	r.Check( Plat_GetMemPageSize() > 0, "Plat_GetMemPageSize" );
	void *block = g_pMemAlloc->Alloc( 4096 );
	r.Check( block != NULL, "g_pMemAlloc->Alloc" );
	if ( block != NULL )
	{
		memset( block, 0x5a, 4096 );
		g_pMemAlloc->Free( block );
	}

	return r.checks * 1000 + r.failures;
}
