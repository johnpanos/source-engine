/*========= Copyright Valve Corporation, All rights reserved. ============
 *
 * Purpose: Box3D private C17 dialect probe (RFC 0004/0006 M0). The pinned
 *          library must compile as C17 without the engine's fast-math
 *          relaxations; a finite-math-only build folds the NaN check below.
 *
 *=======================================================================*/

#include <stdio.h>

#if !defined( __STDC_VERSION__ ) || __STDC_VERSION__ != 201710L
#error "box3d-c17 probe: __STDC_VERSION__ is not 201710L"
#endif
#if defined( __FAST_MATH__ )
#error "box3d-c17 probe: fast-math is enabled"
#endif

int main( void )
{
	volatile double zero = 0.0;
	double nan = zero / zero;
	if ( nan == nan )
		return 1;
	printf( "box3d-c17-probe ok __STDC_VERSION__=%ld\n", (long)__STDC_VERSION__ );
	return 0;
}
