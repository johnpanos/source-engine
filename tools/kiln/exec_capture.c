/* exec_capture: an LD_PRELOAD shim for the RFC 0027 L1 launch-equivalence
 * check (tools/kiln/launch_equivalence.py). When a process exec()s a program
 * whose file name is listed in KILN_CAPTURE_NAMES (colon-separated), it writes
 * the working directory, argv and environment to KILN_CAPTURE_FILE and exits 0
 * instead of starting the program. Every other exec proceeds unchanged.
 *
 * Record format: "cwd\0<dir>\0argv\0<n>\0<arg>...\0env\0<n>\0<NAME=value>...\0".
 * Test tooling only; never linked into a product.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int listed( const char *path )
{
	const char *names = getenv( "KILN_CAPTURE_NAMES" );
	const char *base = strrchr( path, '/' );
	base = base ? base + 1 : path;
	if ( !names || !*names )
		return 0;
	size_t length = strlen( base );
	for ( const char *at = names; *at; )
	{
		const char *end = strchr( at, ':' );
		size_t size = end ? (size_t)( end - at ) : strlen( at );
		if ( size == length && strncmp( at, base, size ) == 0 )
			return 1;
		if ( !end )
			break;
		at = end + 1;
	}
	return 0;
}

static void put( FILE *out, const char *text )
{
	fwrite( text, 1, strlen( text ) + 1, out );
}

static void record( const char *path, char *const argv[], char *const envp[] )
{
	const char *file = getenv( "KILN_CAPTURE_FILE" );
	FILE *out = file ? fopen( file, "wb" ) : NULL;
	char cwd[PATH_MAX];
	char count[32];
	size_t n = 0;
	if ( !out )
		_exit( 97 );
	put( out, "cwd" );
	put( out, getcwd( cwd, sizeof( cwd ) ) ? cwd : "" );
	put( out, "path" );
	put( out, path );
	put( out, "argv" );
	while ( argv && argv[n] )
		++n;
	snprintf( count, sizeof( count ), "%zu", n );
	put( out, count );
	for ( size_t i = 0; i < n; ++i )
		put( out, argv[i] );
	n = 0;
	while ( envp && envp[n] )
		++n;
	put( out, "env" );
	snprintf( count, sizeof( count ), "%zu", n );
	put( out, count );
	for ( size_t i = 0; i < n; ++i )
		put( out, envp[i] );
	fclose( out );
	_exit( 0 );
}

int execve( const char *path, char *const argv[], char *const envp[] )
{
	static int ( *real )( const char *, char *const[], char *const[] );
	if ( listed( path ) )
		record( path, argv, envp );
	if ( !real )
		real =
		    (int ( * )( const char *, char *const[], char *const[] ))dlsym( RTLD_NEXT, "execve" );
	return real( path, argv, envp );
}
