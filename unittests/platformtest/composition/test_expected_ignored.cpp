//========= Copyright Valve Corporation, All rights reserved. ============//
#include "foundation/expected.h"
foundation::Expected<void, int> RequiredDecision()
{
	return {};
}
int main()
{
	RequiredDecision();
}
