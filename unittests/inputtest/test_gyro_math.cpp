//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance of the gyro policy (inputsystem/gyro_math.h): sample
//          period from the display refresh rate, display rotation, the
//          device-to-screen axis remap and sample integration.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/inputtest/gyro_math_checks.h"

int main()
{
	gyrotest::Tally tally;
	gyrotest::CheckAll( gyrotest::RealPolicy(), tally );
	return testing::ReportConformance( tally.checks, tally.failures );
}
