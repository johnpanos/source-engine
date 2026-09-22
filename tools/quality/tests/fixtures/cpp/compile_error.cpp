// Self-test fixture: a suite that does not compile. Proves the runner classifies
// a build failure distinctly and never reports an unbuilt suite as passing.
int main()
{
	this is not valid c++;
}
