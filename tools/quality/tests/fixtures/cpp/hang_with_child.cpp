// Self-test fixture: forks a child, records the child's pid in the file named
// by CHILD_PID_FILE, and hangs. On timeout the runner must kill the whole
// process group, so the child does not outlive the suite.
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	const pid_t child = fork();
	if ( child == 0 )
	{
		for ( ;; )
			pause();
	}
	const char *path = std::getenv( "CHILD_PID_FILE" );
	if ( path )
	{
		if ( FILE *file = std::fopen( path, "w" ) )
		{
			std::fprintf( file, "%d\n", static_cast<int>( child ) );
			std::fclose( file );
		}
	}
	std::printf( "self-test: parent waiting forever\n" );
	std::fflush( stdout );
	for ( ;; )
		pause();
}
