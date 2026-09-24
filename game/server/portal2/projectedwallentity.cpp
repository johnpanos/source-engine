//========= Portal 2 reconstruction ============================================//
//
// Purpose: Hard light bridge segment: builds the custom wall collideable,
//			networks its bounds and segment paint.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "projectedwallentity.h"
#include "info_placement_helper.h"
#include "paint_database.h"
#include "basetempentity.h"
#include "mathlib/polyhedron.h"
#include "physics_shared.h"
#include "vcollide_parse.h"
#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sv_thinnerprojectedwalls;

IMPLEMENT_AUTO_LIST( IProjectedWallEntityAutoList );

LINK_ENTITY_TO_CLASS( projected_wall_entity, CProjectedWallEntity );

BEGIN_DATADESC( CProjectedWallEntity )

	DEFINE_FIELD( m_vWorldSpace_WallMins, FIELD_VECTOR ),
	DEFINE_FIELD( m_vWorldSpace_WallMaxs, FIELD_VECTOR ),
	DEFINE_FIELD( m_flLength, FIELD_FLOAT ),
	DEFINE_FIELD( m_flWidth, FIELD_FLOAT ),
	DEFINE_FIELD( m_flHeight, FIELD_FLOAT ),
	DEFINE_FIELD( m_bIsHorizontal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flSegmentLength, FIELD_FLOAT ),
	DEFINE_FIELD( m_nNumSegments, FIELD_INTEGER ),

	// m_pWallCollideable is rebuilt by ProjectWall() when the projector re-projects on restore

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CProjectedWallEntity, DT_ProjectedWallEntity )

	SendPropVector( SENDINFO( m_vWorldSpace_WallMins ), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vWorldSpace_WallMaxs ), -1, SPROP_NOSCALE ),
	SendPropEHandle( SENDINFO( m_hColorPortal ) ),
	SendPropFloat( SENDINFO( m_flLength ), -1, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flWidth ), -1, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flHeight ), -1, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flSegmentLength ), -1, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flParticleUpdateTime ), -1, SPROP_NOSCALE ),
	SendPropInt( SENDINFO( m_nNumSegments ) ),
	SendPropBool( SENDINFO( m_bIsHorizontal ) ),

END_SEND_TABLE()


//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
CProjectedWallEntity::CProjectedWallEntity()
{
	m_flLength = 0.0f;
	m_flHeight = WALL_PROJECTOR_HEIGHT;
	m_flWidth = WALL_PROJECTOR_THICKNESS;
	m_bIsHorizontal = false;
	m_nNumSegments = 0;
	m_flSegmentLength = 64.0f;
	m_flParticleUpdateTime = 0.0f;
}

CProjectedWallEntity::~CProjectedWallEntity()
{
	CleanupWall();

	// Forget any paint recorded against this wall
	PaintDatabase.RemovePaintedWall( this, true );
}

void CProjectedWallEntity::UpdateOnRemove( void )
{
	CleanupWall();

	BaseClass::UpdateOnRemove();
}

void CProjectedWallEntity::Precache( void )
{
	PrecacheParticleSystem( "projected_wall_impact" );
}

void CProjectedWallEntity::Spawn( void )
{
	BaseClass::Spawn();

	Precache();

	// Collision comes from our own collideable through TestCollision()
	SetSolid( SOLID_CUSTOM );
	SetSolidFlags( FSOLID_CUSTOMRAYTEST | FSOLID_CUSTOMBOXTEST );
	CollisionProp()->SetSurroundingBoundsType( USE_GAME_CODE );
	SetMoveType( MOVETYPE_NONE );

	CreateVPhysics();

	SetTransmitState( FL_EDICT_ALWAYS );
}

void CProjectedWallEntity::OnRestore( void )
{
	BaseClass::OnRestore();

	SetTransmitState( FL_EDICT_ALWAYS );
}

//--------------------------------------------------------------------------------------------------
// Purpose: Build the wall's collideable and physics object between our projected endpoints
//--------------------------------------------------------------------------------------------------
void CProjectedWallEntity::ProjectWall( void )
{
	CleanupWall();

	AddEffects( EF_NOINTERP );

	// Nobody stands on the old wall any more
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer && pPlayer->GetGroundEntity() == this )
		{
			pPlayer->SetGroundEntity( NULL );
		}
	}

	Vector vWallStartPoint = GetStartPoint();
	Vector vWallEndPoint = GetEndPoint();

	// Get current orientation
	Vector vecForward, vecRight, vecUp;
	GetVectors( &vecForward, &vecRight, &vecUp );

	CPhysConvex *pTempConvex;
	if ( sv_thinnerprojectedwalls.GetBool() )
	{
		// A flat quad the width of the wall
		Vector vScaledRight = vecRight * ( WALL_PROJECTOR_THICKNESS * 0.5f );
		Vector vStart = vWallStartPoint;
		Vector vEnd = vWallEndPoint;

		Vector vVerts[4];
		vVerts[0] = vStart + vScaledRight;
		vVerts[1] = vStart - vScaledRight;
		vVerts[2] = vEnd - vScaledRight;
		vVerts[3] = vEnd + vScaledRight;

		Vector *pVerts[4] = { &vVerts[0], &vVerts[1], &vVerts[2], &vVerts[3] };
		pTempConvex = physcollision->ConvexFromVerts( pVerts, 4 );
	}
	else
	{
		// A box bounded by six planes around the projection
		float flWallPlanes[24];

		// Back
		( -vecForward ).CopyToArray( &flWallPlanes[0] );
		flWallPlanes[3] = DotProduct( -vecForward, vWallStartPoint );

		// Front
		vecForward.CopyToArray( &flWallPlanes[4] );
		flWallPlanes[7] = DotProduct( vecForward, vWallEndPoint );

		// Right
		Vector vWallPointRight = vWallStartPoint + ( vecRight * WALL_PROJECTOR_THICKNESS ) / 2.0f;
		vecRight.CopyToArray( &flWallPlanes[8] );
		flWallPlanes[11] = DotProduct( vWallPointRight, vecRight );

		// Left
		Vector vWallPointLeft = vWallStartPoint - ( vecRight * WALL_PROJECTOR_THICKNESS ) / 2.0f;
		( -vecRight ).CopyToArray( &flWallPlanes[12] );
		flWallPlanes[15] = DotProduct( vWallPointLeft, -vecRight );

		// Top
		Vector vWallPointUp = vWallStartPoint + ( vecUp * WALL_PROJECTOR_HEIGHT ) / 2.0f;
		vecUp.CopyToArray( &flWallPlanes[16] );
		flWallPlanes[19] = DotProduct( vWallPointUp, vecUp );

		// Bottom
		Vector vWallPointDown = vWallStartPoint - ( vecUp * WALL_PROJECTOR_HEIGHT ) / 2.0f;
		( -vecUp ).CopyToArray( &flWallPlanes[20] );
		flWallPlanes[23] = DotProduct( vWallPointDown, -vecUp );

		CPolyhedron *pTempPolyhedron = GeneratePolyhedronFromPlanes( flWallPlanes, 6, 0.0f );
		if ( pTempPolyhedron == NULL )
		{
			Warning( "CProjectedWallEntity: GeneratePolyhedronFromPlanes failed! Get a save game for me!.\n" );
			return;
		}

		pTempConvex = physcollision->ConvexFromConvexPolyhedron( *pTempPolyhedron );
		pTempPolyhedron->Release();

		if ( pTempConvex == NULL )
			return;
	}

	m_pWallCollideable = physcollision->ConvertConvexToCollide( &pTempConvex, 1 );
	if ( m_pWallCollideable == NULL )
		return;

	// Create the static physics object for the wall
	solid_t solid;
	Q_strncpy( solid.surfaceprop, "prop_wall_projector", sizeof( solid.surfaceprop ) );
	solid.params = g_PhysDefaultObjectParams;
	solid.params.pGameData = static_cast<void *>( this );

	IPhysicsObject *pWallObj = PhysModelCreateCustom( this, m_pWallCollideable, vec3_origin, vec3_angle, "prop_wall_projector", true, &solid );
	if ( pWallObj == NULL )
		return;

	if ( VPhysicsGetObject() )
	{
		VPhysicsDestroyObject();
	}
	VPhysicsSetObject( pWallObj );

	if ( pWallObj->GetCollide() == NULL )
		return;

	// The collideable is built in world space
	Vector vMins, vMaxs;
	physcollision->CollideGetAABB( &vMins, &vMaxs, pWallObj->GetCollide(), vec3_origin, vec3_angle );

	m_vWorldSpace_WallMins = vMins;
	m_vWorldSpace_WallMaxs = vMaxs;

	{
		Vector vSetMins = vMins - vWallStartPoint;
		Vector vSetMaxs = vMaxs - vWallStartPoint;
		SetSize( vSetMins, vSetMaxs );
	}

	m_flLength = ( vWallEndPoint - vWallStartPoint ).Length();

	m_flWidth = WALL_PROJECTOR_THICKNESS;
	m_flHeight = WALL_PROJECTOR_HEIGHT;

	CollisionProp()->MarkSurroundingBoundsDirty();
	CollisionProp()->MarkPartitionHandleDirty();
	CollisionProp()->UpdatePartition();

	// Walls facing mostly up or down can be walked on
	Vector vUp, vRight;
	AngleVectors( GetAbsAngles(), NULL, &vRight, &vUp );
	m_bIsHorizontal = ( vUp.z > 0.7f || vUp.z < -0.7f ) && ( vRight.z > -0.7f && vRight.z < 0.7f );

	DisplaceObstructingEntities();

	// Reset the paint segments for the new length
	m_nNumSegments = (int)ceil( m_flLength / m_flSegmentLength );
	m_PaintPowers.SetCount( m_nNumSegments );
	CleansePaint();
}

void CProjectedWallEntity::CleanupWall( void )
{
	if ( m_pWallCollideable )
	{
		physcollision->DestroyCollide( m_pWallCollideable );
		m_pWallCollideable = NULL;
	}

	if ( VPhysicsGetObject() )
	{
		VPhysicsDestroyObject();
	}

	m_vWorldSpace_WallMaxs = m_vWorldSpace_WallMins = vec3_origin;
	m_flLength = m_flWidth = m_flHeight = 0.0f;
	m_hHitPortal = NULL;
}

//--------------------------------------------------------------------------------------------------
// Trace the projection with a box half the width of the wall
//--------------------------------------------------------------------------------------------------
void CProjectedWallEntity::GetProjectionExtents( Vector &outMins, Vector &outMaxs )
{
	GetExtents( outMins, outMaxs, 0.5f );
}

bool CProjectedWallEntity::CreateVPhysics( void )
{
	ProjectWall();

	return true;
}

bool CProjectedWallEntity::TestCollision( const Ray_t &ray, unsigned int mask, trace_t &trace )
{
	if ( m_pWallCollideable == NULL )
		return false;

	physcollision->TraceBox( ray, mask, NULL, m_pWallCollideable, vec3_origin, vec3_angle, &trace );

	return trace.DidHit();
}

void CProjectedWallEntity::ComputeWorldSpaceSurroundingBox( Vector *pWorldMins, Vector *pWorldMaxs )
{
	Vector vMins = m_vWorldSpace_WallMins;
	Vector vMaxs = m_vWorldSpace_WallMaxs;

	*pWorldMins = vMins;
	*pWorldMaxs = vMaxs;
}

void CProjectedWallEntity::DrawWallBoundsHighlight( void )
{
	if ( m_pWallCollideable == NULL )
		return;

	Vector *outVerts;
	int vertCount = physcollision->CreateDebugMesh( m_pWallCollideable, &outVerts );
	int triCount = vertCount / 3;
	int vert = 0;
	for ( int i = 0; i < triCount; ++i )
	{
		NDebugOverlay::Triangle( outVerts[vert], outVerts[vert + 1], outVerts[vert + 2], 42, 82, 190, 128, false, 0.5f );
		vert += 3;
	}

	physcollision->DestroyDebugMesh( vertCount, outVerts );
}

void CProjectedWallEntity::NotifyPortalEvent( PortalEvent_t nEventType, CPortal_Base2D *pNotifier )
{
	// We only needed to hear about the portal opening
	if ( nEventType == PORTALEVENT_LINKED )
	{
		pNotifier->RemovePortalEventListener( EHANDLE( this ) );
	}
}

void CProjectedWallEntity::SetSourcePortal( CPortal_Base2D *pPortal )
{
	// Refresh the particles when the portal we come out of changes
	if ( ( pPortal == NULL ) ? ( GetSourcePortal() != NULL ) : DidRedirectionPortalMove( pPortal ) )
	{
		m_flParticleUpdateTime = gpGlobals->curtime + 0.5f;
	}

	m_hColorPortal = pPortal;

	BaseClass::SetSourcePortal( pPortal );
}

void CProjectedWallEntity::SetHitPortal( CPortal_Base2D *pPortal )
{
	// Refresh the particles when the portal we end in changes
	if ( ( pPortal == NULL ) ? ( GetHitPortal() != NULL ) : DidRedirectionPortalMove( pPortal ) )
	{
		m_flParticleUpdateTime = gpGlobals->curtime + 0.5f;
	}

	BaseClass::SetHitPortal( pPortal );
}


//--------------------------------------------------------------------------------------------------
// Tells clients a segment of a wall was painted
//--------------------------------------------------------------------------------------------------
class CTEWallPaintedEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEWallPaintedEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

	CTEWallPaintedEvent( const char *name ) : BaseClass( name )
	{
	}

	CNetworkHandle( CBaseEntity, m_hEntity );
	CNetworkVar( int, m_colorIndex );
	CNetworkVar( int, m_nSegment );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEWallPaintedEvent, DT_TEWallPaintedEvent )
	SendPropEHandle( SENDINFO( m_hEntity ) ),
	SendPropInt( SENDINFO( m_colorIndex ) ),
	SendPropInt( SENDINFO( m_nSegment ) ),
END_SEND_TABLE()

static CTEWallPaintedEvent g_TEWallPaintedEvent( "WallPaintedEvent" );

void WallPainted( int colorIndex, int nSegment, CBaseEntity *pWall )
{
	CRecipientFilter filter;
	filter.AddAllPlayers();

	g_TEWallPaintedEvent.m_hEntity = pWall;
	g_TEWallPaintedEvent.m_colorIndex = colorIndex;
	g_TEWallPaintedEvent.m_nSegment = nSegment;
	g_TEWallPaintedEvent.Create( filter, 0.0f );
}


bool CProjectedWallEntity::IsWallPainted( const Vector &vecPosition ) const
{
	return ( GetPaintPowerAtPoint( vecPosition ) != NO_POWER );
}


int CProjectedWallEntity::GetNumSegments( void ) const
{
	return m_nNumSegments;
}


float CProjectedWallEntity::GetSegmentLength( void ) const
{
	return m_flSegmentLength;
}


PaintPowerType CProjectedWallEntity::GetPaintPowerAtSegment( int i ) const
{
	return m_PaintPowers[i];
}


CProjectedWallEntity *CProjectedWallEntity::CreateNewInstance( void )
{
	return (CProjectedWallEntity *) CreateEntityByName( "projected_wall_entity" );
}


CBaseProjectedEntity *CProjectedWallEntity::CreateNewProjectedEntity( void )
{
	return CreateNewInstance();
}


//--------------------------------------------------------------------------------------------------
// Put the helper at the end of the bridge, raised to standing height on walkable bridges
//--------------------------------------------------------------------------------------------------
void CProjectedWallEntity::PlacePlacementHelper( CInfoPlacementHelper *pHelper )
{
	float flZOffset = m_bIsHorizontal ? 32.0f : 0.0f;
	Vector vecPosition = GetEndPoint() + Vector( 0.0f, 0.0f, flZOffset );

	if ( pHelper )
	{
		pHelper->SetAbsOrigin( vecPosition );
		pHelper->KeyValue( "radius", "52" );
		pHelper->KeyValue( "force_placement", "1" );
		pHelper->SetOwnerEntity( this );
	}
}


void CProjectedWallEntity::OnProjected( void )
{
	BaseClass::OnProjected();

	ProjectWall();

	m_flParticleUpdateTime = gpGlobals->curtime + 0.5f;
}
