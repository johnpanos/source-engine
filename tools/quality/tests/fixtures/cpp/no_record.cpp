// Self-test fixture: a suite that prints progress and exits 0 WITHOUT its
// checks-v1 record, modelling an early return before the checks ran. The
// runner must classify the run as incomplete (fail), not as a pass.
#include <cstdio>

int main()
{
	std::printf( "self-test: started, returning before any check\n" );
	return 0;
}
