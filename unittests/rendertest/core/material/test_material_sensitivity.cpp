//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 sensitivity (RFC 0016 K4 "Material suite"):
//			the material suite's key-mapping oracle and derived-copy clause
//			must pass the real mapping and ParameterBlockCopy and must catch
//			seeded bad ones: a VMT key mapped to the wrong parameter, a key
//			read with the wrong kind, a copy that is never refreshed after a
//			change (a stale GPU copy), and a copy whose revision follows the
//			block while its bytes do not.
//
//=============================================================================//

#include "material_conformance.h"
#include "testing/checks.h"

#include <string>
#include <vector>

namespace
{

using namespace render::material;

bool Mentions( const std::vector<std::string> &violations, std::string_view text )
{
	for ( const std::string &violation : violations )
	{
		if ( violation.find( text ) != std::string::npos )
			return true;
	}
	return false;
}

// The built-in mapping with one lightmapped row changed.
struct SeededMapping
{
	std::vector<VmtKeyRow> keys;
	VmtMappingTable table;

	template <typename Change> explicit SeededMapping( std::string_view key, Change change )
	{
		const VmtMappingTable &builtin = BuiltinVmtMapping();
		keys.assign( builtin.keys.begin(), builtin.keys.end() );
		for ( VmtKeyRow &row : keys )
		{
			if ( row.family == "lightmapped" && row.key == key )
				change( row );
		}
		table = builtin;
		table.keys = keys;
	}
};

// Claims to be current once it has copied anything: a derived copy that is
// not updated after a change.
class StaleCopy
{
public:
	bool Current( const ParameterBlock & ) const { return m_Copied; }
	bool Refresh( const ParameterBlock &block )
	{
		if ( m_Copied )
			return false;
		m_Copied = true;
		return m_Copy.Refresh( block );
	}
	std::span<const std::byte> Bytes() const { return m_Copy.Bytes(); }
	std::span<const render::device::TextureId> Textures() const { return m_Copy.Textures(); }
	std::uint64_t Revision() const { return m_Copy.Revision(); }

private:
	ParameterBlockCopy m_Copy;
	bool m_Copied = false;
};

// Tracks the block's revision but keeps the first bytes it copied.
class RevisionOnlyCopy
{
public:
	bool Current( const ParameterBlock &block ) const { return m_Revision == block.Revision(); }
	bool Refresh( const ParameterBlock &block )
	{
		if ( m_Revision == 0 )
			(void)m_Copy.Refresh( block );
		m_Revision = block.Revision();
		return true;
	}
	std::span<const std::byte> Bytes() const { return m_Copy.Bytes(); }
	std::span<const render::device::TextureId> Textures() const { return m_Copy.Textures(); }
	std::uint64_t Revision() const { return m_Revision; }

private:
	ParameterBlockCopy m_Copy;
	std::uint64_t m_Revision = 0;
};

} // namespace

int main()
{
	using namespace rendertest::material;
	testing::Checks checks;

	checks.That( KeyMappingViolations( BuiltinVmtMapping() ).empty(),
	    "S0.the-built-in-mapping-passes-the-oracle" );
	checks.That( DerivedCopyViolations<ParameterBlockCopy>().empty(),
	    "S0.parameter-block-copy-passes-the-revision-clause" );

	const SeededMapping wrongParameter( "$color",
	    []( VmtKeyRow &row )
	    {
		    row.parameter = "selfillumtint";
	    } );
	const std::vector<std::string> wrong = KeyMappingViolations( wrongParameter.table );
	checks.That( Mentions( wrong, "$color" ), "S1.a-key-mapped-to-the-wrong-parameter-is-caught" );
	checks.That( wrong.size() == 1, "S1.only-the-seeded-key-is-reported" );

	const SeededMapping wrongKind( "$envmaptint",
	    []( VmtKeyRow &row )
	    {
		    row.kind = ValueKind::kFloat;
	    } );
	checks.That( Mentions( KeyMappingViolations( wrongKind.table ), "$envmaptint" ),
	    "S2.a-key-read-with-the-wrong-kind-is-caught" );

	checks.That( !DerivedCopyViolations<StaleCopy>().empty(),
	    "S3.a-copy-not-updated-after-a-change-is-caught" );
	checks.That( !DerivedCopyViolations<RevisionOnlyCopy>().empty(),
	    "S4.a-copy-with-a-current-revision-and-stale-bytes-is-caught" );
	return checks.Report();
}
