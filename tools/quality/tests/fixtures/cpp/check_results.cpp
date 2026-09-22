#include <cstdio>
#ifndef CHECKS
#define CHECKS 4
#endif
#ifndef FAILURES
#define FAILURES 0
#endif
int main()
{
	std::printf( "CONFORMANCE %d %d\n", CHECKS, FAILURES );
#ifdef DUPLICATE
	std::printf( "CONFORMANCE %d %d\n", CHECKS, FAILURES );
#endif
}
