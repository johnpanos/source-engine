//========= F-Stop port =======================================================//
//
// Purpose: Blobulator physics tiler (see public/blobulator/physics/PhysTiler.h).
//
// Clean-room implementation of the blobulator physics interface the F-Stop
// blob NPCs use. It is not Valve's blobulator physics library.
//
//=============================================================================//

#include "blobulator/physics/PhysTiler.h"

#include <algorithm>
#include <cmath>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Cell coordinates are clamped to [-CELL_LIMIT, CELL_LIMIT - 1] and packed in
// 21 bits each. Clamping keeps pairs within one cell of each other, so it only
// merges far cells and never loses a pair.
static const long long CELL_LIMIT = 1 << 20;
static const int CELL_BITS = 21;

// Cells are this much wider than the interaction radius. A pair the float
// distance test accepts is then at most one cell apart in every axis even
// after the float rounding of its squared distance (a few ulp, far below it).
static const double CELL_MARGIN = 1.0 + 1e-5;

static const float MIN_INTERACTION_RADIUS = 1e-3f;

namespace
{
long long CellCoordinate( float flValue, float flOrigin, double flCellWidth )
{
	// Floats are exact in double, so the subtraction is exact and only the
	// division rounds (relative 1e-16, inside CELL_MARGIN).
	double flCell = std::floor( ( double( flValue ) - double( flOrigin ) ) / flCellWidth );
	if ( !( flCell >= double( -CELL_LIMIT ) ) )		// also catches NaN
		return -CELL_LIMIT;
	if ( flCell > double( CELL_LIMIT - 1 ) )
		return CELL_LIMIT - 1;
	return (long long)flCell;
}

unsigned long long PackCell( long long x, long long y, long long z )
{
	return ( (unsigned long long)( x + CELL_LIMIT ) << ( 2 * CELL_BITS ) )
		| ( (unsigned long long)( y + CELL_LIMIT ) << CELL_BITS )
		| (unsigned long long)( z + CELL_LIMIT );
}

bool InCellRange( long long n )
{
	return n >= -CELL_LIMIT && n <= CELL_LIMIT - 1;
}
}

PhysTiler::PhysTiler( float flInteractionRadius )
{
	setInteractionRadius( flInteractionRadius );
}

void PhysTiler::setInteractionRadius( float flInteractionRadius )
{
	m_flInteractionRadius = ( flInteractionRadius > MIN_INTERACTION_RADIUS )
		? flInteractionRadius : MIN_INTERACTION_RADIUS;
}

void PhysTiler::beginFrame( const Point3D &origin )
{
	m_Origin = origin;
	m_Particles.size = 0;
	m_Cells.size = 0;
	m_Cache.m_Nodes.size = 0;
	m_Cache.m_Start.size = 0;
	m_Cache.m_Owners.size = 0;
}

void PhysTiler::insertParticle( PhysParticle *pParticle )
{
	if ( !pParticle )
		return;
	pParticle->tilerIndex = m_Particles.size;
	m_Particles.pushAutoSize( pParticle );
}

unsigned long long PhysTiler::CellKey( const Point3D &center, int nOffsetX, int nOffsetY, int nOffsetZ ) const
{
	double flCellWidth = double( m_flInteractionRadius ) * CELL_MARGIN;
	return PackCell( CellCoordinate( center[0], m_Origin[0], flCellWidth ) + nOffsetX,
		CellCoordinate( center[1], m_Origin[1], flCellWidth ) + nOffsetY,
		CellCoordinate( center[2], m_Origin[2], flCellWidth ) + nOffsetZ );
}

void PhysTiler::processTiles()
{
	const int nParticles = m_Particles.size;
	const double flCellWidth = double( m_flInteractionRadius ) * CELL_MARGIN;
	const float flRadiusSq = m_flInteractionRadius * m_flInteractionRadius;

	m_Cells.size = 0;
	m_Cells.ensureCapacity( nParticles );
	for ( int i = 0; i < nParticles; ++i )
	{
		CellEntry_t &entry = m_Cells.pushAutoSize();
		entry.key = CellKey( m_Particles[i]->center, 0, 0, 0 );
		entry.nParticle = i;
	}
	std::sort( m_Cells.a, m_Cells.a + m_Cells.size, []( const CellEntry_t &a, const CellEntry_t &b )
	{
		return a.key != b.key ? a.key < b.key : a.nParticle < b.nParticle;
	} );

	m_Cache.m_Nodes.size = 0;
	m_Cache.m_Start.size = 0;
	m_Cache.m_Owners.size = 0;
	m_Cache.m_Start.ensureCapacity( nParticles );
	m_Cache.m_Owners.ensureCapacity( nParticles );

	for ( int i = 0; i < nParticles; ++i )
	{
		const PhysParticle *pA = m_Particles[i];
		m_Cache.m_Start.pushAutoSize( m_Cache.m_Nodes.size );
		m_Cache.m_Owners.pushAutoSize( pA );

		long long cx = CellCoordinate( pA->center[0], m_Origin[0], flCellWidth );
		long long cy = CellCoordinate( pA->center[1], m_Origin[1], flCellWidth );
		long long cz = CellCoordinate( pA->center[2], m_Origin[2], flCellWidth );

		for ( int dx = -1; dx <= 1; ++dx )
		{
#ifdef BLOBULATOR_PHYS_SEED_DEFECT
			// Sensitivity build (blobulator.phys-tiler.sensitivity): ignore the
			// cells on one side, so pairs across that face are lost.
			if ( dx < 0 )
				continue;
#endif
			for ( int dy = -1; dy <= 1; ++dy )
			{
				for ( int dz = -1; dz <= 1; ++dz )
				{
					if ( !InCellRange( cx + dx ) || !InCellRange( cy + dy ) || !InCellRange( cz + dz ) )
						continue;
					CellEntry_t probe;
					probe.key = PackCell( cx + dx, cy + dy, cz + dz );
					probe.nParticle = -1;
					CellEntry_t *pEnd = m_Cells.a + m_Cells.size;
					CellEntry_t *pCell = std::lower_bound( m_Cells.a, pEnd, probe,
						[]( const CellEntry_t &a, const CellEntry_t &b )
						{
							return a.key != b.key ? a.key < b.key : a.nParticle < b.nParticle;
						} );
					for ( ; pCell != pEnd && pCell->key == probe.key; ++pCell )
					{
						if ( pCell->nParticle == i )
							continue;
						PhysParticle *pB = m_Particles[pCell->nParticle];
						float fx = pB->center[0] - pA->center[0];
						float fy = pB->center[1] - pA->center[1];
						float fz = pB->center[2] - pA->center[2];
						float flDistSq = fx * fx + fy * fy + fz * fz;
						if ( flDistSq < flRadiusSq )
						{
							PhysParticleAndDist &node = m_Cache.m_Nodes.pushAutoSize();
							node.particle = pB;
							node.distSq = flDistSq;
						}
					}
				}
			}
		}

		PhysParticleAndDist &end = m_Cache.m_Nodes.pushAutoSize();
		end.particle = NULL;
		end.distSq = 0.0f;
	}
}

void PhysTiler::endFrame()
{
	// The lists stay readable until the next beginFrame; only the particle
	// pointers are released here.
	m_Particles.size = 0;
	m_Cells.size = 0;
}

//-----------------------------------------------------------------------------
// Factory
//-----------------------------------------------------------------------------
static PhysTilerFactory s_PhysTilerFactory;
PhysTilerFactory *PhysTilerFactory::factory = &s_PhysTilerFactory;

PhysTilerFactory::~PhysTilerFactory()
{
	for ( int i = 0; i < m_Tilers.size; ++i )
		delete m_Tilers[i];
	m_Tilers.size = 0;
}

PhysTiler *PhysTilerFactory::getTiler()
{
	PhysTiler *pTiler;
	{
		AUTO_LOCK( m_Mutex );
		pTiler = ( m_Tilers.size > 0 ) ? m_Tilers.pop() : NULL;
	}
	if ( !pTiler )
		pTiler = new PhysTiler;
	pTiler->setInteractionRadius( 1.0f );
	return pTiler;
}

void PhysTilerFactory::returnTiler( PhysTiler *pTiler )
{
	if ( !pTiler )
		return;
	AUTO_LOCK( m_Mutex );
	m_Tilers.pushAutoSize( pTiler );
}
