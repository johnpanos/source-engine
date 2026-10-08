// Nintendo 3DS compatibility declarations, force-included in every 3DS
// translation unit (wscript, DEST_OS 3ds). newlib + libctru lack these POSIX
// and BSD functions; platform/n3ds/compat/n3ds_compat.cpp defines them.
#ifndef N3DS_COMPAT_H
#define N3DS_COMPAT_H

#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h> // ntohs/htonl; on glibc <netinet/in.h> declares them
#include <setjmp.h>
#include <stddef.h>

// newlib declares sigsetjmp only for Cygwin and RTEMS; no signal masks here.
#ifndef sigsetjmp
typedef jmp_buf sigjmp_buf;
#define sigsetjmp( env, savemask ) setjmp( env )
#define siglongjmp( env, value ) longjmp( env, value )
#endif

// libctru's <sys/ioctl.h> has FIONBIO only; FIONREAD reports no bytes.
#ifndef SO_KEEPALIVE
#define SO_KEEPALIVE 0x0008 // accepted and ignored by libctru's setsockopt
#endif
#ifndef FIONREAD
#define FIONREAD 0x541B
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sched_param;
size_t strnlen( const char *s, size_t maxlen );
time_t timegm( struct tm *tm );
int getloadavg( double loadavg[], int nelem );
int pthread_getschedparam( pthread_t thread, int *policy, struct sched_param *param );
int pthread_setschedparam( pthread_t thread, int policy, const struct sched_param *param );
int pthread_kill( pthread_t thread, int sig );
int pthread_setname_np( pthread_t thread, const char *name );
// The engine's threads (tier0 CreateSimpleThread and CThread::Start). libctru's
// pthread_create puts every thread on core 0 at priority 0x3F, below the main
// thread, so workers only run when the main thread blocks. This creates the
// thread on the next worker core (the New 3DS's core 2, then the system
// core 1, alternating) at the caller's priority. A libctru pthread_t is the
// Thread threadCreate returns, so join and the rest work unchanged.
int n3ds_pthread_create( pthread_t *thread, void *( *entry )( void * ), void *arg, size_t stack );
struct passwd;
uid_t getuid( void );
struct passwd *getpwuid( uid_t uid );
unsigned alarm( unsigned seconds );

#ifdef __cplusplus
}
#endif

#endif
