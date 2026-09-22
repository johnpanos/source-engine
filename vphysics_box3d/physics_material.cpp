//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D surface-property database (authored content, no IVP coupling).
//
//=============================================================================//
#include "physics_material.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "tier1/strtools.h"
#include "vcollide_parse_private.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Mirrors vphysics/physics_material.h's reserved index without pulling the IVP
// header in. Indices above 127 are reserved (the shadow material).
enum
{
	MATERIAL_INDEX_SHADOW = 0xF000,
};

CPhysicsSurfacePropsBox3D::CPhysicsSurfacePropsBox3D()
	: m_strings( 0, 32, true ), m_fileList( 8, 8 )
{
	// Index 0 is the empty string, so game code can treat 0 as "none" and still
	// resolve to a valid string.
	m_strings.AddString( "" );
	m_init = false;
	m_shadowFallback = 0;
}

CPhysicsSurfacePropsBox3D::~CPhysicsSurfacePropsBox3D()
{
}

int CPhysicsSurfacePropsBox3D::SurfacePropCount( void ) const
{
	return m_props.Size();
}

bool CPhysicsSurfacePropsBox3D::AddFileToDatabase( const char *pFilename )
{
	CUtlSymbol id = m_strings.AddString( pFilename );
	for ( int i = 0; i < m_fileList.Size(); i++ )
	{
		if ( m_fileList[i] == id )
			return false;
	}
	m_fileList.AddToTail( id );
	return true;
}

bool CPhysicsSurfacePropsBox3D::IsReservedMaterialIndex( int materialIndex ) const
{
	return materialIndex > 127;
}

const char *CPhysicsSurfacePropsBox3D::GetReservedMaterialName( int materialIndex ) const
{
	switch ( materialIndex )
	{
	case MATERIAL_INDEX_SHADOW:
		return "$MATERIAL_INDEX_SHADOW";
	}
	return NULL;
}

int CPhysicsSurfacePropsBox3D::GetReservedFallBack( int materialIndex ) const
{
	switch ( materialIndex )
	{
	case MATERIAL_INDEX_SHADOW:
		return m_shadowFallback;
	}
	return 0;
}

int CPhysicsSurfacePropsBox3D::GetReservedSurfaceIndex( const char *pPropertyName ) const
{
	if ( !Q_stricmp( pPropertyName, "$MATERIAL_INDEX_SHADOW" ) )
		return MATERIAL_INDEX_SHADOW;
	return -1;
}

int CPhysicsSurfacePropsBox3D::GetSurfaceIndex( const char *pPropertyName ) const
{
	if ( pPropertyName[0] == '$' )
	{
		int index = GetReservedSurfaceIndex( pPropertyName );
		if ( index >= 0 )
			return index;
	}

	CUtlSymbol id = m_strings.Find( pPropertyName );
	if ( id.IsValid() )
	{
		for ( int i = 0; i < m_props.Size(); i++ )
		{
			if ( m_props[i].m_name == id )
				return i;
		}
	}
	return -1;
}

const char *CPhysicsSurfacePropsBox3D::GetPropName( int surfaceDataIndex ) const
{
	const CSurfaceBox3D *pSurface = GetInternalSurface( surfaceDataIndex );
	if ( pSurface )
		return m_strings.String( pSurface->m_name );
	return NULL;
}

CSurfaceBox3D *CPhysicsSurfacePropsBox3D::GetInternalSurface( int materialIndex )
{
	if ( IsReservedMaterialIndex( materialIndex ) )
		materialIndex = GetReservedFallBack( materialIndex );
	if ( materialIndex < 0 || materialIndex > m_props.Size() - 1 )
		return NULL;
	return &m_props[materialIndex];
}

const CSurfaceBox3D *CPhysicsSurfacePropsBox3D::GetInternalSurface( int materialIndex ) const
{
	return const_cast<CPhysicsSurfacePropsBox3D *>( this )->GetInternalSurface( materialIndex );
}

void CPhysicsSurfacePropsBox3D::GetPhysicsProperties( int materialIndex, float *density, float *thickness, float *friction, float *elasticity ) const
{
	const CSurfaceBox3D *pSurface = GetInternalSurface( materialIndex );
	if ( !pSurface )
		pSurface = GetInternalSurface( GetSurfaceIndex( "default" ) );
	if ( pSurface )
	{
		if ( friction ) *friction = (float)pSurface->data.physics.friction;
		if ( elasticity ) *elasticity = (float)pSurface->data.physics.elasticity;
		if ( density ) *density = pSurface->data.physics.density;
		if ( thickness ) *thickness = pSurface->data.physics.thickness;
	}
}

void CPhysicsSurfacePropsBox3D::GetPhysicsParameters( int surfaceDataIndex, surfacephysicsparams_t *pParamsOut ) const
{
	if ( !pParamsOut )
		return;
	const CSurfaceBox3D *pSurface = GetInternalSurface( surfaceDataIndex );
	if ( pSurface )
		*pParamsOut = pSurface->data.physics;
}

surfacedata_t *CPhysicsSurfacePropsBox3D::GetSurfaceData( int materialIndex )
{
	CSurfaceBox3D *pSurface = GetInternalSurface( materialIndex );
	if ( !pSurface )
		pSurface = GetInternalSurface( 0 ); // 0 is always "default"
	// The database always contains at least the default + shadow records after
	// the first parse, so this never returns null in a running game.
	static surfacedata_t s_empty;
	return pSurface ? &pSurface->data : &s_empty;
}

const char *CPhysicsSurfacePropsBox3D::GetString( unsigned short stringTableIndex ) const
{
	return m_strings.String( stringTableIndex );
}

void CPhysicsSurfacePropsBox3D::SetWorldMaterialIndexTable( int *pMapArray, int mapSize )
{
	// Box3D consumes authored material indices directly for now; no IVP LUT.
}

void CPhysicsSurfacePropsBox3D::CopyPhysicsProperties( CSurfaceBox3D *pOut, int baseIndex )
{
	const CSurfaceBox3D *pSurface = GetInternalSurface( baseIndex );
	if ( pSurface )
		pOut->data = pSurface->data;
}

int CPhysicsSurfacePropsBox3D::ParseSurfaceData( const char *pFileName, const char *pTextfile )
{
	// This surface database owns authored-name resolution for the shared
	// vcollide parser while the Box3D provider is active.
	VPhysicsParseSetSurfaceProps( this );

	if ( !AddFileToDatabase( pFileName ) )
		return 0;

	const char *pText = pTextfile;
	do
	{
		char key[MAX_KEYVALUE], value[MAX_KEYVALUE];

		pText = ParseKeyvalue( pText, key, value );
		if ( !strcmp( value, "{" ) )
		{
			CSurfaceBox3D prop;
			memset( &prop.data, 0, sizeof( prop.data ) );
			prop.m_name = m_strings.AddString( key );
			int baseMaterial = GetSurfaceIndex( key );
			if ( baseMaterial < 0 )
				baseMaterial = GetSurfaceIndex( "default" );

			CopyPhysicsProperties( &prop, baseMaterial );

			do
			{
				pText = ParseKeyvalue( pText, key, value );
				if ( !Q_stricmp( key, "}" ) )
				{
					// If already present, override its data instead of re-adding.
					const char *pOverride = m_strings.String( prop.m_name );
					int propIndex = GetSurfaceIndex( pOverride );
					if ( propIndex >= 0 )
					{
						CSurfaceBox3D *pSurface = GetInternalSurface( propIndex );
						pSurface->data = prop.data;
						break;
					}
					m_props.AddToTail( prop );
					break;
				}
				else if ( !Q_stricmp( key, "base" ) )
				{
					baseMaterial = GetSurfaceIndex( value );
					CopyPhysicsProperties( &prop, baseMaterial );
				}
				else if ( !Q_stricmp( key, "thickness" ) )
					prop.data.physics.thickness = atof( value );
				else if ( !Q_stricmp( key, "density" ) )
					prop.data.physics.density = atof( value );
				else if ( !Q_stricmp( key, "elasticity" ) )
					prop.data.physics.elasticity = atof( value );
				else if ( !Q_stricmp( key, "friction" ) )
					prop.data.physics.friction = atof( value );
				else if ( !Q_stricmp( key, "maxspeedfactor" ) )
					prop.data.game.maxSpeedFactor = atof( value );
				else if ( !Q_stricmp( key, "jumpfactor" ) )
					prop.data.game.jumpFactor = atof( value );
				else if ( !Q_stricmp( key, "climbable" ) )
					prop.data.game.climbable = atoi( value );
				else if ( !Q_stricmp( key, "audioReflectivity" ) )
					prop.data.audio.reflectivity = atof( value );
				else if ( !Q_stricmp( key, "audioHardnessFactor" ) )
					prop.data.audio.hardnessFactor = atof( value );
				else if ( !Q_stricmp( key, "audioHardMinVelocity" ) )
					prop.data.audio.hardVelocityThreshold = atof( value );
				else if ( !Q_stricmp( key, "audioRoughnessFactor" ) )
					prop.data.audio.roughnessFactor = atof( value );
				else if ( !Q_stricmp( key, "scrapeRoughThreshold" ) )
					prop.data.audio.roughThreshold = atof( value );
				else if ( !Q_stricmp( key, "impactHardThreshold" ) )
					prop.data.audio.hardThreshold = atof( value );
				else if ( !Q_stricmp( key, "stepleft" ) )
					prop.data.sounds.stepleft = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "stepright" ) )
					prop.data.sounds.stepright = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "impactsoft" ) )
					prop.data.sounds.impactSoft = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "impacthard" ) )
					prop.data.sounds.impactHard = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "scrapesmooth" ) )
					prop.data.sounds.scrapeSmooth = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "scraperough" ) )
					prop.data.sounds.scrapeRough = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "bulletimpact" ) )
					prop.data.sounds.bulletImpact = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "break" ) )
					prop.data.sounds.breakSound = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "strain" ) )
					prop.data.sounds.strainSound = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "rolling" ) )
					prop.data.sounds.rolling = m_strings.AddString( value );
				else if ( !Q_stricmp( key, "gamematerial" ) )
				{
					if ( strlen( value ) == 1 && !V_isdigit( value[0] ) )
						prop.data.game.material = toupper( value[0] );
					else
						prop.data.game.material = atoi( value );
				}
				else if ( !Q_stricmp( key, "dampening" ) )
					prop.data.physics.dampening = atof( value );
				// Unknown keys are ignored: authored surfaceprops files carry
				// keys this backend does not consume yet, and a running game
				// must not abort level load over one.
			} while ( pText );
		}
	} while ( pText );

	if ( !m_init )
	{
		m_init = true;
		// Reserved shadow material, derived from "default".
		CSurfaceBox3D prop;
		int baseMaterial = GetSurfaceIndex( "default" );
		memset( &prop.data, 0, sizeof( prop.data ) );
		prop.m_name = m_strings.AddString( GetReservedMaterialName( MATERIAL_INDEX_SHADOW ) );
		CopyPhysicsProperties( &prop, baseMaterial );
		prop.data.physics.elasticity = 1e-3f;
		prop.data.physics.friction = 0.8f;
		m_shadowFallback = m_props.AddToTail( prop );
	}
	return m_props.Size();
}
