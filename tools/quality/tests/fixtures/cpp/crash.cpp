// Self-test fixture: a suite that terminates via a signal (SIGABRT). Proves the
// runner classifies a crash distinctly from an ordinary non-zero exit.
#include <cstdlib>

int main()
{
	std::abort();
	return 0;
}
