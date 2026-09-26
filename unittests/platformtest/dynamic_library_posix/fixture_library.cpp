//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Real shared-library fixture for the native loader conformance suite
//          (RFC 0001 reference migration step 1): a small module exporting a
//          known symbol.
//
//=============================================================================//

extern "C" __attribute__( ( visibility( "default" ) ) ) int PlatformFixtureAnswer()
{
	return 42;
}
