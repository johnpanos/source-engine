//========= Copyright Valve Corporation, All rights reserved. ============//
// Sensitivity: an identifier must not be created implicitly from its integer.
#include "foundation/strong_id.h"
struct AlphaTag;
foundation::StrongId<AlphaTag> Make()
{
	unsigned int raw = 7;
	return raw;
}
int main()
{
	return Make().IsValid() ? 0 : 1;
}
