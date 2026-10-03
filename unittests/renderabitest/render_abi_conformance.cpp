//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0016 K0 check `legacy.render-abi`: the vtable slots of the
// frozen material-system and studio-render interfaces (IMaterialSystem,
// IMatRenderContext, IMaterial, IMaterialVar, ITexture, IMesh,
// IMaterialProxy, IStudioRender) match their recorded table,
// quality/fixtures/render-abi/render_abi_v1.h.
//
// Compiled in the legacy-cxx11 dialect, as a prebuilt C++11 consumer (a mod
// or plugin) would compile against these headers. The oracle is the
// compiler's own pointer-to-member-function value, never the header text:
// under the Itanium ABI a virtual member pointer carries its vtable byte
// offset and the this-adjustment a call through the interface applies. Each
// recorded method is taken through its full recorded signature, so a changed
// parameter or return type fails to compile. A probe class that appends one
// virtual to each interface measures its slot count, so a method added
// anywhere (or a table that omits one) is detected; recorded methods and
// destructor pairs must partition every vtable exactly.
//
// Failures print `FAIL <Interface> ...`; tools/render/render_abi.py
// sensitivity relies on that to attribute each seeded reorder.
//
//=============================================================================//

#include <cstdio>
#include <cstring>

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialproxy.h"
#include "istudiorender.h"

#include "testing/conformance_result.h"
#include "unittests/abitable/abi_table_check.h"

#if __cplusplus != 201103L
#error "legacy.render-abi must compile as exact C++11 (the legacy-cxx11 dialect)"
#endif

namespace
{
using abi_table::DecodeThrough;
using abi_table::InterfaceCheck;
using abi_table::Tally;

const char kExpectedTableVersion[] = "render-abi-v1";

// Probes: each appends one virtual to an interface (or to a secondary base),
// so the appended function's slot is the vtable's function slot count. They
// are never instantiated and their vtables are never emitted.
#define RENDER_ABI_INTERFACE( I, header, slots )                                                   \
	struct RenderAbiProbe_##I : I                                                                  \
	{                                                                                              \
		virtual void RenderAbiProbeTail();                                                         \
	};
#define RENDER_ABI_VERSION( I, macro, value )
#define RENDER_ABI_SECONDARY( I, Base, adj, slots )                                                \
	struct RenderAbiProbe_##I##_##Base : Base                                                      \
	{                                                                                              \
		virtual void RenderAbiProbeTail();                                                         \
	};
#define RENDER_ABI_METHOD( I, adj, slot, signature, Declarer, Name, ... )
#define RENDER_ABI_DESTRUCTOR( I, adj, slot, kind )
#define RENDER_ABI_END( I )
#include "quality/fixtures/render-abi/render_abi_v1.h"
#undef RENDER_ABI_INTERFACE
#undef RENDER_ABI_VERSION
#undef RENDER_ABI_SECONDARY
#undef RENDER_ABI_METHOD
#undef RENDER_ABI_DESTRUCTOR
#undef RENDER_ABI_END
#undef RENDER_ABI_TABLE_VERSION

unsigned long g_interfaces = 0;

void CheckTable( Tally &tally )
{
#define RENDER_ABI_INTERFACE( I, header, slots )                                                   \
	{                                                                                              \
		++g_interfaces;                                                                            \
		InterfaceCheck check( tally, #I, header, slots,                                            \
		    DecodeThrough<RenderAbiProbe_##I>( &RenderAbiProbe_##I::RenderAbiProbeTail ) );
#define RENDER_ABI_VERSION( I, macro, value ) check.Version( #macro, macro, value );
#define RENDER_ABI_SECONDARY( I, Base, adj, slots )                                                \
	check.Secondary( #Base, adj, slots, abi_table::BaseOffset<I, Base>(),                          \
	    DecodeThrough<RenderAbiProbe_##I##_##Base>(                                                \
	        &RenderAbiProbe_##I##_##Base::RenderAbiProbeTail ) );
// The member pointer is formed in a class derived from its declarer: the
// recorded signature's nested types resolve there, and protected members may
// be named through it ([class.protected]). Its type still names the declarer.
#define RENDER_ABI_METHOD( I, adj, slot, signature, Declarer, Name, ... )                          \
	check.Method( signature, adj, slot,                                                            \
	    DecodeThrough<I>(                                                                          \
	        []()                                                                                   \
	        {                                                                                      \
		        struct RenderAbiAccess : Declarer                                                  \
		        {                                                                                  \
			        typedef __VA_ARGS__;                                                           \
			        static RenderAbiPmf Get()                                                      \
			        {                                                                              \
				        return static_cast<RenderAbiPmf>( &RenderAbiAccess::Name );                \
			        }                                                                              \
		        };                                                                                 \
		        return RenderAbiAccess::Get();                                                     \
	        }() ) );
#define RENDER_ABI_DESTRUCTOR( I, adj, slot, kind ) check.Destructor( adj, slot, kind );
#define RENDER_ABI_END( I )                                                                        \
	check.Finish();                                                                                \
	}
#include "quality/fixtures/render-abi/render_abi_v1.h"

	++tally.checks;
	if ( std::strcmp( RENDER_ABI_TABLE_VERSION, kExpectedTableVersion ) != 0 )
	{
		++tally.failures;
		std::printf( "FAIL table version %s (expected %s)\n", RENDER_ABI_TABLE_VERSION,
		    kExpectedTableVersion );
	}
}
} // namespace

int main()
{
	Tally tally = { 0, 0 };
	CheckTable( tally );
	// All eight frozen interfaces of RFC 0016 K0 are in the table.
	++tally.checks;
	if ( g_interfaces != 8 )
	{
		++tally.failures;
		std::printf( "FAIL table lists %lu interfaces (expected 8)\n", g_interfaces );
	}
	std::printf( "legacy.render-abi: %lu interfaces, %lu checks, %lu failures\n", g_interfaces,
	    tally.checks, tally.failures );
	return testing::ReportConformance( tally.checks, tally.failures );
}
