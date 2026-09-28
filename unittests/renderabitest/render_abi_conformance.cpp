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

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialproxy.h"
#include "istudiorender.h"

#include "testing/conformance_result.h"

#if __cplusplus != 201103L
#error "legacy.render-abi must compile as exact C++11 (the legacy-cxx11 dialect)"
#endif

namespace
{
const char kExpectedTableVersion[] = "render-abi-v1";

struct DecodedMember
{
	bool isVirtual;
	long long adjust; // bytes added to an I* before the call
	long long slot;   // function index in the vtable that pointer selects
};

// Decodes a member function pointer as seen through an I*: the implicit
// base-to-derived member conversion applies the base's offset in I.
template <class I, class F, class D> DecodedMember DecodeThrough( F D::*member )
{
	F I::*through = member;
	struct Raw
	{
		std::ptrdiff_t ptr;
		std::ptrdiff_t adj;
	} raw;
	static_assert( sizeof( through ) == sizeof( raw ), "Itanium member function pointer layout" );
	std::memcpy( &raw, &through, sizeof( raw ) );
	DecodedMember decoded;
#if defined( __arm__ ) || defined( __aarch64__ )
	// ARM variant of the Itanium ABI: the virtual flag is adj's low bit.
	decoded.isVirtual = ( raw.adj & 1 ) != 0;
	decoded.adjust = raw.adj >> 1;
	decoded.slot = decoded.isVirtual ? raw.ptr / (std::ptrdiff_t)sizeof( void * ) : -1;
#else
	decoded.isVirtual = ( raw.ptr & 1 ) != 0;
	decoded.adjust = raw.adj;
	decoded.slot = decoded.isVirtual ? ( raw.ptr - 1 ) / (std::ptrdiff_t)sizeof( void * ) : -1;
#endif
	return decoded;
}

// Offset of Base inside I, from a pointer conversion.
template <class I, class Base> long long BaseOffset()
{
	static char storage[64];
	I *derived = reinterpret_cast<I *>( storage );
	Base *base = derived;
	return reinterpret_cast<char *>( base ) - reinterpret_cast<char *>( derived );
}

struct Tally
{
	unsigned long checks;
	unsigned long failures;
};

class InterfaceCheck
{
public:
	InterfaceCheck(
	    Tally &tally, const char *name, const char *header, long long slots, DecodedMember tail )
	    : m_tally( tally ), m_name( name ), m_header( header )
	{
		m_vtables[0].assign( (std::size_t)( slots > 0 ? slots : 0 ), 0 );
		m_derived[0].assign( m_vtables[0].size(), 0 );
		Check( tail.isVirtual && tail.adjust == 0 && tail.slot == slots,
		    "primary vtable has %lld function slots (probe tail slot %lld)", slots, tail.slot );
	}

	void Version( const char *macro, const char *actual, const char *recorded )
	{
		Check( std::strcmp( actual, recorded ) == 0, "%s is \"%s\" (recorded \"%s\")", macro,
		    actual, recorded );
	}

	void Secondary( const char *base, long long adjust, long long slots, long long actualAdjust,
	    DecodedMember tail )
	{
		m_vtables[adjust].assign( (std::size_t)( slots > 0 ? slots : 0 ), 0 );
		m_derived[adjust].assign( m_vtables[adjust].size(), 0 );
		Check( actualAdjust == adjust, "%s subobject at offset %lld (recorded %lld)", base,
		    actualAdjust, adjust );
		Check( tail.isVirtual && tail.adjust == 0 && tail.slot == slots,
		    "%s vtable has %lld function slots (probe tail slot %lld)", base, slots, tail.slot );
	}

	void Method( const char *signature, long long adjust, long long slot, DecodedMember actual )
	{
		Check( actual.isVirtual && actual.adjust == adjust && actual.slot == slot,
		    "%s: slot %lld adj %lld (recorded slot %lld adj %lld, virtual %d)", signature,
		    actual.slot, actual.adjust, slot, adjust, actual.isVirtual ? 1 : 0 );
		Mark( m_vtables, adjust, slot );
		Mark( m_derived, actual.adjust, actual.slot );
	}

	void Destructor( long long adjust, long long slot, const char *kind )
	{
		(void)kind;
		Mark( m_vtables, adjust, slot );
		Mark( m_derived, adjust, slot );
	}

	void Finish()
	{
		// Every recorded vtable is partitioned exactly by the recorded entries,
		// and exactly by the slots the compiler derived for them.
		Partition( m_vtables, "recorded entries" );
		Partition( m_derived, "derived slots" );
		std::printf( "interface %s (%s): %lu checks\n", m_name, m_header, m_checks );
	}

private:
	typedef std::map<long long, std::vector<int>> Coverage;

	void Mark( Coverage &coverage, long long adjust, long long slot )
	{
		Coverage::iterator vtable = coverage.find( adjust );
		if ( vtable == coverage.end() || slot < 0 || slot >= (long long)vtable->second.size() )
		{
			m_outOfRange = true;
			return;
		}
		++vtable->second[(std::size_t)slot];
	}

	void Partition( const Coverage &coverage, const char *what )
	{
		for ( Coverage::const_iterator vtable = coverage.begin(); vtable != coverage.end();
		    ++vtable )
		{
			long long holes = 0, doubles = 0;
			for ( std::size_t slot = 0; slot < vtable->second.size(); ++slot )
			{
				holes += vtable->second[slot] == 0;
				doubles += vtable->second[slot] > 1;
			}
			Check( holes == 0 && doubles == 0 && !m_outOfRange,
			    "%s cover vtable adj %lld exactly (%lld unfilled, %lld doubled, out of range %d)",
			    what, vtable->first, holes, doubles, m_outOfRange ? 1 : 0 );
		}
	}

	void Check( bool ok, const char *format, ... ) __attribute__( ( format( printf, 3, 4 ) ) )
	{
		++m_tally.checks;
		++m_checks;
		if ( ok )
			return;
		++m_tally.failures;
		char message[1024];
		va_list args;
		va_start( args, format );
		std::vsnprintf( message, sizeof( message ), format, args );
		va_end( args );
		std::printf( "FAIL %s %s\n", m_name, message );
	}

	Tally &m_tally;
	const char *m_name;
	const char *m_header;
	unsigned long m_checks = 0;
	bool m_outOfRange = false;
	Coverage m_vtables;
	Coverage m_derived;
};

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
	check.Secondary( #Base, adj, slots, BaseOffset<I, Base>(),                                     \
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
