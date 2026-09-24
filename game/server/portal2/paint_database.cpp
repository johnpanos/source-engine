//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server-side record of painted surfaces, entities and projected walls.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include <algorithm>
#include <functional>
#include "paint_database.h"
#include "paintable_entity.h"
#include "paint_color_manager.h"
#include "projectedwallentity.h"
#include "portal_util_shared.h"
#include "world.h"
#include "debugoverlay_shared.h"
#include "engine/IEngineTrace.h"
#include "isaverestore.h"
#include "bitvec.h"
#include "vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CPaintDatabase PaintDatabase( "CPaintDatabase" );

CPaintDatabase::CPaintDatabase( const char *name )
	: CAutoGameSystemPerFrame( name )
{
	ClearPaintData();
}


CPaintDatabase::~CPaintDatabase()
{
	ClearPaintData();
}


void CPaintDatabase::LevelInitPreEntity()
{
	ClearPaintData();
}


void CPaintDatabase::LevelInitPostEntity()
{
	m_bCanPaint = true;

	SendPaintDataToEngine();
}


void CPaintDatabase::LevelShutdownPostEntity()
{
	m_bCanPaint = false;
}


void CPaintDatabase::AddPaint( const trace_t& tr, PaintPowerType powerType )
{
	CBaseEntity *pPaintedEntity = tr.m_pEnt;
	Vector vecPaintLocation = tr.endpos;
	Vector vecNormal = tr.plane.normal;

	if ( pPaintedEntity->IsBSPModel() && UTIL_IsPaintableSurface( tr.surface ) )
	{
		PaintLocationData_t data;
		data.pBrushEntity = pPaintedEntity;
		data.location = vecPaintLocation + 0.1f * vecNormal;
		data.normal = vecNormal;
		data.type = powerType;

		m_PaintThisFrame.AddToTail( data );
	}
	else
	{
		// Reconstruction note: the 2010 binary tests the sign bit of m_fFlags (1<<31),
		// which is the flag the imported Portal 2 code names FL_UNPAINTABLE.
		if ( pPaintedEntity->GetFlags() & FL_UNPAINTABLE )
			return;

		CProjectedWallEntity *pPaintableWall = dynamic_cast< CProjectedWallEntity* >( pPaintedEntity );
		if ( pPaintableWall )
		{
			PaintProjectedWall( pPaintableWall, powerType, vecPaintLocation );
		}
		else
		{
			PaintEntity( pPaintedEntity, powerType, vecPaintLocation );
		}
	}
}


void CPaintDatabase::PaintEntity( CBaseEntity *pPaintedEntity, PaintPowerType newPowerType, const Vector &vecPosition )
{
	IPaintableEntity *pPaintableEntity = dynamic_cast< IPaintableEntity* >( pPaintedEntity );
	PaintPowerType paintedPowerType;

	if ( pPaintableEntity )
	{
		paintedPowerType = pPaintableEntity->GetPaintPowerAtPoint( vecPosition );

		// Players need to know right away so they can predict the new power
		if ( pPaintedEntity->IsPlayer() )
		{
			CBroadcastRecipientFilter filter;
			filter.MakeReliable();
			UserMessageBegin( filter, "PaintEntity" );
				WRITE_EHANDLE( pPaintedEntity );
				WRITE_BYTE( newPowerType );
				WRITE_FLOAT( vecPosition.x );
				WRITE_FLOAT( vecPosition.y );
				WRITE_FLOAT( vecPosition.z );
			MessageEnd();
		}
	}
	else
	{
		const color32 otherColor = pPaintedEntity->GetRenderColor();
		const Color paintedColor( otherColor.r, otherColor.g, otherColor.b );
		paintedPowerType = MapColorToPower( paintedColor );
	}

	const bool bPainted = paintedPowerType != NO_POWER;
	if ( bPainted )
	{
		// Erasing the paint on a painted entity
		if ( newPowerType == NO_POWER )
		{
			RemovePaintedEntity( pPaintedEntity );
		}
	}
	else if ( newPowerType != NO_POWER && FindPaintedEntity( pPaintedEntity ) == -1 )
	{
		PaintEntityData_t data;
		data.hEnt = pPaintedEntity;

		m_PaintedEntities.AddToTail( data );
	}

	if ( pPaintableEntity )
	{
		// Wake the physics object so it reacts to its new power
		IPhysicsObject *pPhysics = pPaintedEntity->VPhysicsGetObject();
		if ( pPhysics && pPhysics->IsAsleep() )
		{
			pPhysics->Wake();
		}

		pPaintableEntity->Paint( newPowerType, vecPosition );
	}
	else
	{
		Color newColor = MapPowerToVisualColor( newPowerType );
		pPaintedEntity->SetRenderColor( newColor.r(), newColor.g(), newColor.b() );
	}
}


void CPaintDatabase::PaintProjectedWall( CProjectedWallEntity *pWall, PaintPowerType powerType, const Vector &vecPosition )
{
	const bool bWallPainted = pWall->IsWallPainted( vecPosition );

	pWall->Paint( powerType, vecPosition );

	if ( !bWallPainted && powerType != NO_POWER )
	{
		if ( m_PaintedProjectedWalls.Find( pWall ) == m_PaintedProjectedWalls.InvalidIndex() )
		{
			m_PaintedProjectedWalls.AddToTail( pWall );
		}
	}
}


void CPaintDatabase::RemoveAllPaint()
{
	// Cleanse all the painted entities; the list itself is cleared below
	const int nEntityCount = m_PaintedEntities.Count();
	for ( int i = 0; i < nEntityCount; ++i )
	{
		CBaseEntity *pEnt = EntityFromEntityHandle( m_PaintedEntities[i].hEnt.Get() );
		if ( pEnt )
		{
			RemovePaintedEntity( i, false );
		}
	}

	// Reconstruction note: both builds declare nPaintedProjectedWallCount here but emit no
	// code for it; painted projected walls are only forgotten by ClearPaintData().
	const int nPaintedProjectedWallCount = m_PaintedProjectedWalls.Count();
	NOTE_UNUSED( nPaintedProjectedWallCount );

	ClearPaintData();

	// This engine has no BSP paintmap service; entity paint is cleared above.

	CBroadcastRecipientFilter filter;
	filter.MakeReliable();
	UserMessageBegin( filter, "RemoveAllPaint" );
	MessageEnd();
}


void CPaintDatabase::RemovePaintedEntity( const CBaseEntity *pPaintedEntity )
{
	const int index = FindPaintedEntity( pPaintedEntity );
	if ( index != -1 )
	{
		RemovePaintedEntity( index, true );
	}
}


void CPaintDatabase::RemovePaintedEntity( const CBaseEntity *pPaintedEntity, bool bDeleteData )
{
	const int index = FindPaintedEntity( pPaintedEntity );
	if ( index != -1 )
	{
		RemovePaintedEntity( index, bDeleteData );
	}
}


void CPaintDatabase::RemovePaintedEntity( int index, bool bDeleteData )
{
	CBaseEntity *pEnt = EntityFromEntityHandle( m_PaintedEntities[index].hEnt.Get() );
	if ( pEnt )
	{
		IPaintableEntity *pPaintableEntity = dynamic_cast< IPaintableEntity* >( pEnt );
		if ( pPaintableEntity )
		{
			pPaintableEntity->CleansePaint();
		}
		else
		{
			pEnt->SetRenderColor( 255, 255, 255 );
		}
	}

	if ( bDeleteData )
	{
		m_PaintedEntities.Remove( index );
	}
}


void CPaintDatabase::RemovePaintedWall( CProjectedWallEntity *pWall, bool bDeleteData )
{
	if ( bDeleteData )
	{
		const int index = m_PaintedProjectedWalls.Find( pWall );
		if ( index != m_PaintedProjectedWalls.InvalidIndex() )
		{
			m_PaintedProjectedWalls.Remove( index );
		}
	}
}


int CPaintDatabase::FindPaintedEntity( const CBaseEntity *pPaintedEntity ) const
{
	const int nPaintCount = m_PaintedEntities.Count();
	for ( int i = 0; i < nPaintCount; ++i )
	{
		CBaseEntity *pEnt = EntityFromEntityHandle( m_PaintedEntities[i].hEnt.Get() );
		if ( pEnt == pPaintedEntity )
		{
			return i;
		}
	}

	return -1;
}


void CPaintDatabase::FireGameEvent( IGameEvent *event )
{
}


void CPaintDatabase::ClearPaintData()
{
	m_PaintThisFrame.RemoveAll();
	m_PaintedEntities.RemoveAll();
	m_PaintedProjectedWalls.RemoveAll();
}


int AscendingPaintLocationCompare( const PaintLocationData_t *a, const PaintLocationData_t *b )
{
	return a->pBrushEntity >= b->pBrushEntity && a->type >= b->type;
}


ConVar paint_location_distance_threshold_square( "paint_location_distance_threshold_square", "25", 0 );

static void RemoveRepeatedPaintData( PaintLocationVector_t &dataList )
{
	for ( int i = 0; i < dataList.Count(); ++i )
	{
		const PaintLocationData_t data = dataList[i];

		int j = i + 1;
		while ( j < dataList.Count() )
		{
			const PaintLocationData_t &other = dataList[j];
			const Vector delta = data.location - other.location;
			if ( data.type == other.type &&
				 delta.LengthSqr() < paint_location_distance_threshold_square.GetFloat() )
			{
				dataList.FastRemove( j );
			}
			else
			{
				++j;
			}
		}
	}
}


// Predicate: true for the first paint location that doesn't belong in the same
// PaintWorld message as the given brush entity and power.
struct PaingDatabase_PreClientUpdate_DifferentType_t : public std::unary_function< const PaintLocationData_t&, bool >
{
	PaintPowerType m_PowerType;
	CBaseEntity *m_pBrushEntity;

	bool operator()( const PaintLocationData_t& data ) const
	{
		return data.pBrushEntity != m_pBrushEntity || data.type != m_PowerType;
	}

	PaingDatabase_PreClientUpdate_DifferentType_t( CBaseEntity *pBrushEntity, PaintPowerType powerType )
		: m_PowerType( powerType ), m_pBrushEntity( pBrushEntity )
	{
	}
};


// Collects (and merges) the boxes of world space that changed paint this frame.
struct CChangedPaintBoundsCache
{
	struct Bounds_t
	{
		Vector m_vecMins;
		Vector m_vecMaxs;
	};

	void AddChangedBounds( const Vector &vecCenter, float flRadius )
	{
		const Vector vecExtents( flRadius, flRadius, flRadius );
		const Vector vecMins = vecCenter - vecExtents;
		const Vector vecMaxs = vecCenter + vecExtents;

		// Grow an existing box if the result stays small enough
		for ( int i = 0; i < m_ChangedBoxes.Count(); ++i )
		{
			Bounds_t &b = m_ChangedBoxes[i];

			Vector vecNewMins, vecNewMaxs;
			VectorMin( b.m_vecMins, vecMins, vecNewMins );
			VectorMax( b.m_vecMaxs, vecMaxs, vecNewMaxs );

			const Vector delta = vecNewMins - vecNewMaxs;
			if ( delta.LengthSqr() < 3.0f * 256.0f * 256.0f )
			{
				b.m_vecMins = vecNewMins;
				b.m_vecMaxs = vecNewMaxs;
				return;
			}
		}

		Bounds_t newBounds;
		newBounds.m_vecMins = vecMins;
		newBounds.m_vecMaxs = vecMaxs;
		m_ChangedBoxes.AddToTail( newBounds );
	}

	CUtlVectorFixedGrowable< Bounds_t, 16 > m_ChangedBoxes;
};


// Find each entity touching changed paint once. Entity flags have no spare bit
// in this profile, so keep the per-update membership in this temporary list.
struct CPaintAffectedEntityList : public IEntityEnumerator
{
	virtual bool EnumEntity( IHandleEntity *pHandleEntity )
	{
		if ( !pHandleEntity )
			return true;

		CBaseEntity *pEntity = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
		if ( !pEntity )
			return true;

		if ( m_EntitiesToUpdate.Find( pEntity ) == m_EntitiesToUpdate.InvalidIndex() )
			m_EntitiesToUpdate.AddToTail( pEntity );

		return true;
	}

	CUtlVectorFixedGrowable< CBaseEntity*, 32 > m_EntitiesToUpdate;
};


// Reconstruction note: the 2010 UTIL_PaintBrushEntity( pBrushEntity, contactPoint, power )
// took its sphere radius and coat alpha from these portal_util_shared ConVars; the
// imported helper takes both as arguments, so the paint database supplies them.
static ConVar sv_paint_surface_sphere_radius( "sv_paint_surface_sphere_radius", "60.f", FCVAR_REPLICATED, "Radius of query sphere to paint all intersecting surfaces." );
static ConVar sv_erase_surface_sphere_radius( "sv_erase_surface_sphere_radius", "80.f", FCVAR_REPLICATED, "Radius of query sphere to erase all intersecting surfaces." );
static ConVar sv_paint_alpha_coat( "sv_paint_alpha_coat", "0.4f", FCVAR_REPLICATED | FCVAR_CHEAT );

void CPaintDatabase::PreClientUpdate()
{
	VPROF_BUDGET( "CPaintDatabase::PreClientUpdate", "paint" );

	if ( !m_bCanPaint )
		return;

	int count = m_PaintThisFrame.Count();

	if ( count > 1 )
	{
		RemoveRepeatedPaintData( m_PaintThisFrame );
		count = m_PaintThisFrame.Count();
	}

	// Paint the surfaces and remember where paint changed
	CChangedPaintBoundsCache boundsCache;
	for ( int i = 0; i < count; ++i )
	{
		PaintLocationData_t data = m_PaintThisFrame[i];

		const float flPaintRadius = ( data.type == NO_POWER ) ? sv_erase_surface_sphere_radius.GetFloat() : sv_paint_surface_sphere_radius.GetFloat();
		float flChangedPaintRadius = UTIL_PaintBrushEntity( data.pBrushEntity, data.location, data.type, flPaintRadius, sv_paint_alpha_coat.GetFloat() );
		if ( flChangedPaintRadius != 0.0f )
		{
			boundsCache.AddChangedBounds( data.location, flChangedPaintRadius );
		}
	}

	// Let everything touching the changed paint update its powers
	CPaintAffectedEntityList paintEnum;
	for ( int i = 0; i < boundsCache.m_ChangedBoxes.Count(); ++i )
	{
		enginetrace->EnumerateEntities( boundsCache.m_ChangedBoxes[i].m_vecMins, boundsCache.m_ChangedBoxes[i].m_vecMaxs, &paintEnum );
	}

	for ( int i = 0; i < paintEnum.m_EntitiesToUpdate.Count(); ++i )
	{
		CBaseEntity *pEntity = paintEnum.m_EntitiesToUpdate[i];
		pEntity->UpdatePaintPowersFromContacts();
	}

	// The listen server host shares the engine's paintmap; send the paint to the other player
	if ( gpGlobals->maxClients > 1 )
	{
		CBasePlayer *pOtherPlayer = UTIL_PlayerByIndex( 2 );
		if ( !pOtherPlayer )
		{
			m_bSentClientPaintData = false;
			m_PaintThisFrame.RemoveAll();
			return;
		}

		if ( !m_bSentClientPaintData && pOtherPlayer->IsConnected() )
		{
			SendPaintDataTo( pOtherPlayer );
		}

		m_bSentClientPaintData = true;

		// Group the paint by brush entity and power
		m_PaintThisFrame.Sort( AscendingPaintLocationCompare );

		CSingleUserRecipientFilter filter( pOtherPlayer );
		filter.MakeReliable();

		PaintLocationConstIter cbegin = m_PaintThisFrame.Base();
		PaintLocationConstIter cend = cbegin + m_PaintThisFrame.Count();

		for ( PaintLocationConstIter currentLocation = cbegin; currentLocation != cend; )
		{
			UserMessageBegin( filter, "PaintWorld" );

			PaintPowerType powerType = currentLocation->type;
			WRITE_BYTE( powerType );

			CBaseEntity *pBrushEntity = currentLocation->pBrushEntity;
			WRITE_EHANDLE( pBrushEntity );

			// Reconstruction note: only the resulting limit (40 locations per message) is
			// in the binary; the header term is the one that yields it from MAX_USER_MSG_DATA.
			const int SIZEOF_FLOAT_PAINT_POSITION = 3 * sizeof( float );
			const int SIZEOF_SHORT_PAINT_POSITION = 3 * sizeof( short );
			const int SIZEOF_PAINT_ENTITY_MSG_HEADER = sizeof( BYTE ) + sizeof( BYTE ) + SIZEOF_FLOAT_PAINT_POSITION;
			const int SIZE_OF_PAINT_ENTITY_MSG_BODY = MAX_USER_MSG_DATA - SIZEOF_PAINT_ENTITY_MSG_HEADER;

			PaintLocationConstIter firstDifferent = std::find_if( currentLocation, cend, PaingDatabase_PreClientUpdate_DifferentType_t( pBrushEntity, powerType ) );
			const int paintLocationCount = MIN( firstDifferent - currentLocation, SIZE_OF_PAINT_ENTITY_MSG_BODY / SIZEOF_SHORT_PAINT_POSITION );
			WRITE_BYTE( paintLocationCount );

			// Positions are sent as short offsets from their center
			Vector center( 0.0f, 0.0f, 0.0f );
			for ( int j = 0; j < paintLocationCount; ++j )
			{
				center += currentLocation[j].location;
			}

			center *= 1.0f / paintLocationCount;
			WRITE_FLOAT( center.x );
			WRITE_FLOAT( center.y );
			WRITE_FLOAT( center.z );

			for ( int j = 0; j < paintLocationCount; ++j, ++currentLocation )
			{
				const Vector offset = currentLocation->location - center;
				WRITE_SHORT( offset.x );
				WRITE_SHORT( offset.y );
				WRITE_SHORT( offset.z );
			}

			MessageEnd();
		}
	}

	m_PaintThisFrame.RemoveAll();
}


void CC_RemoveAllPaint()
{
	PaintDatabase.RemoveAllPaint();
}

static ConCommand removeallpaint( "removeallpaint", CC_RemoveAllPaint );


void CC_PaintAt( const CCommand &args )
{
	if ( args.ArgC() < 5 )
	{
		DevMsg( "paintat painttype[0:bounce, 1:stick, 2:speed] pos[x y z]\n" );
		return;
	}

	PaintPowerType power = static_cast< PaintPowerType >( atoi( args[1] ) );

	Vector pos( atof( args[2] ), atof( args[3] ), atof( args[4] ) );

	trace_t tr;
	tr.m_pEnt = GetWorldEntity();
	tr.endpos = pos;
	tr.plane.normal = Vector( 0.0f, 0.0f, 0.0f );
	PaintDatabase.AddPaint( tr, power );

	if ( args.ArgC() == 6 )
	{
		NDebugOverlay::Sphere( pos, 2.0f, 255, 255, 0, true, atof( args[5] ) );
	}
}

static ConCommand paintat( "paintat", CC_PaintAt );


template< typename T >
void RLEEncodeSave( ISave *pSave, T *pArray, int count )
{
	int i = 0;
	while ( i < count )
	{
		int length = 1;
		T val = pArray[i];
		++i;

		while ( i < count && pArray[i] == val )
		{
			++length;
			++i;
		}

		pSave->WriteInt( &length );
		pSave->WriteData( reinterpret_cast< const char* >( &val ), sizeof( T ) );
	}
}


void CPaintDatabase::SavePaintmapData( ISave *pSave )
{
	CUtlVector< CUtlVector< uint8 > > data;
	// No BSP paintmap data is available from this engine.
	int nPaintData = data.Count();
	pSave->WriteInt( &nPaintData );

	for ( int i = 0; i < nPaintData; ++i )
	{
		pSave->StartBlock();

		int count = data[i].Count();
		pSave->WriteInt( &count );
		RLEEncodeSave( pSave, data[i].Base(), count );

		pSave->EndBlock();
	}

	CVarBitVec paintSurfBits;
	// An empty bitset records that the engine has no painted BSP surfaces.

	pSave->StartBlock();
	int numBits = paintSurfBits.GetNumBits();
	pSave->WriteInt( &numBits );
	pSave->WriteInt( reinterpret_cast< int* >( paintSurfBits.Base() ), paintSurfBits.GetNumDWords() );
	pSave->EndBlock();
}


template< typename T >
void RLEDecodeRestore( IRestore *pRestore, T *pArray, int count )
{
	int total = 0;
	while ( total < count )
	{
		int length = pRestore->ReadInt();

		T val;
		pRestore->ReadData( reinterpret_cast< char* >( &val ), sizeof( T ), 0 );

		V_memset( pArray + total, val, length * sizeof( T ) );
		total += length;
	}
}


void CPaintDatabase::RestorePaintmapData( IRestore *pRestore )
{
	int nPaintData = pRestore->ReadInt();
	m_Paintmaps.SetCount( nPaintData );
	DevMsg( "Restoring %d paintmaps\n", nPaintData );

	for ( int i = 0; i < nPaintData; ++i )
	{
		pRestore->StartBlock();

		int count = pRestore->ReadInt();
		m_Paintmaps[i].SetSize( count );
		RLEDecodeRestore( pRestore, m_Paintmaps[i].Base(), count );

		pRestore->EndBlock();
	}

	CVarBitVec paintSurfBits;

	pRestore->StartBlock();
	{
		int numBits = pRestore->ReadInt();
		paintSurfBits.Resize( numBits );

		int numIntsInStream = CalcNumIntsForBits( numBits );
		int readSize = MIN( paintSurfBits.GetNumDWords(), numIntsInStream );
		pRestore->ReadInt( reinterpret_cast< int* >( paintSurfBits.Base() ), numIntsInStream );

		numIntsInStream -= readSize;
		while ( numIntsInStream-- > 0 )
		{
			int ignored;
			pRestore->ReadInt( &ignored, 1 );
		}
	}
	pRestore->EndBlock();

	// The engine has no BSP paintmap service to restore these bits into.
}


void CPaintDatabase::SendPaintDataTo( CBasePlayer *pPlayer )
{
	if ( pPlayer->IsConnected() )
	{
		CUtlVector< CUtlVector< uint8 > > data;
		// No BSP paintmap data is available to send to the client.

		CSingleUserRecipientFilter filter( pPlayer );
		filter.MakeReliable();

		CUtlVector< float > msgLength;
		CUtlVector< uint8 > msgVal;

		// Each message holds a paintmap id and offset followed by as many RLE runs as fit
		const int SIZEOF_PAINT_ID = sizeof( BYTE );
		const int SIZEOF_PAINT_OFFSET = sizeof( float );
		const int SIZEOF_RLE_DATA = sizeof( float ) + sizeof( uint8 );
		int MAX_RLE = ( MAX_USER_MSG_DATA - SIZEOF_PAINT_ID - SIZEOF_PAINT_OFFSET ) / SIZEOF_RLE_DATA;

		int nPaintmap = data.Count();
		for ( int n = 0; n < nPaintmap; ++n )
		{
			int count = data[n].Count();
			int total = 0;

			int nRLE = 0;
			int nOffset = 0;
			for ( int i = 0; i < count; )
			{
				int length = 1;
				uint8 val = data[n][i];
				++i;

				while ( i < count && data[n][i] == val )
				{
					++length;
					++i;
				}

				msgLength.AddToTail( length );
				msgVal.AddToTail( val );

				total += length;
				++nRLE;

				if ( nRLE == MAX_RLE || total == count )
				{
					UserMessageBegin( filter, "LoadPaintmapData" );
						WRITE_BYTE( n );
						WRITE_FLOAT( nOffset );
						WRITE_FLOAT( nRLE );
						for ( int m = 0; m < nRLE; ++m )
						{
							WRITE_FLOAT( msgLength[m] );
							WRITE_BYTE( msgVal[m] );
						}
					MessageEnd();

					msgVal.RemoveAll();
					msgLength.RemoveAll();
					nRLE = 0;
					nOffset = total;
				}
			}
		}

		CVarBitVec paintSurfBits;
		// Send an empty surface bitset for this engine profile.
		int numBits = paintSurfBits.GetNumBits();

		UserMessageBegin( filter, "LoadPaintmapBits" );
			WRITE_FLOAT( numBits );
			for ( int i = 0; i < paintSurfBits.GetNumDWords(); ++i )
			{
				WRITE_FLOAT( paintSurfBits.Base()[i] );
			}
		MessageEnd();
	}
}


void CPaintDatabase::SendPaintDataToEngine()
{
	for ( int i = 0; i < m_Paintmaps.Count(); ++i )
	{
		// BSP paintmap loading is unavailable in this engine profile.
	}

	m_Paintmaps.Purge();
}


void CC_PaintAllSurfaces( const CCommand &args )
{
	PaintPowerType power = ( args.ArgC() == 2 ) ? static_cast< PaintPowerType >( atoi( args[1] ) ) : SPEED_POWER;

	// BSP paintmaps are unavailable in this engine profile.

	CBroadcastRecipientFilter filter;
	filter.MakeReliable();
	UserMessageBegin( filter, "PaintAllSurfaces" );
		WRITE_BYTE( power );
	MessageEnd();
}

static ConCommand paintallsurfaces( "paintallsurfaces", CC_PaintAllSurfaces );
