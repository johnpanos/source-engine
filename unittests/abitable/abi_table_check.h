//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The shared checker of recorded vtable tables (RFC 0016 K0
// `legacy.render-abi`, RFC 0010 V0 `legacy.vgui-abi`). A table suite
// includes this header, defines its table's X-macros over InterfaceCheck and
// prints `FAIL <Interface> ...` for each failed check, which the tables'
// sensitivity runs (tools/quality/abi_table.py) rely on to attribute a
// seeded change.
//
// The oracle is the compiler's own pointer-to-member-function value, never
// the header text: under the Itanium ABI a virtual member pointer carries
// its vtable byte offset and the this-adjustment a call through the
// interface applies. A probe class that appends one virtual to each
// interface measures its slot count; recorded methods and destructor pairs
// must partition every vtable exactly.
//
// Exact C++11: it is compiled in the legacy-cxx11 dialect, as a prebuilt
// consumer would be.
//
//=============================================================================//

#ifndef ABI_TABLE_CHECK_H
#define ABI_TABLE_CHECK_H

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>

namespace abi_table
{

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

} // namespace abi_table

#endif // ABI_TABLE_CHECK_H
