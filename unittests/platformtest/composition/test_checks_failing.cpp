//========= Copyright Valve Corporation, All rights reserved. ============//
// Sensitivity: a suite whose matcher check fails must fail through the runner.
#include "testing/checks.h"
int main()
{
	testing::Checks checks;
	checks.That( true, "one passing check" );
	checks.Equal( 1, 2, "one failing check" );
	return checks.Report();
}
