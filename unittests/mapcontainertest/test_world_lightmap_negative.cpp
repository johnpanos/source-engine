//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity of the LMAP case table (RFC 0008 F4, RFC 0011): each
//          deliberately permissive validator below must get cases wrong.
//          A table they all passed could not tell a correct reader from a
//          careless one.
//
//=============================================================================//

#include "world_lightmap_cases.h"
#include "testing/conformance_result.h"

#include <cstdio>

using namespace mapcontainer;

namespace
{

// Accepts anything the real validator rejects only for its layer count.
WorldLightmapError AnyLayerCount(
    const void *p, size_t size, uint32_t version, WorldLightmapLayout *layout )
{
	const WorldLightmapError error = ValidateWorldLightmap( p, size, version, layout );
	return error == WorldLightmapError::InvalidLayerCount ? WorldLightmapError::Ok : error;
}

// Ignores the lump version (a v1 lump may carry layers, and the reverse).
WorldLightmapError IgnoresVersion(
    const void *p, size_t size, uint32_t, WorldLightmapLayout *layout )
{
	return ValidateWorldLightmap( p, size, 0, layout );
}

// Checks only the identifier: truncated, compressed or 3D data passes.
WorldLightmapError IdentifierOnly(
    const void *p, size_t size, uint32_t, WorldLightmapLayout *layout )
{
	const WorldLightmapError error = ValidateWorldLightmap( p, size, 0, layout );
	return error == WorldLightmapError::BadIdentifier || error == WorldLightmapError::Truncated
	           ? error
	           : WorldLightmapError::Ok;
}

// Validates but reports every layer as the total page.
WorldLightmapError AllTotal(
    const void *p, size_t size, uint32_t version, WorldLightmapLayout *layout )
{
	const WorldLightmapError error = ValidateWorldLightmap( p, size, version, layout );
	if ( error == WorldLightmapError::Ok && layout )
		for ( auto &role : layout->roles )
			role = WorldLightmapLayer::Total;
	return error;
}

} // namespace

int main()
{
	struct Bad
	{
		const char *name;
		lmap_cases::Validator validate;
	};
	const Bad bad[] = { { "accepts any layer count", &AnyLayerCount },
	    { "ignores the lump version", &IgnoresVersion },
	    { "checks only the identifier", &IdentifierOnly },
	    { "reports every layer as total", &AllTotal } };
	unsigned long checks = 0;
	unsigned long failures = 0;
	for ( const Bad &validator : bad )
	{
		const lmap_cases::CaseResult result = lmap_cases::RunLmapCases( validator.validate, false );
		++checks;
		std::printf(
		    "%-30s wrong on %d of %d cases\n", validator.name, result.wrong, result.cases );
		if ( result.wrong == 0 )
		{
			++failures;
			std::printf(
			    "FAIL the case table does not detect a validator that %s\n", validator.name );
		}
	}
	return testing::ReportConformance( checks, failures );
}
