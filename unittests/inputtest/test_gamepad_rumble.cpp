//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance of the gamepad rumble policy
//          (inputsystem/gamepad_rumble.h): motor levels, and the lease's
//          latency, motor independence, rate, tracking, renewal, stall and
//          reset behavior on a model two-motor gamepad.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/inputtest/gamepad_rumble_checks.h"

int main()
{
	gamepadrumbletest::Tally tally;
	gamepadrumbletest::CheckAll( gamepadrumbletest::RealPolicy(), tally );
	return testing::ReportConformance( tally.checks, tally.failures );
}
