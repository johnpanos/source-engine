//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive tool-process conformance against the deterministic fake.
//
//=============================================================================//

#include "fake_tool_process.h"
#include "tool_process_conformance.h"
#include "tool_process_fixture.h"
#include "testing/conformance_result.h"

int main()
{
	platformtest::CTestCancellation cancellation( true );
	const platformtest::ToolProcessFixture fixture =
	    platformtest::MakeToolProcessFixture( &cancellation );
	platformtest::CFakeToolProcessProvider provider( fixture );
	const platformtest::ToolProcessReport report =
	    platformtest::RunToolProcessConformance( provider, fixture );
	platformtest::ReportToolProcessConformance( "platform.tool_process", report );
	return testing::ReportConformance( report.checks, report.failures );
}
