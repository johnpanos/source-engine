// Nintendo 3DS compatibility definitions (see include/n3ds_compat.h and
// include/dlfcn.h). Linked once, in tier0.
#include <3ds.h>

#include "n3ds_compat.h"
#include "dlfcn.h"
#include "sys/mman.h"

#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

#define N3DS_EXPORT __attribute__(( visibility( "default" ) ))

static char g_dlError[] = "dynamic loading is not available on the 3DS (statically composed product)";

extern "C" {

N3DS_EXPORT void *dlopen( const char *, int ) { return nullptr; }
N3DS_EXPORT void *dlsym( void *, const char * ) { return nullptr; }
N3DS_EXPORT int dlclose( void * ) { return 0; }
N3DS_EXPORT char *dlerror( void ) { return g_dlError; }
N3DS_EXPORT int dladdr( const void *, Dl_info *info )
{
	if ( info )
		memset( info, 0, sizeof( *info ) );
	return 0;
}

N3DS_EXPORT void *mmap( void *, size_t, int, int, int, off_t )
{
	errno = ENOSYS;
	return MAP_FAILED;
}
N3DS_EXPORT int munmap( void *, size_t ) { return 0; }
N3DS_EXPORT int mprotect( void *, size_t, int ) { return 0; }
N3DS_EXPORT int madvise( void *, size_t, int ) { return 0; }
N3DS_EXPORT int shm_open( const char *, int, mode_t )
{
	errno = ENOSYS;
	return -1;
}
N3DS_EXPORT int shm_unlink( const char * ) { return 0; }

// Thread-specific data for pthread_key_create and friends. devkitARM's
// libsysbase forwards them to these hooks, which libctru (2.7) does not
// provide, so every key creation failed with ENOSYS. Keys index a fixed
// table; each thread's values live in compiler TLS (libctru gives every
// thread it creates a TLS block). Destructors are recorded but not run at
// thread exit.
namespace
{
constexpr unsigned kMaxKeys = 512;
bool g_KeyUsed[kMaxKeys];
void ( *g_KeyDestructor[kMaxKeys] )( void * );
LightLock g_KeyLock = 1; // unlocked (LightLock_Init's value)
thread_local void *t_KeyValues[kMaxKeys];
}

extern "C" {
N3DS_EXPORT int __syscall_tls_create( pthread_key_t *key, void ( *destructor )( void * ) )
{
	LightLock_Lock( &g_KeyLock );
	for ( unsigned i = 0; i < kMaxKeys; ++i )
	{
		if ( !g_KeyUsed[i] )
		{
			g_KeyUsed[i] = true;
			g_KeyDestructor[i] = destructor;
			LightLock_Unlock( &g_KeyLock );
			*key = pthread_key_t( i );
			return 0;
		}
	}
	LightLock_Unlock( &g_KeyLock );
	return EAGAIN;
}

N3DS_EXPORT int __syscall_tls_set( pthread_key_t key, const void *value )
{
	if ( key >= kMaxKeys || !g_KeyUsed[key] )
		return EINVAL;
	t_KeyValues[key] = const_cast<void *>( value );
	return 0;
}

N3DS_EXPORT void *__syscall_tls_get( pthread_key_t key )
{
	return key < kMaxKeys ? t_KeyValues[key] : nullptr;
}

N3DS_EXPORT int __syscall_tls_delete( pthread_key_t key )
{
	if ( key >= kMaxKeys || !g_KeyUsed[key] )
		return EINVAL;
	LightLock_Lock( &g_KeyLock );
	g_KeyUsed[key] = false;
	g_KeyDestructor[key] = nullptr;
	LightLock_Unlock( &g_KeyLock );
	return 0;
}
}

N3DS_EXPORT int pthread_setname_np( pthread_t, const char * ) { return 0; }

// One user, no processes, no signals.
N3DS_EXPORT uid_t getuid( void ) { return 0; }
N3DS_EXPORT struct passwd *getpwuid( uid_t ) { return nullptr; }
N3DS_EXPORT unsigned alarm( unsigned ) { return 0; }
N3DS_EXPORT FILE *popen( const char *, const char * )
{
	errno = ENOSYS;
	return nullptr;
}
N3DS_EXPORT int pclose( FILE * ) { return -1; }
N3DS_EXPORT int execlp( const char *, const char *, ... )
{
	errno = ENOSYS;
	return -1;
}
N3DS_EXPORT int getrusage( int, struct rusage *usage )
{
	if ( usage )
		memset( usage, 0, sizeof( *usage ) );
	return 0;
}
N3DS_EXPORT int futimes( int, const struct timeval * ) { return 0; }

// Days from the civil date (Howard Hinnant's algorithm), so timegm is exact
// without touching the process time zone.
N3DS_EXPORT time_t timegm( struct tm *tm )
{
	long long year = tm->tm_year + 1900LL;
	long long month = tm->tm_mon + 1;
	year -= month <= 2;
	const long long era = ( year >= 0 ? year : year - 399 ) / 400;
	const long long yoe = year - era * 400;
	const long long doy = ( 153 * ( month + ( month > 2 ? -3 : 9 ) ) + 2 ) / 5 + tm->tm_mday - 1;
	const long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	const long long days = era * 146097 + doe - 719468;
	return (time_t)( days * 86400 + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec );
}

N3DS_EXPORT int getloadavg( double loadavg[], int nelem )
{
	for ( int i = 0; i < nelem; ++i )
		loadavg[i] = 0.0;
	return nelem;
}

N3DS_EXPORT int pthread_getschedparam( pthread_t, int *policy, struct sched_param *param )
{
	if ( policy )
		*policy = SCHED_OTHER;
	if ( param )
		memset( param, 0, sizeof( *param ) );
	return 0;
}

N3DS_EXPORT int pthread_setschedparam( pthread_t, int, const struct sched_param * )
{
	return 0;
}

// No signals on the 3DS: only the liveness probe (signal 0) succeeds.
N3DS_EXPORT int pthread_kill( pthread_t, int sig )
{
	return sig == 0 ? 0 : ENOSYS;
}

}

int n3ds_pthread_create( pthread_t *thread, void *( *entry )( void * ), void *arg, size_t stack )
{
	static bool s_checked = false, s_new3ds = false;
	if ( !s_checked )
	{
		APT_CheckNew3DS( &s_new3ds );
		s_checked = true;
	}
	static const int kNew3dsCores[] = { 2, 1 };
	static const int kOld3dsCores[] = { 1 };
	static unsigned s_next = 0;
	const int *cores = s_new3ds ? kNew3dsCores : kOld3dsCores;
	const unsigned count = s_new3ds ? 2 : 1;
	const int core = cores[__atomic_fetch_add( &s_next, 1, __ATOMIC_RELAXED ) % count];
	s32 priority = 0x30;
	svcGetThreadPriority( &priority, CUR_THREAD_HANDLE );
	const size_t bytes = stack ? stack : 64 * 1024;
	// ThreadFunc returns void; the entry's return value is not used.
	Thread created = threadCreate( (ThreadFunc)entry, arg, bytes, priority, core, false );
	if ( !created ) // the core refused (exheader affinity, time limit): any core
		created = threadCreate( (ThreadFunc)entry, arg, bytes, priority, -2, false );
	if ( !created )
		return EAGAIN;
	*thread = (pthread_t)created;
	return 0;
}
