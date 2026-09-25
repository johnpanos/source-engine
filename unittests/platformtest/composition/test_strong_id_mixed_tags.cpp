//========= Copyright Valve Corporation, All rights reserved. ============//
// Sensitivity: identifiers of different tags must not compare.
#include "foundation/strong_id.h"
struct AlphaTag;
struct BetaTag;
int main()
{
	return foundation::StrongId<AlphaTag>{ 1 } == foundation::StrongId<BetaTag>{ 1 } ? 0 : 1;
}
