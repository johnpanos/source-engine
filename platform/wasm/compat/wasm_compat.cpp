//========= Copyright Valve Corporation, All rights reserved. ============//
//
// WebAssembly (Emscripten, RFC 0029) compatibility definitions: the POSIX
// functions the engine calls that Emscripten's libc does not provide. Linked
// once, in tier0. Exported, so every module object of the static
// composition (scripts/waifulib/static_composition.py) resolves to this copy.
//
//=============================================================================//

#include <emscripten/threading.h>

#include <fcntl.h>
#include <string.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>

#define WASM_EXPORT __attribute__(( visibility( "default" ) ))

extern "C" {

// Names the calling thread in the browser's tools; other threads are not named.
WASM_EXPORT int pthread_setname_np( pthread_t thread, const char *name )
{
	if ( thread == pthread_self() )
		emscripten_set_thread_name( thread, name );
	return 0;
}

// No process accounting in a browser: zero usage (the engine reports it).
WASM_EXPORT int getrusage( int, struct rusage *usage )
{
	if ( usage )
		memset( usage, 0, sizeof( *usage ) );
	return 0;
}

WASM_EXPORT int futimes( int fd, const struct timeval tv[2] )
{
	if ( !tv )
		return futimens( fd, nullptr );
	struct timespec times[2];
	for ( int i = 0; i < 2; ++i )
	{
		times[i].tv_sec = tv[i].tv_sec;
		times[i].tv_nsec = tv[i].tv_usec * 1000;
	}
	return futimens( fd, times );
}

}
