//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side placement helper used by predicted portal placement
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_INFO_PLACEMENT_HELPER_H
#define C_INFO_PLACEMENT_HELPER_H
#ifdef _WIN32
#pragma once
#endif

class C_Portal_Base2D;

class C_InfoPlacementHelper : public C_BaseEntity
{
	DECLARE_CLASS( C_InfoPlacementHelper, C_BaseEntity );
	DECLARE_CLIENTCLASS();

public:
	C_InfoPlacementHelper( void );
	virtual void	Spawn( void );
	virtual void	UpdateOnRemove( void );

	float			GetTargetRadius( void ) { return m_flRadius; }

	// Reconstruction note: the client has no lookup of entities by targetname, so proxy and
	// attachment entities are never resolved here. The 852 client build returns the constant
	// orientation for a proxied helper, and the origin/override accessors follow suit.
	C_BaseEntity	*GetTargetOverride( void )
	{
		if ( m_strTargetEntity == NULL_STRING )
			return NULL;

		// Attachment targets only exist on the server
		return NULL;
	}

	const Vector	&GetTargetOrigin( void )
	{
		// Proxy entities are only known to the server
		if ( m_strTargetProxy != NULL_STRING )
			return vec3_origin;

		return GetAbsOrigin();
	}

	const QAngle	&GetTargetAngles( void )
	{
		// Proxy entities are only known to the server
		if ( m_strTargetProxy != NULL_STRING )
			return vec3_angle;

		return GetAbsAngles();
	}

	bool			ShouldUseHelperAngles( void );
	bool			ShouldForcePlacement( void ) { return m_bForcePlacement; }

	void			Disable( float flDuration ) { m_flDisableTime = gpGlobals->curtime + flDuration; }
	bool			IsEnabled( void );
	void			BindToPortal( C_Portal_Base2D *pPortal );

private:
	string_t		m_strTargetProxy;
	string_t		m_strTargetEntity;
	float			m_flRadius;
	bool			m_bSnapToHelperAngles;
	bool			m_bForcePlacement;
	bool			m_bDisabled;
	float			m_flDisableTime;
	bool			m_bDeferringToPortal;
};

C_InfoPlacementHelper *UTIL_FindPlacementHelper( const Vector &vecEndPoint, C_BasePlayer *pPlayer );

#endif // C_INFO_PLACEMENT_HELPER_H
