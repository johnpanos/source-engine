// Self-test fixture: a suite that builds but fails a check (exit 1). Proves the
// runner detects a failing conformance suite instead of reporting it green.
#include <cstdio>

int main()
{
	std::printf( "FAIL failing.cpp:1: deliberate self-test failure\n" );
	return 1;
}
