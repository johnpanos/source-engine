//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance of the device-vibrator rumble policy
//          (inputsystem/vibrator_policy.h): two-motor mixing, amplitude
//          quantization, and the command stream's latency, rate, tracking,
//          lease and reset behavior on a model vibrator.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/inputtest/vibrator_policy_checks.h"

int main()
{
	vibratortest::Tally tally;
	vibratortest::CheckAll( vibratortest::RealPolicy(), tally );
	return testing::ReportConformance( tally.checks, tally.failures );
}
