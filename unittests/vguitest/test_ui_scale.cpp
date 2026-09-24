//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance of the VGUI UI scale policy (vguimatsurface/UIScale.h):
//          scale selection from the display scale, ui_scale and presentation,
//          screen extents, cursor coordinates and font sizes.
//
//=============================================================================//

#include "testing/conformance_result.h"
#include "unittests/vguitest/ui_scale_checks.h"

int main()
{
	uiscaletest::Tally tally;
	uiscaletest::CheckAll( uiscaletest::RealPolicy(), tally );
	return testing::ReportConformance( tally.checks, tally.failures );
}
