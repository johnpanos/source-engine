//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side fizzler trigger; predicts portal removal for the local gun
//			and drives the field's look (the FizzlerVortex material proxy and the
//			cleanser_scanline particle)
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled) and the retail
// Linux client.so. Not original Valve source; the repository's provenance and
// distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_trigger_portal_cleanser.h"
#include "c_portal_player.h"
#include "weapon_portalgun_shared.h"
#include "prop_portal_shared.h"
#ifdef PORTAL2
// Portal 2 port: the Portal 1 c_prop_portal.h in this directory would shadow it.
#include "portal2/portal/c_prop_portal.h"
#include "portal2/portal/c_weapon_portalgun.h"
#endif
#include "ispatialpartition.h"
#include "proxyentity.h"
#include "materialsystem/imaterialvar.h"
#include "imaterialproxydict.h"
#include "cdll_client_int.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/itexture.h"
#include "mathlib/vmatrix.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_portal_cleanser_default_intensity( "cl_portal_cleanser_default_intensity", "1.0f",
    FCVAR_CHEAT, "The default intensity of the cleanser field effect." );
ConVar cl_portal_cleanser_shot_pulse_time( "cl_portal_cleanser_shot_pulse_time", "0.1f",
    FCVAR_CHEAT, "The amount of time to pulse the cleanser field for when it is shot at." );
ConVar cl_portal_cleanser_shot_pulse_intensity( "cl_portal_cleanser_shot_pulse_intensity", "10.0f",
    FCVAR_CHEAT, "The intensity of the cleanser field when it gets shot at." );
ConVar cl_portal_cleanser_powerup_time( "cl_portal_cleanser_powerup_time", "1.0f", FCVAR_CHEAT,
    "The amount of time the power up sequence takes to complete." );
ConVar cl_portal_cleanser_scanline(
    "cl_portal_cleanser_scanline", "1", FCVAR_CHEAT, "Use particle scanline." );

// Controls only the core light source; the retained field draw stays visible.
static ConVar cl_fizzler_core_emission( "cl_fizzler_core_emission", "1", FCVAR_CHEAT,
    "Fizzlers emit two-sided area light on core receivers (0: lighting negative control)." );
static ConVar cl_fizzler_core_emission_report( "cl_fizzler_core_emission_report", "0", FCVAR_CHEAT,
    "Report the next successfully evaluated core fizzler emitter." );

IMPLEMENT_CLIENTCLASS_DT(
    C_TriggerPortalCleanser, DT_TriggerPortalCleanser, CTriggerPortalCleanser )
RecvPropBool( RECVINFO( m_bDisabled ) ), RecvPropBool( RECVINFO( m_bVisible ) ),
    RecvPropFloat( RECVINFO( m_flPortalShotTime ) ), RecvPropBool( RECVINFO( m_bObject1InRange ) ),
    RecvPropBool( RECVINFO( m_bObject2InRange ) ), RecvPropEHandle( RECVINFO( m_hObject1 ) ),
    RecvPropEHandle( RECVINFO( m_hObject2 ) ), RecvPropBool( RECVINFO( m_bUseScanline ) ),
    RecvPropBool( RECVINFO( m_bPlayersPassTriggerFilters ) ),
    END_RECV_TABLE()

        C_TriggerPortalCleanser::C_TriggerPortalCleanser()
{
	m_bDisabled = false;
	m_bVisible = false;
	m_bUseScanline = false;
	m_bPlayersPassTriggerFilters = false;
	m_bObject1InRange = false;
	m_bObject2InRange = false;
	m_flPortalShotTime = 0.0f;
	m_flPowerUpTimer = 0.0f;
	m_flLastShotTime = 0.0f;
	m_flShotPulseTimer = 0.0f;
	m_flLastUpdateTime = 0.0f;
	m_nStateFrame = -1;
	m_flFrameIntensity = 0.0f;
	m_flFramePowerUp = 0.0f;
	m_nEmissionModel = -1;
	m_pEmissionMaterial = NULL;
	m_bEmissionWarned = false;
	EmissiveAreaLights_AddSource( this );
}

C_TriggerPortalCleanser::~C_TriggerPortalCleanser()
{
	EmissiveAreaLights_RemoveSource( this );
	if ( m_pEmissionMaterial )
		m_pEmissionMaterial->DecrementReferenceCount();
	StopScanline();
}

//-----------------------------------------------------------------------------
// Purpose: Predict the cleanser fizzling the portals of a gun that passes through
//-----------------------------------------------------------------------------
void C_TriggerPortalCleanser::Touch( C_BaseEntity *pOther )
{
	if ( m_bDisabled )
		return;

	// The server tells whether players pass this trigger's filters at all.
	if ( !m_bPlayersPassTriggerFilters )
		return;

	// A dropped portalgun can pass through by itself
	const bool bIsPortalGun = ( pOther && FClassnameIs( pOther, "weapon_portalgun" ) );

	if ( !pOther->IsPlayer() && !bIsPortalGun )
		return;

	{
		C_Portal_Player *pPlayer = ToPortalPlayer( pOther );

		// In multiplayer, players without a gun have nothing to fizzle
		if ( gpGlobals->maxClients > 1 && pPlayer &&
		     !pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) )
			return;

		if ( pPlayer || bIsPortalGun )
		{
			C_WeaponPortalgun *pPortalgun =
			    pPlayer ? dynamic_cast<C_WeaponPortalgun *>(
			                  pPlayer->Weapon_OwnsThisType( "weapon_portalgun" ) )
			            : dynamic_cast<C_WeaponPortalgun *>( pOther );

			if ( pPortalgun )
			{
				bool bPortal1Active = false;
				bool bPortal2Active = false;

				// Deactivate any portals this gun owns
				bool bFizzledPortal = false;

				if ( pPortalgun->CanFirePortal1() )
				{
					CProp_Portal *pPortal = pPortalgun->GetAssociatedPortal( false );

					if ( pPortal && pPortal->IsActive() )
					{
						pPortal->SetActive( false );
						bPortal1Active = true;
						bFizzledPortal = true;
					}
				}

				if ( pPortalgun->CanFirePortal2() )
				{
					CProp_Portal *pPortal = pPortalgun->GetAssociatedPortal( true );

					if ( pPortal && pPortal->IsActive() )
					{
						pPortal->SetActive( false );
						bPortal2Active = true;
						bFizzledPortal = true;
					}
				}

				if ( bFizzledPortal )
				{
					pPortalgun->SendWeaponAnim( ACT_VM_FIZZLE );
					pPortalgun->DoCleanseEffect( bPortal1Active, bPortal2Active );
					pPortalgun->SetLastFiredPortal( 0 );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Cleansers live in the client trigger list so predicted touches reach them
//-----------------------------------------------------------------------------
void C_TriggerPortalCleanser::UpdatePartitionListEntry( void )
{
	partition->RemoveAndInsert( PARTITION_CLIENT_SOLID_EDICTS | PARTITION_CLIENT_RESPONSIVE_EDICTS |
	                                PARTITION_CLIENT_NON_STATIC_EDICTS, // remove
	    PARTITION_CLIENT_TRIGGER_ENTITIES,                              // add
	    CollisionProp()->GetPartitionHandle() );
}

void C_TriggerPortalCleanser::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
	m_nStateFrame = -1;
	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_flLastUpdateTime = gpGlobals->curtime;
	}
}

C_BaseEntity *C_TriggerPortalCleanser::GetVortexObject( int iObject )
{
	if ( iObject == 0 )
		return m_bObject1InRange ? m_hObject1.Get() : NULL;
	return m_bObject2InRange ? m_hObject2.Get() : NULL;
}

void C_TriggerPortalCleanser::GetCurrentState( float &flIntensity, float &flPowerUp )
{
	if ( m_nStateFrame == gpGlobals->framecount )
	{
		flIntensity = m_flFrameIntensity;
		flPowerUp = m_flFramePowerUp;
		return;
	}
	m_nStateFrame = gpGlobals->framecount;
	const float flLastUpdateTime = m_flLastUpdateTime;
	const float flFrameTime = gpGlobals->curtime - flLastUpdateTime;
	m_flLastUpdateTime = gpGlobals->curtime;

	// A portal shot hitting the field pulses its intensity up and back down.
	flIntensity = cl_portal_cleanser_default_intensity.GetFloat();
	if ( m_flPortalShotTime > m_flLastShotTime )
	{
		m_flLastShotTime = m_flPortalShotTime;
		m_flShotPulseTimer = 0.0f;
	}
	const float flPulseTime = cl_portal_cleanser_shot_pulse_time.GetFloat();
	if ( m_flPortalShotTime != 0.0f && m_flShotPulseTimer <= flPulseTime + flPulseTime )
	{
		m_flShotPulseTimer += flFrameTime;
		float flPulse = m_flShotPulseTimer < flPulseTime
		                    ? m_flShotPulseTimer / flPulseTime
		                    : ( flPulseTime - m_flShotPulseTimer ) / flPulseTime + 1.0f;
		flPulse = clamp( flPulse, 0.0f, 1.0f );
		flIntensity = Lerp( flPulse, cl_portal_cleanser_default_intensity.GetFloat(),
		    cl_portal_cleanser_shot_pulse_intensity.GetFloat() );
	}

	// The field powers up while enabled and back down while disabled.
	const float flPowerUpTime = cl_portal_cleanser_powerup_time.GetFloat();
	if ( !m_bDisabled )
	{
		if ( m_flPowerUpTimer < flPowerUpTime )
			m_flPowerUpTimer += flFrameTime;
	}
	else
	{
		if ( m_flPowerUpTimer > 0.0f )
			m_flPowerUpTimer = ( flLastUpdateTime + m_flPowerUpTimer ) - gpGlobals->curtime;
		StopScanline();
	}
	m_flPowerUpTimer = clamp( m_flPowerUpTimer, 0.0f, flPowerUpTime );
	flPowerUp = flPowerUpTime > 0.0f ? clamp( m_flPowerUpTimer / flPowerUpTime, 0.0f, 1.0f ) : 1.0f;

	m_flFrameIntensity = flIntensity;
	m_flFramePowerUp = flPowerUp;

	if ( !m_bDisabled && cl_portal_cleanser_scanline.GetBool() && m_bUseScanline )
		UpdateScanline();
	else
		StopScanline();
}

//-----------------------------------------------------------------------------
// Purpose: The cleanser_scanline particle sweeps the field's bounds; control
//			points 4 and 5 follow the vortex objects (or sit far below)
//-----------------------------------------------------------------------------
void C_TriggerPortalCleanser::UpdateScanline( void )
{
	Vector vMins, vMaxs;
	CollisionProp()->CollisionToWorldSpace( CollisionProp()->OBBMins(), &vMins );
	CollisionProp()->CollisionToWorldSpace( CollisionProp()->OBBMaxs(), &vMaxs );

	if ( !m_hScanlineEffect )
	{
		// Floor and ceiling fields get no scanline.
		if ( vMaxs.z - vMins.z < 32.0f )
			return;

		m_hScanlineEffect = ParticleProp()->Create( "cleanser_scanline", PATTACH_CUSTOMORIGIN, -1 );
		if ( !m_hScanlineEffect )
			return;

		m_hScanlineEffect->SetControlPoint( 0, vMaxs );
		m_hScanlineEffect->SetControlPoint( 1, Vector( vMins.x, vMins.y, vMaxs.z ) );
		m_hScanlineEffect->SetControlPoint( 2, Vector( vMaxs.x, vMaxs.y, vMins.z ) );
	}

	const Vector vHidden( vMins.x, vMins.y, vMins.z - 512.0f );
	C_BaseEntity *pObject1 = GetVortexObject( 0 );
	m_hScanlineEffect->SetControlPoint( 4, pObject1 ? pObject1->WorldSpaceCenter() : vHidden );
	C_BaseEntity *pObject2 = GetVortexObject( 1 );
	m_hScanlineEffect->SetControlPoint( 5, pObject2 ? pObject2->WorldSpaceCenter() : vHidden );
}

void C_TriggerPortalCleanser::StopScanline( void )
{
	if ( m_hScanlineEffect )
	{
		m_hScanlineEffect->StopEmission( false, false, true );
		m_hScanlineEffect = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: SolidEnergy's inputs from the cleanser it draws: the vortex objects,
//			the field intensity and the power-up amount
//-----------------------------------------------------------------------------

int C_TriggerPortalCleanser::GetAreaLights( area_light::AreaLight *pLights, int *pKeys, int nMax )
{
	static ConVarRef coreWorld( "r_core_world" );
	if ( !coreWorld.IsValid() || coreWorld.GetInt() != 1 || !cl_fizzler_core_emission.GetBool() )
		return 0;
	float intensity, powerUp;
	GetCurrentState( intensity, powerUp );
	if ( nMax < 1 || !pLights || !pKeys || !arealights || !m_bVisible ||
	     IsEffectActive( EF_NODRAW ) )
		return 0;
	if ( m_nEmissionModel != GetModelIndex() )
	{
		m_nEmissionModel = GetModelIndex();
		m_bEmissionWarned = false;
		if ( m_pEmissionMaterial )
			m_pEmissionMaterial->DecrementReferenceCount();
		m_pEmissionMaterial = NULL;
		if ( !arealights->GetEnergyFieldSurface( m_nEmissionModel, m_EmissionSurface ) )
		{
			Warning( "fizzler %d: no rectangular SolidEnergy face; core emission unavailable\n",
			    entindex() );
			return 0;
		}
		m_pEmissionMaterial =
		    materials->FindMaterial( m_EmissionSurface.material, TEXTURE_GROUP_OTHER, false );
		if ( !m_pEmissionMaterial || m_pEmissionMaterial->IsErrorMaterial() )
		{
			m_pEmissionMaterial = NULL;
			return 0;
		}
		m_pEmissionMaterial->IncrementReferenceCount();
	}
	if ( !m_pEmissionMaterial )
		return 0;
	auto refuse = [&]( const char *reason )
	{
		if ( !m_bEmissionWarned )
			Warning( "fizzler %d (%s): %s; core emission unavailable\n", entindex(),
			    m_EmissionSurface.material, reason );
		m_bEmissionWarned = true;
		return 0;
	};
	auto variable = [&]( const char *name ) -> IMaterialVar *
	{
		bool found = false;
		IMaterialVar *value = m_pEmissionMaterial->FindVar( name, &found, false );
		return found && value->IsDefined() ? value : NULL;
	};
	auto scalar = [&]( const char *name, float fallback )
	{
		IMaterialVar *value = variable( name );
		return value ? value->GetFloatValue() : fallback;
	};
	energy_field::Flow flow;
	// SolidEnergy uses IShaderAPI::CurrentTime (Plat_FloatTime), not simulation time.
	flow.time = float( Plat_FloatTime() );
	flow.intensity = intensity;
	flow.powerUp = powerUp;
	flow.outputIntensity = scalar( "$outputintensity", 1.0f );
	flow.worldUvScale = scalar( "$flow_worlduvscale", 1.0f );
	flow.normalUvScale = scalar( "$flow_normaluvscale", 1.0f );
	flow.noiseScale = scalar( "$flow_noise_scale", 1.0f );
	flow.interval = scalar( "$flow_timeintervalinseconds", 0.4f );
	flow.scrollDistance = scalar( "$flow_uvscrolldistance", 0.2f );
	flow.lerpExponent = scalar( "$flow_lerpexp", 1.0f );
	flow.vortexSize = scalar( "$flow_vortex_size", 1.0f );
	flow.cheap = scalar( "$flow_cheap", 0.0f ) != 0.0f;
	const char *colors[] = { "$flow_color", "$flow_vortex_color" };
	for ( int i = 0; i < 2; ++i )
	{
		IMaterialVar *value = variable( colors[i] );
		if ( !value )
			return refuse( "missing flow color" );
		value->GetVecValue( i == 0 ? flow.color : flow.vortexColor, 3 );
		C_BaseEntity *object = GetVortexObject( i );
		flow.vortexEnabled[i] = object != NULL;
		if ( object )
			for ( int k = 0; k < 3; ++k )
				flow.vortex[i][k] = object->WorldSpaceCenter()[k];
	}
	const char *names[] = {
	    "$basetexture", "$flowmap", "$flow_noise_texture", "$flowbounds", "$detail1", "$detail2" };
	ITexture *textures[6] = {};
	for ( int i = 0; i < 6; ++i )
	{
		IMaterialVar *value = variable( names[i] );
		if ( value && value->GetType() == MATERIAL_VAR_TYPE_TEXTURE )
			textures[i] = value->GetTextureValue();
	}
	flow.detail1 = textures[4] != NULL;
	flow.detail2 = textures[5] != NULL;
	flow.detail1Blend = int( scalar( "$detail1blendmode", 0 ) );
	flow.detail2Blend = int( scalar( "$detail2blendmode", 0 ) );
	// Model-format flow and vertex modulation require a separate caller cohort.
	if ( scalar( "$modelformat", 0 ) != 0 || scalar( "$vertexcolor", 0 ) != 0 )
		return refuse( "model-format flow or vertex modulation is outside this brush cohort" );
	energy_field::Surface surface = m_EmissionSurface;
	const matrix3x4_t &transform = EntityToWorldTransform();
	for ( int c = 0; c < 4; ++c )
	{
		Vector placed;
		VectorTransform(
		    Vector( surface.p[c][0], surface.p[c][1], surface.p[c][2] ), transform, placed );
		for ( int k = 0; k < 3; ++k )
			surface.p[c][k] = placed[k];
	}
	for ( float *tangent : { surface.tangentS, surface.tangentT } )
	{
		Vector placed;
		VectorRotate( Vector( tangent[0], tangent[1], tangent[2] ), transform, placed );
		for ( int k = 0; k < 3; ++k )
			tangent[k] = placed[k];
	}
	if ( IMaterialVar *value = variable( "$basetexturetransform" ) )
	{
		const VMatrix &matrix = value->GetMatrixValue();
		for ( int c = 0; c < 4; ++c )
		{
			const float u = surface.uv[c][0], v = surface.uv[c][1];
			surface.uv[c][0] = matrix[0][0] * u + matrix[0][1] * v + matrix[0][3];
			surface.uv[c][1] = matrix[1][0] * u + matrix[1][1] * v + matrix[1][3];
		}
	}
	if ( !energy_field::MeanLight(
	         surface, flow, 16,
	         [&]( int texture, float u, float v, float rgba[4] )
	         {
		         return EmissiveAreaLights_SampleFieldTexture(
		             textures[texture], u, v, texture == 0 || texture >= 4, rgba );
	         },
	         pLights[0] ) )
		return refuse( "missing texture, invalid flow settings or unsupported geometry" );
	if ( !( pLights[0].reach > 0.0f ) )
		return 0;
	pKeys[0] = EmissiveAreaLights_EntityKey( entindex(), 0 );
	if ( cl_fizzler_core_emission_report.GetBool() )
	{
		cl_fizzler_core_emission_report.SetValue( 0 );
		const area_light::AreaLight &light = pLights[0];
		Msg( "fizzler core emitter ent %d key %d material %s center %.3f %.3f %.3f area %.3f "
		     "radiance %.6f %.6f %.6f reach %.3f intensity %.3f powerup %.3f\n",
		    entindex(), pKeys[0], m_EmissionSurface.material, light.rect.center[0],
		    light.rect.center[1], light.rect.center[2], area_light::Area( light.rect ),
		    light.radiance[0], light.radiance[1], light.radiance[2], light.reach, intensity,
		    powerUp );
	}
	return 1;
}

class CFizzlerVortexProxy : public CEntityMaterialProxy
{
public:
	CFizzlerVortexProxy()
	    : m_pMaterial( NULL ), m_pVortexPos1( NULL ), m_pVortex1( NULL ), m_pVortexPos2( NULL ),
	      m_pVortex2( NULL ), m_pIntensity( NULL ), m_pPowerUp( NULL )
	{
	}

	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues )
	{
		m_pMaterial = pMaterial;
		bool bFound;
		m_pVortexPos1 = pMaterial->FindVar( "$FLOW_VORTEX_POS1", &bFound, true );
		if ( !bFound )
			return false;
		m_pVortex1 = pMaterial->FindVar( "$flow_vortex1", &bFound, true );
		if ( !bFound )
			return false;
		m_pVortexPos2 = pMaterial->FindVar( "$FLOW_VORTEX_POS2", &bFound, true );
		if ( !bFound )
			return false;
		m_pVortex2 = pMaterial->FindVar( "$flow_vortex2", &bFound, true );
		if ( !bFound )
			return false;
		m_pIntensity = pMaterial->FindVar( "$flow_color_intensity", &bFound, true );
		if ( !bFound )
			return false;
		m_pPowerUp = pMaterial->FindVar( "$powerup", &bFound, true );
		if ( !bFound )
			m_pPowerUp = NULL;
		return true;
	}

	virtual void OnBind( C_BaseEntity *pEntity )
	{
		C_TriggerPortalCleanser *pCleanser = dynamic_cast<C_TriggerPortalCleanser *>( pEntity );
		if ( !pCleanser )
			return;

		SetVortex( pCleanser->GetVortexObject( 0 ), m_pVortex1, m_pVortexPos1 );
		SetVortex( pCleanser->GetVortexObject( 1 ), m_pVortex2, m_pVortexPos2 );

		float flIntensity, flPowerUp;
		pCleanser->GetCurrentState( flIntensity, flPowerUp );
		m_pIntensity->SetFloatValue( flIntensity );
		if ( m_pPowerUp )
			m_pPowerUp->SetFloatValue( flPowerUp );
	}

	virtual IMaterial *GetMaterial() { return m_pMaterial; }

private:
	static void SetVortex( C_BaseEntity *pObject, IMaterialVar *pEnabled, IMaterialVar *pPosition )
	{
		if ( !pObject )
		{
			pEnabled->SetIntValue( 0 );
			return;
		}
		pEnabled->SetIntValue( 1 );
		const Vector &vCenter = pObject->WorldSpaceCenter();
		pPosition->SetVecValue( vCenter.x, vCenter.y, vCenter.z );
	}

	IMaterial *m_pMaterial;
	IMaterialVar *m_pVortexPos1;
	IMaterialVar *m_pVortex1;
	IMaterialVar *m_pVortexPos2;
	IMaterialVar *m_pVortex2;
	IMaterialVar *m_pIntensity;
	IMaterialVar *m_pPowerUp;
};

EXPOSE_MATERIAL_PROXY( CFizzlerVortexProxy, FizzlerVortex );
