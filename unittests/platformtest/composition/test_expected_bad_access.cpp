//========= Copyright Valve Corporation, All rights reserved. ============//
#include "foundation/expected.h"
int main()
{
	foundation::Expected<int, int> result( foundation::MakeUnexpected( 1 ) );
	return result.Value();
}
