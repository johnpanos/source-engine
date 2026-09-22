// Self-test fixture: a suite that never terminates. Proves the runner bounds
// suite duration and fails on timeout while retaining diagnostics. The volatile
// side effect keeps the loop from being optimized away.
int main()
{
	volatile int x = 0;
	while ( true )
	{
		x = x + 1;
	}
	return x;
}
