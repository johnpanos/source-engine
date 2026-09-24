//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance of the KeyValues conditional expression evaluator
//          (tier1/kvconditional.h) against kv_conditional_checks.h.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/tier1test/kv_conditional_checks.h"

int main()
{
	kvcondtest::Tally tally;
	kvcondtest::CheckAll( EvaluateKVConditionalExpression, tally );
	return testing::ReportConformance( tally.checks, tally.failures );
}
