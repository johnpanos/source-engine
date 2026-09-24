//========= Portal 2 reconstruction ============================================//
//
// Purpose: paint_sphere: paints the brush surfaces inside a sphere with one
//          paint power when its Paint input fires.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). The retail server's
// Paint input does nothing unless the engine has a paint map for the level;
// with one, it queues a sphere paint of the world and of every entity inside
// the sphere into the engine paint map. Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "paint_sphere.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CPaintSphere )

	DEFINE_KEYFIELD( m_nPaintType, FIELD_INTEGER, "paint_type" ),
	DEFINE_KEYFIELD( m_flSphereRadius, FIELD_FLOAT, "radius" ),
	DEFINE_KEYFIELD( m_flAlphaPercent, FIELD_FLOAT, "alpha_percent" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Paint", Paint ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( paint_sphere, CPaintSphere );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPaintSphere::CPaintSphere( void )
{
	m_nPaintType = BOUNCE_POWER;
	m_flSphereRadius = 60.0f;
	m_flAlphaPercent = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Applies the paint type to the surfaces inside the sphere
//-----------------------------------------------------------------------------
void CPaintSphere::Paint( inputdata_t &inputdata )
{
	// Portal 2 port: sphere painting writes the engine's per-level paint maps
	// (retail: only when the engine reports a paint map, then a sphere paint of
	// the world and of each entity in UTIL_EntitiesInSphere). This engine has no
	// paint maps, so the input reports the unsupported feature and paints nothing.
	static bool s_bWarned = false;
	if ( !s_bWarned )
	{
		s_bWarned = true;
		DevWarning( "Portal 2: paint_sphere painting is not supported by this engine\n" );
	}

	DevMsg( 2, "paint_sphere '%s': power %d radius %.1f alpha %.2f at (%.1f %.1f %.1f) ignored\n",
		GetDebugName(), m_nPaintType, m_flSphereRadius, m_flAlphaPercent,
		GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
}
