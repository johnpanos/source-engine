// Self-test fixture modelling a VACUOUS sensitivity suite: its oracle never
// actually exercises the broken provider, so it cannot detect a violation. It
// exits non-zero; the runner must report it as a mismatch (fail), proving that a
// sensitivity suite which fails to catch a bug does not pass the gate.
#include <cstdio>

int main()
{
	std::printf( "FAIL: sensitivity oracle did not detect the broken provider\n" );
	return 1;
}
