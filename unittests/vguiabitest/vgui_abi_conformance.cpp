//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0010 V0 check `legacy.vgui-abi`: the vtable slots of the
// frozen VGUI interfaces that cross a module boundary match their recorded
// table, quality/fixtures/vgui-abi/vgui_abi_v1.h. RFC 0010 rebuilds VGUI's
// machinery beneath this surface; this suite is what keeps every prebuilt
// caller working while it does.
//
// Compiled in the legacy-cxx11 dialect, as a prebuilt C++11 consumer (a mod
// or plugin) would compile against these headers. The shared checker
// (unittests/abitable/abi_table_check.h, also used by legacy.render-abi)
// takes the compiler's own pointer-to-member-function value of every
// recorded method through its full recorded signature, so a changed
// parameter or return type fails to compile; a probe class that appends one
// virtual to each interface measures its slot count, so a method added
// anywhere (or a table that omits one) is detected; recorded methods and
// destructor pairs must partition every vtable exactly.
//
// The table is resolved inside namespace vgui, where the parameter types
// clang prints unqualified (HCursor, VPANEL) are found. Failures print
// `FAIL <Interface> ...`; tools/vgui/vgui_abi.py sensitivity relies on that
// to attribute each seeded change.
//
//=============================================================================//

#include <cstdio>
#include <cstring>

#include "vgui/ISurface.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "vgui/IPanel.h"
#include "vgui/IClientPanel.h"
#include "vgui/IVGui.h"
#include "vgui/IScheme.h"
#include "vgui/IBorder.h"
#include "vgui/IImage.h"
#include "vgui/IInput.h"
#include "vgui/IInputInternal.h"
#include "vgui/ISystem.h"
#include "vgui/ILocalize.h"
#include "vgui/IHTML.h"
#include "vgui/IVguiMatInfo.h"
#include "vgui/IVguiMatInfoVar.h"

#include "testing/conformance_result.h"
#include "unittests/abitable/abi_table_check.h"

#if __cplusplus != 201103L
#error "legacy.vgui-abi must compile as exact C++11 (the legacy-cxx11 dialect)"
#endif

namespace vgui
{
namespace
{
using abi_table::DecodeThrough;
using abi_table::InterfaceCheck;
using abi_table::Tally;

const char kExpectedTableVersion[] = "vgui-abi-v1";
const unsigned long kExpectedInterfaces = 17;

// Probes: each appends one virtual to an interface (or to a secondary base),
// so the appended function's slot is the vtable's function slot count. They
// are never instantiated and their vtables are never emitted. VguiAbiType_I
// names the interface's C++ type for the entries that follow.
#define VGUI_ABI_INTERFACE( I, Type, header, slots )                                               \
	typedef Type VguiAbiType_##I;                                                                  \
	struct VguiAbiProbe_##I : Type                                                                 \
	{                                                                                              \
		virtual void VguiAbiProbeTail();                                                           \
	};
#define VGUI_ABI_VERSION( I, macro, value )
#define VGUI_ABI_SECONDARY( I, Base, adj, slots )                                                  \
	struct VguiAbiProbe_##I##_##Base : Base                                                        \
	{                                                                                              \
		virtual void VguiAbiProbeTail();                                                           \
	};
#define VGUI_ABI_METHOD( I, adj, slot, signature, Declarer, Name, ... )
#define VGUI_ABI_DESTRUCTOR( I, adj, slot, kind )
#define VGUI_ABI_END( I )
#include "quality/fixtures/vgui-abi/vgui_abi_v1.h"
#undef VGUI_ABI_INTERFACE
#undef VGUI_ABI_VERSION
#undef VGUI_ABI_SECONDARY
#undef VGUI_ABI_METHOD
#undef VGUI_ABI_DESTRUCTOR
#undef VGUI_ABI_END
#undef VGUI_ABI_TABLE_VERSION

unsigned long g_interfaces = 0;

void CheckTable( Tally &tally )
{
#define VGUI_ABI_INTERFACE( I, Type, header, slots )                                               \
	{                                                                                              \
		++g_interfaces;                                                                            \
		InterfaceCheck check( tally, #I, header, slots,                                            \
		    DecodeThrough<VguiAbiProbe_##I>( &VguiAbiProbe_##I::VguiAbiProbeTail ) );
#define VGUI_ABI_VERSION( I, macro, value ) check.Version( #macro, macro, value );
#define VGUI_ABI_SECONDARY( I, Base, adj, slots )                                                  \
	check.Secondary( #Base, adj, slots, abi_table::BaseOffset<VguiAbiType_##I, Base>(),            \
	    DecodeThrough<VguiAbiProbe_##I##_##Base>(                                                  \
	        &VguiAbiProbe_##I##_##Base::VguiAbiProbeTail ) );
// The member pointer is formed in a class derived from its declarer: the
// recorded signature's nested types resolve there, and protected members may
// be named through it ([class.protected]). Its type still names the declarer.
#define VGUI_ABI_METHOD( I, adj, slot, signature, Declarer, Name, ... )                            \
	check.Method( signature, adj, slot,                                                            \
	    DecodeThrough<VguiAbiType_##I>(                                                            \
	        []()                                                                                   \
	        {                                                                                      \
		        struct VguiAbiAccess : Declarer                                                    \
		        {                                                                                  \
			        typedef __VA_ARGS__;                                                           \
			        static VguiAbiPmf Get()                                                        \
			        {                                                                              \
				        return static_cast<VguiAbiPmf>( &VguiAbiAccess::Name );                    \
			        }                                                                              \
		        };                                                                                 \
		        return VguiAbiAccess::Get();                                                       \
	        }() ) );
#define VGUI_ABI_DESTRUCTOR( I, adj, slot, kind ) check.Destructor( adj, slot, kind );
#define VGUI_ABI_END( I )                                                                          \
	check.Finish();                                                                                \
	}
#include "quality/fixtures/vgui-abi/vgui_abi_v1.h"

	++tally.checks;
	if ( std::strcmp( VGUI_ABI_TABLE_VERSION, kExpectedTableVersion ) != 0 )
	{
		++tally.failures;
		std::printf( "FAIL table version %s (expected %s)\n", VGUI_ABI_TABLE_VERSION,
		    kExpectedTableVersion );
	}
}
} // namespace
} // namespace vgui

int main()
{
	vgui::Tally tally = { 0, 0 };
	vgui::CheckTable( tally );
	// Every frozen VGUI interface of RFC 0010 V0 is in the table.
	++tally.checks;
	if ( vgui::g_interfaces != vgui::kExpectedInterfaces )
	{
		++tally.failures;
		std::printf( "FAIL table lists %lu interfaces (expected %lu)\n", vgui::g_interfaces,
		    vgui::kExpectedInterfaces );
	}
	std::printf( "legacy.vgui-abi: %lu interfaces, %lu checks, %lu failures\n", vgui::g_interfaces,
	    tally.checks, tally.failures );
	return testing::ReportConformance( tally.checks, tally.failures );
}
