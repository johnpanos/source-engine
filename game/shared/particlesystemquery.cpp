//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: particle system definitions
//
//===========================================================================//

#include "cbase.h"
#include "particles/particles.h"
#include "baseparticleentity.h"
#include "entityparticletrail_shared.h"
#include "collisionutils.h"

#if defined( CLIENT_DLL )
#include "c_pixel_visibility.h"
#include "animation.h"
#include "tier3/mdlutils.h"
#endif

#if defined( CLIENT_DLL ) && defined( PORTAL2 )
#include "blobulator/Implicit/ImpTiler.h"
#endif

#ifdef TF_CLIENT_DLL
#include "tf_shareddefs.h"
#endif

#ifdef GAME_DLL
#include "ai_utils.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Interface to allow the particle system to call back into the game code
//-----------------------------------------------------------------------------
class CParticleSystemQuery : public CBaseAppSystem< IParticleSystemQuery >
{
public:
	// Inherited from IParticleSystemQuery
	virtual void GetLightingAtPoint( const Vector& vecOrigin, Color &cTint );
	virtual void TraceLine( const Vector& vecAbsStart,
							const Vector& vecAbsEnd, unsigned int mask, 
							const IHandleEntity *ignore,
							int collisionGroup, CBaseTrace *ptr );

	virtual bool MovePointInsideControllingObject( CParticleCollection *pParticles,
												   void *pObject,
												   Vector *pPnt );
	virtual void GetRandomPointsOnControllingObjectHitBox( 
		CParticleCollection *pParticles,
		int nControlPointNumber, 
		int nNumPtsOut,
		float flBBoxScale,
		int nNumTrysToGetAPointInsideTheModel,
		Vector *pPntsOut,
		Vector vecDirectionalBias,
		Vector *pHitBoxRelativeCoordOut,
		int *pHitBoxIndexOut
		);

	virtual int GetCollisionGroupFromName( const char *pszCollisionGroupName );


	virtual int GetControllingObjectHitBoxInfo(
		CParticleCollection *pParticles,
		int nControlPointNumber,
		int nBufSize,										// # of output slots available
		ModelHitBoxInfo_t *pHitBoxOutputBuffer );

	virtual	bool IsPointInControllingObjectHitBox( 
		CParticleCollection *pParticles,
		int nControlPointNumber, Vector vecPos, bool bBBoxOnly );

	virtual Vector GetLocalPlayerPos( void );
	virtual void GetLocalPlayerEyeVectors( Vector *pForward, Vector *pRight = NULL, Vector *pUp = NULL );

	virtual float GetPixelVisibility( int *pQueryHandle, const Vector &vecOrigin, float flScale );
	virtual void SetUpLightingEnvironment( const Vector& pos );

	// Portal 2 particle hooks
	virtual int GetRayTraceEnvironmentFromName( const char *pszRtEnvName );
	virtual void *GetModel( char const *pMdlName );
	virtual int GetActivityNumber( void *pModel, const char *pszActivityName );
	virtual void BeginDrawModels(
	    int nNumModels, Vector const &vecCenter, CParticleCollection *pParticles );
	virtual void DrawModel( void *pModel, const matrix3x4_t &DrawMatrix,
	    CParticleCollection *pParticles, int nParticleNumber, int nBodyPart, int nSubModel,
	    int nSkin, int nAnimationSequence, float flAnimationRate, float r, float g, float b,
	    float a );
	virtual void DrawBlobs( IMatRenderContext *pRenderContext, IMaterial *pMaterial,
	    const Vector *pCenters, int nCount, float flCubeWidth, float flCutoffRadius,
	    float flRenderRadius );
};


static CParticleSystemQuery s_ParticleSystemQuery;
IParticleSystemQuery *g_pParticleSystemQuery = &s_ParticleSystemQuery;


//-----------------------------------------------------------------------------
// Exposes the interface (so tools can get at it)
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CParticleSystemQuery, IParticleSystemQuery, PARTICLE_SYSTEM_QUERY_INTERFACE_VERSION, s_ParticleSystemQuery );
#endif

static CThreadFastMutex s_LightMutex;
static CThreadFastMutex s_BoneMutex;

//-----------------------------------------------------------------------------
// Inherited from IParticleSystemQuery
//-----------------------------------------------------------------------------
void CParticleSystemQuery::GetLightingAtPoint( const Vector& vecOrigin, Color &cTint )
{
#ifdef GAME_DLL

	// FIXME: Go through to the engine from the server to get these values
	cTint.SetColor( 255, 255, 255, 255 );

#else

	if ( engine->IsInGame() )
	{
		s_LightMutex.Lock();
		// Compute our lighting at our position
		Vector totalColor = engine->GetLightForPoint( vecOrigin, true );
		s_LightMutex.Unlock();

		// Get our lighting information
		cTint.SetColor( totalColor.x*255, totalColor.y*255, totalColor.z*255, 0 );
	}
	else
	{
		// FIXME: Go through to the engine from the server to get these values
		cTint.SetColor( 255, 255, 255, 255 );
 	}

#endif
}

void CParticleSystemQuery::SetUpLightingEnvironment( const Vector& pos )
{
#ifndef GAME_DLL
	if ( !engine->IsInGame() )
		return;

	s_LightMutex.Lock();
	modelrender->SetupLighting( pos );
	s_LightMutex.Unlock();
#endif
}

void CParticleSystemQuery::TraceLine( const Vector& vecAbsStart,
									  const Vector& vecAbsEnd, unsigned int mask, 
									  const IHandleEntity *ignore,
									  int collisionGroup, CBaseTrace *ptr )
{
	bool bDoTrace = false;
#ifndef GAME_DLL
	bDoTrace = engine->IsInGame();
#endif
	if ( bDoTrace )
	{
		trace_t tempTrace;
		UTIL_TraceLine( vecAbsStart, vecAbsEnd, mask, ignore, collisionGroup, &tempTrace );
		memcpy( ptr, &tempTrace, sizeof ( CBaseTrace ) );
	}
	else
	{
		ptr->startsolid = 0;
		ptr->fraction = 1.0;
	}

}

bool CParticleSystemQuery::MovePointInsideControllingObject( 
	CParticleCollection *pParticles, void *pObject, Vector *pPnt )
{
#ifdef GAME_DLL
	return true;
#else
	if (! pObject )
		return true;										// accept the input point unmodified

	Ray_t ray;
	trace_t tr;
	ray.Init( *pPnt, *pPnt );
	enginetrace->ClipRayToEntity( ray, MASK_ALL, (CBaseEntity *) pObject, &tr );
	
	return ( tr.startsolid );
#endif
}

static float GetSurfaceCoord( float flRand, float flMinX, float flMaxX )
{
	return Lerp( flRand, flMinX, flMaxX );

}


void CParticleSystemQuery::GetRandomPointsOnControllingObjectHitBox( 
	CParticleCollection *pParticles,
	int nControlPointNumber, 
	int nNumPtsOut,
	float flBBoxScale,
	int nNumTrysToGetAPointInsideTheModel,
	Vector *pPntsOut,
	Vector vecDirectionalBias,
	Vector *pHitBoxRelativeCoordOut,
	int *pHitBoxIndexOut
	)
{

	bool bSucesss = false;


#ifndef GAME_DLL

	EHANDLE *phMoveParent = reinterpret_cast<EHANDLE *> ( pParticles->m_ControlPoints[nControlPointNumber].m_pObject );
	CBaseEntity *pMoveParent = NULL;
	if ( phMoveParent )
	{
		pMoveParent = *( phMoveParent );
	}
	if ( pMoveParent )
	{
		float flRandMax = flBBoxScale;
		float flRandMin = 1.0 - flBBoxScale;
		Vector vecBasePos;
		pParticles->GetControlPointAtTime( nControlPointNumber, pParticles->m_flCurTime, &vecBasePos );

		s_BoneMutex.Lock();
		C_BaseAnimating *pAnimating = pMoveParent->GetBaseAnimating();
		if ( pAnimating )
		{
			
			matrix3x4_t	*hitboxbones[MAXSTUDIOBONES];
			
			if ( pAnimating->HitboxToWorldTransforms( hitboxbones ) )
			{
		
				studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( pAnimating->GetModel() );
				
				if ( pStudioHdr )
				{
					mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( pAnimating->GetHitboxSet() );
					
					if ( set )
					{
						bSucesss = true;
						
						Vector vecWorldPosition;
						float u = 0, v = 0, w = 0;
						int nHitbox = 0;
						int nNumIters = nNumTrysToGetAPointInsideTheModel;
						if (! vecDirectionalBias.IsZero( 0.0001 ) )
							nNumIters = MAX( nNumIters, 5 );

						for( int i=0 ; i < nNumPtsOut; i++)
						{
							int nTryCnt = nNumIters;
							float flBestPointGoodness = -1.0e20;
							do
							{
								int nTryHitbox = pParticles->RandomInt( 0, set->numhitboxes - 1 );
								mstudiobbox_t *pBox = set->pHitbox(nTryHitbox);
								
								float flTryU = pParticles->RandomFloat( flRandMin, flRandMax );
								float flTryV = pParticles->RandomFloat( flRandMin, flRandMax );
								float flTryW = pParticles->RandomFloat( flRandMin, flRandMax );

								Vector vecLocalPosition;
								vecLocalPosition.x = GetSurfaceCoord( flTryU, pBox->bbmin.x * pAnimating->GetModelScale(), pBox->bbmax.x * pAnimating->GetModelScale() );
								vecLocalPosition.y = GetSurfaceCoord( flTryV, pBox->bbmin.y * pAnimating->GetModelScale(), pBox->bbmax.y * pAnimating->GetModelScale() );
								vecLocalPosition.z = GetSurfaceCoord( flTryW, pBox->bbmin.z * pAnimating->GetModelScale(), pBox->bbmax.z * pAnimating->GetModelScale() );

								Vector vecTryWorldPosition;

								VectorTransform( vecLocalPosition, *hitboxbones[pBox->bone], vecTryWorldPosition );
								
								
								float flPointGoodness = pParticles->RandomFloat( 0, 72 )
									+ DotProduct( vecTryWorldPosition - vecBasePos, 
												  vecDirectionalBias );

								if ( nNumTrysToGetAPointInsideTheModel )
								{
									// do a point in solid test
									Ray_t ray;
									trace_t tr;
									ray.Init( vecTryWorldPosition, vecTryWorldPosition );
									enginetrace->ClipRayToEntity( ray, MASK_ALL, pMoveParent, &tr );
									if ( tr.startsolid )
										flPointGoodness += 1000.; // got a point inside!
								}
								if ( flPointGoodness > flBestPointGoodness )
								{
									u = flTryU;
									v = flTryV;
									w = flTryW;
									vecWorldPosition = vecTryWorldPosition;
									nHitbox = nTryHitbox;
									flBestPointGoodness = flPointGoodness;
								}
							} while ( nTryCnt-- );
							*( pPntsOut++ ) = vecWorldPosition;
							if ( pHitBoxRelativeCoordOut )
								( pHitBoxRelativeCoordOut++ )->Init( u, v, w );
							if ( pHitBoxIndexOut )
								*( pHitBoxIndexOut++ ) = nHitbox;
						}
					}
				}
			}
		}

		if ( pMoveParent->IsBrushModel() )
		{
			Vector vecMin;
			Vector vecMax;
			matrix3x4_t matOrientation;
			Vector VecOrigin;
			pMoveParent->GetRenderBounds( vecMin, vecMax  );
			VecOrigin = pMoveParent->GetRenderOrigin();
			matOrientation = pMoveParent->EntityToWorldTransform();

			

			Vector vecWorldPosition;
			float u = 0, v = 0, w = 0;
			int nHitbox = 0;
			int nNumIters = nNumTrysToGetAPointInsideTheModel;
			if (! vecDirectionalBias.IsZero( 0.0001 ) )
				nNumIters = MAX( nNumIters, 5 );

			for( int i=0 ; i < nNumPtsOut; i++)
			{
				int nTryCnt = nNumIters;
				float flBestPointGoodness = -1.0e20;
				do
				{
					float flTryU = pParticles->RandomFloat( flRandMin, flRandMax );
					float flTryV = pParticles->RandomFloat( flRandMin, flRandMax );
					float flTryW = pParticles->RandomFloat( flRandMin, flRandMax );

					Vector vecLocalPosition;
					vecLocalPosition.x = GetSurfaceCoord( flTryU, vecMin.x, vecMax.x );
					vecLocalPosition.y = GetSurfaceCoord( flTryV, vecMin.y, vecMax.y );
					vecLocalPosition.z = GetSurfaceCoord( flTryW, vecMin.z, vecMax.z );

					Vector vecTryWorldPosition;
					VectorTransform( vecLocalPosition, matOrientation, vecTryWorldPosition );

					float flPointGoodness = pParticles->RandomFloat( 0, 72 )
						+ DotProduct( vecTryWorldPosition - vecBasePos, 
						vecDirectionalBias );

					if ( nNumTrysToGetAPointInsideTheModel )
					{
						// do a point in solid test
						Ray_t ray;
						trace_t tr;
						ray.Init( vecTryWorldPosition, vecTryWorldPosition );
						enginetrace->ClipRayToEntity( ray, MASK_ALL, pMoveParent, &tr );
						if ( tr.startsolid )
							flPointGoodness += 1000.; // got a point inside!
					}
					if ( flPointGoodness > flBestPointGoodness )
					{
						u = flTryU;
						v = flTryV;
						w = flTryW;
						vecWorldPosition = vecTryWorldPosition;
						nHitbox = 0;
						flBestPointGoodness = flPointGoodness;
					}
				} while ( nTryCnt-- );
				*( pPntsOut++ ) = vecWorldPosition;
				if ( pHitBoxRelativeCoordOut )
					( pHitBoxRelativeCoordOut++ )->Init( u, v, w );
				if ( pHitBoxIndexOut )
					*( pHitBoxIndexOut++ ) = nHitbox;
			}
		}

		s_BoneMutex.Unlock();
	}
#endif
	if (! bSucesss )
	{
		// don't have a model or am in editor or something - fill return with control point
		for( int i=0 ; i < nNumPtsOut; i++)
		{
			pPntsOut[i] = pParticles->m_ControlPoints[nControlPointNumber].m_Position; // fallback if anything goes wrong
			
			if ( pHitBoxIndexOut )
				pHitBoxIndexOut[i] = 0;
			
			if ( pHitBoxRelativeCoordOut )
				pHitBoxRelativeCoordOut[i].Init();
		}
	}
}


int CParticleSystemQuery::GetControllingObjectHitBoxInfo(
	CParticleCollection *pParticles,
	int nControlPointNumber,
	int nBufSize,										// # of output slots available
	ModelHitBoxInfo_t *pHitBoxOutputBuffer )
{
	int nRet = 0;

#ifndef GAME_DLL
	s_BoneMutex.Lock();

	EHANDLE *phMoveParent = reinterpret_cast<EHANDLE *> ( pParticles->m_ControlPoints[nControlPointNumber].m_pObject );
	CBaseEntity *pMoveParent = NULL;
	if ( phMoveParent )
	{
		pMoveParent = *( phMoveParent );
	}

	if ( pMoveParent )
	{
		C_BaseAnimating *pAnimating = pMoveParent->GetBaseAnimating();
		if ( pAnimating )
		{
			matrix3x4_t	*hitboxbones[MAXSTUDIOBONES];
			
			if ( pAnimating->HitboxToWorldTransforms( hitboxbones ) )
			{
		
				studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( pAnimating->GetModel() );
				
				if ( pStudioHdr )
				{
					mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( pAnimating->GetHitboxSet() );
					
					if ( set )
					{
						nRet = MIN( nBufSize, set->numhitboxes );
						for( int i=0 ; i < nRet; i++ )
						{
							mstudiobbox_t *pBox = set->pHitbox( i );
							pHitBoxOutputBuffer[i].m_vecBoxMins.x = pBox->bbmin.x;
							pHitBoxOutputBuffer[i].m_vecBoxMins.y = pBox->bbmin.y;
							pHitBoxOutputBuffer[i].m_vecBoxMins.z = pBox->bbmin.z;

							pHitBoxOutputBuffer[i].m_vecBoxMaxes.x = pBox->bbmax.x;
							pHitBoxOutputBuffer[i].m_vecBoxMaxes.y = pBox->bbmax.y;
							pHitBoxOutputBuffer[i].m_vecBoxMaxes.z = pBox->bbmax.z;

							pHitBoxOutputBuffer[i].m_Transform = *hitboxbones[pBox->bone];
						}
					}
				}
			}
		}
		if ( pMoveParent->IsBrushModel() )
		{
			Vector vecMin;
			Vector vecMax;
			matrix3x4_t matOrientation;
			pMoveParent->GetRenderBounds( vecMin, vecMax  );
			matOrientation = pMoveParent->EntityToWorldTransform();
			pHitBoxOutputBuffer[0].m_vecBoxMins = vecMin;
			pHitBoxOutputBuffer[0].m_vecBoxMaxes = vecMax;
			pHitBoxOutputBuffer[0].m_Transform = matOrientation;
			nRet = 1;
		}
	}
	s_BoneMutex.Unlock();
#endif
	return nRet;
}



bool CParticleSystemQuery::IsPointInControllingObjectHitBox( 
	CParticleCollection *pParticles,
	int nControlPointNumber, Vector vecPos, bool bBBoxOnly )
{
	bool bSuccess = false;
#ifndef GAME_DLL

	EHANDLE *phMoveParent = reinterpret_cast<EHANDLE *> ( pParticles->m_ControlPoints[nControlPointNumber].m_pObject );
	CBaseEntity *pMoveParent = NULL;
	if ( phMoveParent )
	{
		pMoveParent = *( phMoveParent );
	}
	if ( pMoveParent )
	{
		s_BoneMutex.Lock();
		C_BaseAnimating *pAnimating = pMoveParent->GetBaseAnimating();

		bool bInBBox = false;
		Vector vecBBoxMin;
		Vector vecBBoxMax;
		Vector vecOrigin;

		vecBBoxMin = pMoveParent->CollisionProp()->OBBMins();
		vecBBoxMax = pMoveParent->CollisionProp()->OBBMaxs();

		matrix3x4_t matOrientation;
		matOrientation = pMoveParent->EntityToWorldTransform();
		Vector vecLocalPos;
		VectorITransform( vecPos, matOrientation, vecLocalPos );
		if ( IsPointInBox( vecLocalPos, vecBBoxMin, vecBBoxMax ) )
			bInBBox = true;

		if ( bInBBox && bBBoxOnly )
			bSuccess = true;
		else if ( pAnimating && bInBBox )
		{
			matrix3x4_t	*hitboxbones[MAXSTUDIOBONES];
			if ( pAnimating->HitboxToWorldTransforms( hitboxbones ) )
			{

				studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( pAnimating->GetModel() );

				if ( pStudioHdr )
				{
					mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( pAnimating->GetHitboxSet() );

					if ( set )
					{
						// do a point in solid test
						Ray_t ray;
						trace_t tr;
						ray.Init( vecPos, vecPos );
						enginetrace->ClipRayToEntity( ray, MASK_ALL, pMoveParent, &tr );
						if ( tr.startsolid )
							bSuccess = true;
					}
				}
			}
		}
		else if ( pMoveParent->IsBrushModel() && bInBBox )
		{
			// do a point in solid test
			Ray_t ray;
			trace_t tr;
			ray.Init( vecPos, vecPos );
			enginetrace->ClipRayToEntity( ray, MASK_ALL, pMoveParent, &tr );
			if ( tr.startsolid )
				bSuccess = true;
		}

		s_BoneMutex.Unlock();
	}
#endif
	return bSuccess;
}


struct CollisionGroupNameRecord_t
{
	const char *m_pszGroupName;
	int m_nGroupID;
};


static CollisionGroupNameRecord_t s_NameMap[]={
	{ "NONE", COLLISION_GROUP_NONE },
	{ "DEBRIS", COLLISION_GROUP_DEBRIS },
	{ "INTERACTIVE", COLLISION_GROUP_INTERACTIVE },
	{ "NPC", COLLISION_GROUP_NPC },
	{ "ACTOR", COLLISION_GROUP_NPC_ACTOR },
	{ "PASSABLE", COLLISION_GROUP_PASSABLE_DOOR },	
#if defined( TF_CLIENT_DLL )
	{ "ROCKETS", TFCOLLISION_GROUP_ROCKETS },
#endif
};


int CParticleSystemQuery::GetCollisionGroupFromName( const char *pszCollisionGroupName )
{
	for(int i = 0; i < ARRAYSIZE( s_NameMap ); i++ )
	{
		if ( ! stricmp( s_NameMap[i].m_pszGroupName, pszCollisionGroupName ) )
			return s_NameMap[i].m_nGroupID;
	}
	return COLLISION_GROUP_NONE;
}

Vector CParticleSystemQuery::GetLocalPlayerPos( void )
{
#ifdef CLIENT_DLL
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return vec3_origin;
	return pPlayer->WorldSpaceCenter();
#else
	CBasePlayer *pPlayer = AI_GetSinglePlayer();	
	if ( !pPlayer )
		return vec3_origin;
	return pPlayer->WorldSpaceCenter();
#endif
}

void CParticleSystemQuery::GetLocalPlayerEyeVectors( Vector *pForward, Vector *pRight, Vector *pUp )
{
#ifdef CLIENT_DLL
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
	{
		*pForward = vec3_origin;
		*pRight = vec3_origin;
		*pUp = vec3_origin;
		return;
	}
	pPlayer->EyeVectors( pForward, pRight, pUp );
#else
	CBasePlayer *pPlayer = AI_GetSinglePlayer();	
	if ( !pPlayer )
	{
		*pForward = vec3_origin;
		*pRight = vec3_origin;
		*pUp = vec3_origin;
		return;
	}
	pPlayer->EyeVectors( pForward, pRight, pUp );
#endif
}

float CParticleSystemQuery::GetPixelVisibility( int *pQueryHandle, const Vector &vecOrigin, float flScale )
{
#ifdef CLIENT_DLL
	pixelvis_queryparams_t params;
	params.Init( vecOrigin, flScale, 1.0 );
	float flVisibility = PixelVisibility_FractionVisible( params, pQueryHandle );
	flVisibility = MAX( 0.0f, flVisibility );
	return flVisibility;
#else
	return 0.0f;
#endif
}

//-----------------------------------------------------------------------------
// Ray trace environments ("Cull relative to Ray Trace Environment"). The names
// match Portal 2 / CS:GO. This game builds no environments (those come from
// particle precipitation), so TraceAgainstRayTraceEnv keeps the interface
// default and every ray misses.
//-----------------------------------------------------------------------------
struct RayTraceEnvironmentNameRecord_t
{
	const char *m_pszGroupName;
	int m_nGroupID;
};

static RayTraceEnvironmentNameRecord_t s_RtEnvNameMap[] = {
    { "PRECIPITATION", 0 },
    { "PRECIPITATIONBLOCKER", 1 },
};

int CParticleSystemQuery::GetRayTraceEnvironmentFromName( const char *pszRtEnvName )
{
	for ( int i = 0; i < ARRAYSIZE( s_RtEnvNameMap ); i++ )
	{
		if ( !stricmp( s_RtEnvNameMap[i].m_pszGroupName, pszRtEnvName ) )
			return s_RtEnvNameMap[i].m_nGroupID;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Models drawn by the "Render models" particle renderer
//-----------------------------------------------------------------------------
void *CParticleSystemQuery::GetModel( char const *pMdlName )
{
#ifdef CLIENT_DLL
	if ( !pMdlName || !pMdlName[0] || !V_stricmp( pMdlName, "NONE" ) )
		return NULL;

	char szModelName[MAX_PATH];
	V_snprintf( szModelName, sizeof( szModelName ), "models/%s", pMdlName );
	V_FixSlashes( szModelName, '/' );
	return (void *)engine->LoadModel( szModelName );
#else
	return NULL;
#endif
}

int CParticleSystemQuery::GetActivityNumber( void *pModel, const char *pszActivityName )
{
#ifdef CLIENT_DLL
	const model_t *pMDL = (const model_t *)pModel;
	if ( pMDL )
	{
		studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( pMDL );
		if ( !pStudioHdr )
			return -1;

		CStudioHdr studioHdr( pStudioHdr, mdlcache );
		int nActivityNum = LookupActivity( &studioHdr, pszActivityName );
		return SelectWeightedSequence( &studioHdr, nActivityNum );
	}
#endif
	return -1;
}

void CParticleSystemQuery::BeginDrawModels(
    int nNumModels, Vector const &vecCenter, CParticleCollection *pParticles )
{
	// Light the models from the system's center
	SetUpLightingEnvironment( vecCenter );
}

#ifdef CLIENT_DLL
static void SetParticleModelBodygroup( studiohdr_t *pstudiohdr, int &body, int iGroup, int iValue )
{
	if ( !pstudiohdr )
		return;

	if ( iGroup >= pstudiohdr->numbodyparts )
		return;

	mstudiobodyparts_t *pbodypart = pstudiohdr->pBodypart( iGroup );

	if ( iValue >= pbodypart->nummodels )
		return;

	int iCurrent = ( body / pbodypart->base ) % pbodypart->nummodels;

	body = ( body - ( iCurrent * pbodypart->base ) + ( iValue * pbodypart->base ) );
}
#endif

void CParticleSystemQuery::DrawModel( void *pModel, const matrix3x4_t &DrawMatrix,
    CParticleCollection *pParticles, int nParticleNumber, int nBodyPart, int nSubModel, int nSkin,
    int nAnimationSequence, float flAnimationRate, float r, float g, float b, float a )
{
#ifdef CLIENT_DLL
	const model_t *pMDL = (const model_t *)pModel;
	if ( !pMDL )
		return;

	MDLHandle_t hStudioHdr = modelinfo->GetCacheHandle( pMDL );
	if ( hStudioHdr == MDLHANDLE_INVALID )
		return;

	studiohdr_t *pStudioHdr = mdlcache->GetStudioHdr( hStudioHdr );

	CMDL MDL;
	MDL.SetMDL( hStudioHdr );

	SetParticleModelBodygroup( pStudioHdr, MDL.m_nBody, nBodyPart, nSubModel );
	MDL.m_Color = Color( r * 255, g * 255, b * 255, a * 255 );

	MDL.m_nSkin = nSkin;
	MDL.m_nSequence = MAX( 0, nAnimationSequence );
	MDL.m_flPlaybackRate = flAnimationRate;
	MDL.m_flTime = pParticles->m_flCurTime;

	MDL.Draw( DrawMatrix );
#endif
}

//-----------------------------------------------------------------------------
// render_blobs: Portal 2 draws the isosurface with the paint blob tiler
//-----------------------------------------------------------------------------
void CParticleSystemQuery::DrawBlobs( IMatRenderContext *pRenderContext, IMaterial *pMaterial,
    const Vector *pCenters, int nCount, float flCubeWidth, float flCutoffRadius,
    float flRenderRadius )
{
#if defined( CLIENT_DLL ) && defined( PORTAL2 )
	if ( nCount <= 0 || !pRenderContext )
		return;

	ImpTiler *pTiler = ImpTilerFactory::factory->getTiler();
	pTiler->SetCubeWidth( flCubeWidth );
	pTiler->SetRenderRadius( flRenderRadius );
	pTiler->SetCutoffRadius( flCutoffRadius );
	pTiler->setTileIndexToDraw( -1 );
	pTiler->SetRenderContext( &pRenderContext );

	pTiler->beginFrame( Point3D( 0.0f, 0.0f, 0.0f ), true, false );
	for ( int i = 0; i < nCount; ++i )
	{
		ImpParticleWithOneInterpolant particle;
		particle.center = Point3D( pCenters[i] );
		particle.setFieldScale( 1.0f );
		particle.interpolants1 = Point3D( 1.0f, 1.0f, 1.0f );
		pTiler->insertParticle( &particle );
	}
	pTiler->drawSurface( false );
	pTiler->endFrame( false );

	pTiler->SetRenderContext( NULL );
	ImpTilerFactory::factory->returnTiler( pTiler );
#endif
}
