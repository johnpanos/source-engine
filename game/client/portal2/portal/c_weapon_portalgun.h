//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Portal 2 portal gun (client).
//
// Portal 2 reconstruction: this header was absent from the imported Portal 2
// source; the Portal 1 gun occupies game/client/portal/c_weapon_portalgun.h.
// The class layout follows the Steam2 depot 852_3 client.dylib DWARF, extended
// with the members the retained Portal 2 weapon_portalgun_shared.cpp uses.
//
//=============================================================================//

#ifndef PORTAL2_C_WEAPON_PORTALGUN_H
#define PORTAL2_C_WEAPON_PORTALGUN_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_portalbasecombatweapon.h"

#include "c_prop_portal.h"
#include "portal_placement.h"
#include "fx_interpvalue.h"
#include "beamdraw.h"
#include "iviewrender_beams.h"

struct TracePortalPlacementInfo_t;
struct ComplexPortalTrace_t;
struct dlight_t;

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//  CPortalgunEffect class
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class CPortalgunEffect
{
public:

	CPortalgunEffect( void )
		: m_vecColor( 255, 255, 255 ),
		  m_bVisibleViewModel( true ),
		  m_bVisible3rdPerson( true ),
		  m_nAttachment( -1 )
	{}

	void SetAttachment( int attachment ) { m_nAttachment = attachment; }
	int	GetAttachment( void ) const { return m_nAttachment; }

	void SetVisible( bool visible = true ) { m_bVisibleViewModel = visible; m_bVisible3rdPerson = visible; }

	void SetVisibleViewModel( bool visible = true ) { m_bVisibleViewModel = visible; }
	int IsVisibleViewModel( void ) const { return m_bVisibleViewModel; }

	void SetVisible3rdPerson( bool visible = true ) { m_bVisible3rdPerson = visible; }
	int IsVisible3rdPerson( void ) const { return m_bVisible3rdPerson; }

	void SetColor( const Vector &color ) { m_vecColor = color; }
	const Vector &GetColor( void ) const { return m_vecColor; }

	bool SetMaterial( const char *materialName )
	{
		m_hMaterial.Init( materialName, TEXTURE_GROUP_CLIENT_EFFECTS );
		return ( m_hMaterial != NULL );
	}

	CMaterialReference &GetMaterial( void ) { return m_hMaterial; }

	CInterpolatedValue &GetAlpha( void ) { return m_Alpha; }
	CInterpolatedValue &GetScale( void ) { return m_Scale; }

	virtual int GetWeaponID( void ) const { return WEAPON_PORTALGUN; }

private:
	CInterpolatedValue	m_Alpha;
	CInterpolatedValue	m_Scale;

	Vector				m_vecColor;
	bool				m_bVisibleViewModel;
	bool				m_bVisible3rdPerson;
	int					m_nAttachment;
	CMaterialReference	m_hMaterial;
};


class CPortalgunEffectBeam
{
public:
	CPortalgunEffectBeam( void );
	~CPortalgunEffectBeam( void );

	void Release( void );

	void Init( int startAttachment, int endAttachment, C_BaseEntity *pEntity, bool firstPerson );

	void SetVisibleViewModel( bool visible = true );
	int IsVisibleViewModel( void ) const;

	void SetVisible3rdPerson( bool visible = true );
	int SetVisible3rdPerson( void ) const;

	void SetBrightness( float fBrightness );

	void DrawBeam( void );

private:
	Beam_t	*m_pBeam;

	float	m_fBrightness;
};


class C_WeaponPortalgun : public C_BasePortalCombatWeapon
{
public:
	DECLARE_CLASS( C_WeaponPortalgun, C_BasePortalCombatWeapon );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

private:
	CNetworkVar( bool, m_bCanFirePortal1 );	// Is able to use primary fire
	CNetworkVar( bool, m_bCanFirePortal2 );	// Is able to use secondary fire
	CNetworkVar( int, m_iLastFiredPortal );		// Which portal was placed last
	CNetworkVar( bool, m_bOpenProngs );		// Whether the prongs are open (holding an object)

	CNetworkVar( float, m_fEffectsMaxSize1 );
	CNetworkVar( float, m_fEffectsMaxSize2 );

public:
	virtual const Vector &GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_10DEGREES;
		return cone;
	}

	virtual void Precache( void );

	virtual void OnRestore( void );
	virtual void UpdateOnRemove( void );
	virtual void Spawn( void );

	virtual bool ShouldDrawCrosshair( void );
	void SetLastFiredPortal( int iLastFiredPortal ) { m_iLastFiredPortal = iLastFiredPortal; }
	int GetLastFiredPortal( void ) { return m_iLastFiredPortal; }
	bool IsHoldingObject( void ) { return m_bOpenProngs; }

	virtual bool Reload( void );
	void FillClip( void );
	void CheckHolsterReload( void );
	virtual void ItemHolsterFrame( void );
	virtual bool Holster( C_BaseCombatWeapon *pSwitchingTo = NULL );
	virtual bool Deploy( void );
	void ResetRefireTime( void );

	void SetCanFirePortal1( bool bCanFire = true );
	void SetCanFirePortal2( bool bCanFire = true );
	bool CanFirePortal1( void ) const;
	bool CanFirePortal2( void ) const;

	void FirePortal1( void );
	void FirePortal2( void );
	PortalPlacementResult_t FirePortal( bool bPortal2, Vector *pVector = NULL );
	bool TraceFirePortal( const Vector &vTraceStart, const Vector &vDirection, bool bPortal2, PortalPlacedBy_t ePlacedBy, TracePortalPlacementInfo_t &placementInfo );

	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack( void );

	void DelayAttack( float fDelay );

	void DryFire( void );
	void UseDeny( void );
	virtual float GetFireRate( void ) { return 0.7; };
	virtual void WeaponIdle( void );

	virtual Activity GetPrimaryAttackActivity( void );

	Vector GetBluePortalPosition( void ) { return m_vecBluePortalPos; }
	Vector GetOrangePortalPosition( void ) { return m_vecOrangePortalPos; }

	CProp_Portal *GetAssociatedPortal( bool bPortal2 );

	virtual int GetWeaponID( void ) const { return WEAPON_PORTALGUN; }

protected:
	void PostAttack( void );

	void StartEffects( void );	// Initialize all sprites and beams
	void StopEffects( bool stopSound = true );	// Hide all effects temporarily
	void DestroyEffects( void );	// Destroy all sprites and beams

	// Portalgun effects
	void DoEffect( int effectType, Vector *pos = NULL );

	void DoEffectClosed( void );
	void DoEffectReady( void );
	void DoEffectHolding( void );
	void DoEffectNone( void );

public:
	enum EffectType_t
	{
		PORTALGUN_GRAVLIGHT = 0,
		PORTALGUN_GRAVLIGHT_WORLD,
		PORTALGUN_PORTAL1LIGHT,
		PORTALGUN_PORTAL1LIGHT_WORLD,
		PORTALGUN_PORTAL2LIGHT,
		PORTALGUN_PORTAL2LIGHT_WORLD,
		PORTALGUN_PORTALTINTEDLIGHT,
		PORTALGUN_PORTALTINTEDLIGHT_WORLD,

		PORTALGUN_GLOW1,	// Must be in order!
		PORTALGUN_GLOW2,
		PORTALGUN_GLOW3,
		PORTALGUN_GLOW4,
		PORTALGUN_GLOW5,
		PORTALGUN_GLOW6,

		PORTALGUN_GLOW1_WORLD,
		PORTALGUN_GLOW2_WORLD,
		PORTALGUN_GLOW3_WORLD,
		PORTALGUN_GLOW4_WORLD,
		PORTALGUN_GLOW5_WORLD,
		PORTALGUN_GLOW6_WORLD,

		PORTALGUN_ENDCAP1,	// Must be in order!
		PORTALGUN_ENDCAP2,
		PORTALGUN_ENDCAP3,

		PORTALGUN_ENDCAP1_WORLD,
		PORTALGUN_ENDCAP2_WORLD,
		PORTALGUN_ENDCAP3_WORLD,

		PORTALGUN_MUZZLE_GLOW,

		PORTALGUN_MUZZLE_GLOW_WORLD,

		PORTALGUN_TUBE_BEAM1,
		PORTALGUN_TUBE_BEAM2,
		PORTALGUN_TUBE_BEAM3,
		PORTALGUN_TUBE_BEAM4,
		PORTALGUN_TUBE_BEAM5,

		PORTALGUN_TUBE_BEAM1_WORLD,
		PORTALGUN_TUBE_BEAM2_WORLD,
		PORTALGUN_TUBE_BEAM3_WORLD,
		PORTALGUN_TUBE_BEAM4_WORLD,
		PORTALGUN_TUBE_BEAM5_WORLD,

		NUM_PORTALGUN_PARAMETERS	// Must be last!
	};

	#define	NUM_GLOW_SPRITES ((C_WeaponPortalgun::PORTALGUN_GLOW6-C_WeaponPortalgun::PORTALGUN_GLOW1)+1)
	#define	NUM_GLOW_SPRITES_WORLD ((C_WeaponPortalgun::PORTALGUN_GLOW6_WORLD-C_WeaponPortalgun::PORTALGUN_GLOW1_WORLD)+1)
	#define NUM_ENDCAP_SPRITES ((C_WeaponPortalgun::PORTALGUN_ENDCAP3-C_WeaponPortalgun::PORTALGUN_ENDCAP1)+1)
	#define NUM_ENDCAP_SPRITES_WORLD ((C_WeaponPortalgun::PORTALGUN_ENDCAP3_WORLD-C_WeaponPortalgun::PORTALGUN_ENDCAP1_WORLD)+1)
	#define NUM_TUBE_BEAM_SPRITES ((C_WeaponPortalgun::PORTALGUN_TUBE_BEAM5-C_WeaponPortalgun::PORTALGUN_TUBE_BEAM1)+1)
	#define NUM_TUBE_BEAM_SPRITES_WORLD ((C_WeaponPortalgun::PORTALGUN_TUBE_BEAM5_WORLD-C_WeaponPortalgun::PORTALGUN_TUBE_BEAM1_WORLD)+1)

	#define	NUM_PORTALGUN_BEAMS	6

protected:
	void			DrawEffects( bool b3rdPerson );
	Vector			GetEffectColor( int iPalletIndex );
	void			GetEffectParameters( EffectType_t effectID, color32 &color, float &scale, IMaterial **pMaterial, Vector &vecAttachment, bool b3rdPerson );
	void			DrawEffectSprite( EffectType_t effectID, bool b3rdPerson );
	inline bool		IsEffectVisible( EffectType_t effectID, bool b3rdPerson );
	void			UpdateElementPosition( void );

	CPortalgunEffect		m_Parameters[NUM_PORTALGUN_PARAMETERS];	// Interpolated parameters for the effects
	CPortalgunEffectBeam	m_Beams[NUM_PORTALGUN_BEAMS];				// Beams

	int				m_nOldEffectState = 0;	// Used for parity checks (0 is EFFECT_NONE)
	bool			m_bOldCanFirePortal1 = false;
	bool			m_bOldCanFirePortal2 = false;

	bool			m_bPulseUp = false;
	float			m_fPulse = 0.0f;

	CNetworkVar( Vector, m_vecBluePortalPos );
	CNetworkVar( Vector, m_vecOrangePortalPos );

	unsigned char	m_iPortalLinkageGroupID = 0;

	dlight_t		*m_pELight = NULL;	// First-person view model light (portal_viewmodel_use_dlight)

	CNetworkVar( int, m_EffectState );	// Current state of the effects on the gun

	CHandle<C_Prop_Portal> m_hPrimaryPortal;
	CHandle<C_Prop_Portal> m_hSecondaryPortal;

public:
	virtual int		DrawModel( int flags, const RenderableInstance_t &instance ) OVERRIDE;
	virtual void	ViewModelDrawn( C_BaseViewModel *pBaseViewModel );
	virtual void	OnPreDataChanged( DataUpdateType_t updateType );
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	ClientThink( void );

	void DoEffectIdle( void );
	void DoEffectBlast( C_BaseEntity *pOwner, bool bPortal2, int iPlacedBy, const Vector &ptStart, const Vector &ptFinalPos, const QAngle &qStartAngles, float fDelay );
	void DoCleanseEffect( bool bPortal1Active, bool bPortal2Active );

	bool AttemptStealCoopPortal( TracePortalPlacementInfo_t &placementInfo );
	bool PortalTraceClippedByBlockers( ComplexPortalTrace_t *pTraceResults, int nNumResultSegments, const Vector &vecDirection, bool bIsSecondPortal, TracePortalPlacementInfo_t &placementInfo );
	bool AttemptSnapToPlacementHelper( CProp_Portal *pPortal, ComplexPortalTrace_t *pTraceResults, int nNumResultSegments, PortalPlacedBy_t ePlacedBy, TracePortalPlacementInfo_t &placementInfo );

public:
	DECLARE_ACTTABLE();

	C_WeaponPortalgun( void );
	virtual ~C_WeaponPortalgun( void );

private:
	C_WeaponPortalgun( const C_WeaponPortalgun & );
};

#endif // PORTAL2_C_WEAPON_PORTALGUN_H
