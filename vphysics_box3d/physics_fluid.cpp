//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D fluid controllers and springs (RFC 0004 D1/C4). See
//          physics_fluid.h for the IVP semantics reproduced here.
//
//=============================================================================//
#include "physics_fluid.h"

#include <math.h>
#include <string.h>

#include "box3d/box3d.h"
#include "box3d_convert.h"
#include "physics_collision.h"
#include "physics_environment.h"
#include "physics_material.h"
#include "physics_object.h"
#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const float kMetersPerInch = 0.0254f;
const int kMaxClip = 8;

// Clips a polygon to the half space dot(n, p) <= d.
int ClipBelow( const Vector *pIn, int count, const Vector &normal, float dist, Vector *pOut )
{
	int out = 0;
	for ( int i = 0; i < count; i++ )
	{
		const Vector &a = pIn[i];
		const Vector &b = pIn[( i + 1 ) % count];
		float da = DotProduct( normal, a ) - dist;
		float db = DotProduct( normal, b ) - dist;
		if ( da <= 0 && out < kMaxClip )
			pOut[out++] = a;
		if ( ( da < 0 && db > 0 ) || ( da > 0 && db < 0 ) )
		{
			if ( out < kMaxClip )
				pOut[out++] = a + ( b - a ) * ( da / ( da - db ) );
		}
	}
	return out;
}

struct SubmergedPart_t
{
	float volume;			// cubic inches
	Vector volumeMoment;	// volume-weighted centroid sum
	Vector dragImpulse;		// Source impulse (kg*in/s) this step
	Vector dragAngular;		// sum of r x impulse about the body center
};
}

//-----------------------------------------------------------------------------
// Fluid controller
//-----------------------------------------------------------------------------
CPhysicsFluidControllerBox3D::CPhysicsFluidControllerBox3D( CPhysicsObjectBox3D *pFluidObject, const fluidparams_t &params )
	: m_pObject( pFluidObject ), m_pGameData( params.pGameData ), m_params( params ), m_contents( params.contents )
{
	// As IVP's CreateFluidController: the fluid object becomes a trigger and
	// its surface property supplies the density.
	pFluidObject->BecomeTrigger();
	pFluidObject->SetFluidController( this );
	surfacedata_t *pSurface = g_SurfaceDatabase.GetSurfaceData( pFluidObject->GetMaterialIndex() );
	m_density = pSurface ? pSurface->physics.density : 1000.0f;
	m_damping = params.damping;

	cplane_t worldPlane;
	worldPlane.normal = params.surfacePlane.AsVector3D();
	worldPlane.dist = params.surfacePlane[3];
	matrix3x4_t objectToWorld;
	pFluidObject->GetPositionMatrix( &objectToWorld );
	MatrixITransformPlane( objectToWorld, worldPlane, m_objectSpacePlane );
	VectorIRotate( params.currentVelocity, objectToWorld, m_objectSpaceCurrent );
}

CPhysicsFluidControllerBox3D::~CPhysicsFluidControllerBox3D()
{
	if ( m_pObject && m_pObject->GetFluidController() == this )
		m_pObject->SetFluidController( NULL );
}

void CPhysicsFluidControllerBox3D::GetWorldSurface( Vector *pNormal, float *pDist, Vector *pCurrent ) const
{
	matrix3x4_t objectToWorld;
	m_pObject->GetPositionMatrix( &objectToWorld );
	cplane_t worldPlane;
	MatrixTransformPlane( objectToWorld, m_objectSpacePlane, worldPlane );
	if ( pNormal )
		*pNormal = worldPlane.normal;
	if ( pDist )
		*pDist = worldPlane.dist;
	if ( pCurrent )
		VectorRotate( m_objectSpaceCurrent, objectToWorld, *pCurrent );
}

void CPhysicsFluidControllerBox3D::GetSurfacePlane( Vector *pNormal, float *pDist ) const
{
	GetWorldSurface( pNormal, pDist, NULL );
}

void CPhysicsFluidControllerBox3D::WakeAllSleepingObjects()
{
	m_pObject->GetEnvironment()->WakeTriggerOverlaps( m_pObject );
}

void CPhysicsFluidControllerBox3D::ApplyToObject( CPhysicsObjectBox3D *pObject, float dt )
{
	if ( !pObject->IsMoveable() || pObject->IsAsleep() || dt <= 0 )
		return;
	// IVP's CBuoyancyAttacher: no fluid simulation for shadows or objects
	// that opted out (their medium density is zero, which also zeroes the
	// pressure damping).
	if ( pObject->GetShadowController() || !( pObject->GetCallbackFlags() & CALLBACK_DO_FLUID_SIMULATION ) )
		return;
	float mediumDensity = m_density * pObject->GetBuoyancyRatio();
	if ( mediumDensity <= 0 )
		return;
	// IVP_Buoyancy_Solver: pressure dampening uses the authored damping,
	// friction dampening the template default (0.05); both scale with half
	// the medium density.
	float pressureDamp = m_damping * 0.5f * mediumDensity;
	float frictionDamp = 0.05f * 0.5f * mediumDensity;

	Vector normal, current;
	float dist;
	GetWorldSurface( &normal, &dist, &current );
	VectorNormalize( normal );

	b3BodyId body = pObject->GetBody();
	Vector bodyCenter = FromB3( b3Body_GetWorldCenter( body ) );
	SubmergedPart_t part;
	memset( &part, 0, sizeof( part ) );
	const Vector planePoint = normal * dist;

	// Damping on one submerged surface (square inches: the part under the
	// surface, and the whole triangle for friction; outward unit normal;
	// centroid): IVP's per-triangle pressure and friction dampening.
	auto addDrag = [&]( float area, float frictionArea, const Vector &surfaceNormal, const Vector &center )
	{
		Vector velocity;
		pObject->GetVelocityAtPoint( center, &velocity );
		Vector relative = current - velocity;	// speed of the medium relative to the surface
		float speedSq = relative.LengthSqr() * kMetersPerInch * kMetersPerInch;
		if ( speedSq <= 1e-8f )
			return;
		Vector direction = relative;
		VectorNormalize( direction );
		float facing = DotProduct( surfaceNormal, direction );
		if ( facing >= 0 )
			return;		// turned away from the current
		float projected = -facing * area * kMetersPerInch * kMetersPerInch;
		float wetted = frictionArea * kMetersPerInch * kMetersPerInch;
		// Newtons -> Source force (kg*in/s^2), times dt for the impulse.
		float impulse = ( projected * pressureDamp + wetted * frictionDamp ) * speedSq / kMetersPerInch * dt;
		Vector p = direction * impulse;
		part.dragImpulse += p;
		part.dragAngular += CrossProduct( center - bodyCenter, p );
	};

	float radius = pObject->GetSphereRadius();
	if ( radius > 0.0f )
	{
		Vector center;
		pObject->GetPosition( &center, NULL );
		float height = DotProduct( normal, center ) - dist;
		float submerged = clamp( radius - height, 0.0f, 2.0f * radius );
		if ( submerged <= 0 )
			return;
		float volume = M_PI_F * submerged * submerged * ( 3.0f * radius - submerged ) / 3.0f;
		float offset = 3.0f * ( 2.0f * radius - submerged ) * ( 2.0f * radius - submerged ) / ( 4.0f * ( 3.0f * radius - submerged ) );
		part.volume = volume;
		part.volumeMoment = ( center - normal * offset ) * volume;
		Vector velocity;
		pObject->GetVelocityAtPoint( center, &velocity );
		Vector relative = current - velocity;
		if ( relative.LengthSqr() > 1e-6f )
		{
			Vector direction = relative;
			VectorNormalize( direction );
			float projected = M_PI_F * radius * radius * ( submerged / ( 2.0f * radius ) );
			addDrag( projected, 3.0f * projected, -direction, center );
		}
	}
	else if ( pObject->GetCollide() )
	{
		matrix3x4_t objectToWorld;
		pObject->GetPositionMatrix( &objectToWorld );
		const CPhysCollideBox3D *pCollide = ToBox3D( pObject->GetCollide() );
		for ( int c = 0; c < pCollide->convexes.Count(); c++ )
		{
			const CPhysConvexBox3D *pConvex = pCollide->convexes[c];
			int triangleIndices = pConvex->triangles.Count();
			if ( triangleIndices < 12 )
				continue;
			// Triangle winding is consistent within a convex; the sign of the
			// enclosed volume tells whether it is outward.
			float convexVolume = 0.0f;
			Vector convexMoment( 0, 0, 0 );
			CUtlVector<Vector> world;
			world.SetCount( pConvex->points.Count() );
			for ( int i = 0; i < pConvex->points.Count(); i++ )
				VectorTransform( pConvex->points[i], objectToWorld, world[i] );
			float windingVolume = 0.0f;
			Vector interior( 0, 0, 0 );
			for ( int i = 0; i < world.Count(); i++ )
				interior += world[i];
			interior /= (float)world.Count();
			for ( int t = 0; t + 2 < triangleIndices; t += 3 )
			{
				const Vector &a = world[pConvex->triangles[t]], &b = world[pConvex->triangles[t + 1]], &d = world[pConvex->triangles[t + 2]];
				windingVolume += DotProduct( a - interior, CrossProduct( b - interior, d - interior ) );
			}
			float sign = windingVolume < 0 ? -1.0f : 1.0f;

			for ( int t = 0; t + 2 < triangleIndices; t += 3 )
			{
				Vector triangle[3] = { world[pConvex->triangles[t]], world[pConvex->triangles[t + 1]], world[pConvex->triangles[t + 2]] };
				Vector clipped[kMaxClip];
				int count = ClipBelow( triangle, 3, normal, dist, clipped );
				if ( count < 3 )
					continue;
				Vector faceNormal = CrossProduct( triangle[1] - triangle[0], triangle[2] - triangle[0] ) * sign;
				float faceLength = VectorNormalize( faceNormal );
				if ( faceLength <= 0 )
					continue;
				float area = 0.0f;
				Vector areaCenter( 0, 0, 0 );
				for ( int k = 1; k + 1 < count; k++ )
				{
					// Tetrahedron from a point on the surface plane: the cap
					// polygon closing the submerged part adds no volume.
					float v = sign * DotProduct( clipped[0] - planePoint, CrossProduct( clipped[k] - planePoint, clipped[k + 1] - planePoint ) ) / 6.0f;
					convexVolume += v;
					convexMoment += ( planePoint + clipped[0] + clipped[k] + clipped[k + 1] ) * ( 0.25f * v );
					float triArea = 0.5f * CrossProduct( clipped[k] - clipped[0], clipped[k + 1] - clipped[0] ).Length();
					area += triArea;
					areaCenter += ( clipped[0] + clipped[k] + clipped[k + 1] ) * ( triArea / 3.0f );
				}
				if ( area > 0 )
					addDrag( area, 0.5f * faceLength, faceNormal, areaCenter / area );
			}
			if ( convexVolume > 0 )
			{
				part.volume += convexVolume;
				part.volumeMoment += convexMoment;
			}
		}
	}

	bool touched = false;
	if ( part.volume > 0 )
	{
		// Archimedes in Havok units (kg/m^3 * m^3 * m/s^2 = N), converted to
		// a Source impulse.
		Vector gravity;
		pObject->GetEnvironment()->GetGravity( &gravity );
		float g = gravity.Length() * kMetersPerInch;
		float volume = part.volume * kMetersPerInch * kMetersPerInch * kMetersPerInch;
		float impulse = mediumDensity * volume * g / kMetersPerInch * dt;
		Vector center = part.volumeMoment / part.volume;
		b3Body_ApplyLinearImpulse( body, ToB3( normal * impulse ), ToB3( center ), false );
		touched = true;
	}
	if ( part.dragImpulse.LengthSqr() > 0 )
	{
		// Pressure damping can at most stop the relative motion.
		Vector velocity;
		pObject->GetVelocityAtPoint( bodyCenter, &velocity );
		float mass = pObject->GetMass();
		Vector relative = current - velocity;
		float maxImpulse = relative.Length() * mass;
		float impulse = part.dragImpulse.Length();
		float scale = impulse > maxImpulse && impulse > 0 ? maxImpulse / impulse : 1.0f;
		b3Body_ApplyLinearImpulseToCenter( body, ToB3( part.dragImpulse * scale ), false );
		b3Body_ApplyAngularImpulse( body, ToB3( part.dragAngular * scale ), false );
		touched = true;
	}
	(void)touched;
}

//-----------------------------------------------------------------------------
// Spring
//-----------------------------------------------------------------------------
CPhysicsSpringBox3D::CPhysicsSpringBox3D( CPhysicsObjectBox3D *pStart, CPhysicsObjectBox3D *pEnd, const springparams_t &params )
	: m_pStart( pStart ), m_pEnd( pEnd ), m_constant( params.constant ), m_naturalLength( params.naturalLength ),
	  m_damping( params.damping ), m_relativeDamping( params.relativeDamping ), m_onlyStretch( params.onlyStretch )
{
	if ( params.useLocalPositions )
	{
		m_localStart = params.startPosition;
		m_localEnd = params.endPosition;
	}
	else
	{
		pStart->WorldToLocal( &m_localStart, params.startPosition );
		pEnd->WorldToLocal( &m_localEnd, params.endPosition );
	}
}

IPhysicsObject *CPhysicsSpringBox3D::GetStartObject( void )
{
	return m_pStart;
}

IPhysicsObject *CPhysicsSpringBox3D::GetEndObject( void )
{
	return m_pEnd;
}

bool CPhysicsSpringBox3D::Links( IPhysicsObject *pObject ) const
{
	return pObject && ( pObject == (IPhysicsObject *)m_pStart || pObject == (IPhysicsObject *)m_pEnd );
}

void CPhysicsSpringBox3D::GetEndpoints( Vector *worldPositionStart, Vector *worldPositionEnd )
{
	if ( !m_pStart || !m_pEnd )
		return;
	if ( worldPositionStart )
		m_pStart->LocalToWorld( worldPositionStart, m_localStart );
	if ( worldPositionEnd )
		m_pEnd->LocalToWorld( worldPositionEnd, m_localEnd );
}

void CPhysicsSpringBox3D::Simulate( float dt )
{
	if ( !m_pStart || !m_pEnd )
		return;
	Vector start, end;
	m_pStart->LocalToWorld( &start, m_localStart );
	m_pEnd->LocalToWorld( &end, m_localEnd );
	Vector direction = start - end;
	float length = VectorNormalize( direction );
	if ( length < 1e-6f )
		return;
	if ( m_onlyStretch && length <= m_naturalLength )
		return;

	Vector startVelocity, endVelocity;
	m_pStart->GetVelocityAtPoint( start, &startVelocity );
	m_pEnd->GetVelocityAtPoint( end, &endVelocity );
	Vector relative = endVelocity - startVelocity;
	// Positive when the ends close on each other.
	float closing = DotProduct( direction, relative );
	float force = ( length - m_naturalLength ) * m_constant - m_damping * closing;
	Vector impulse = direction * ( force * dt ) - relative * ( dt * m_relativeDamping );

	// As IVP's actuator: only simulated (awake, unpinned) ends are pushed.
	if ( m_pEnd->IsMoveable() && !m_pEnd->IsAsleep() && b3Body_GetType( m_pEnd->GetBody() ) == b3_dynamicBody )
		b3Body_ApplyLinearImpulse( m_pEnd->GetBody(), ToB3( impulse ), ToB3( end ), false );
	if ( m_pStart->IsMoveable() && !m_pStart->IsAsleep() && b3Body_GetType( m_pStart->GetBody() ) == b3_dynamicBody )
		b3Body_ApplyLinearImpulse( m_pStart->GetBody(), ToB3( -impulse ), ToB3( start ), false );
}

void CPhysicsSpringBox3D::WriteParams( springparams_t &params ) const
{
	memset( &params, 0, sizeof( params ) );
	params.constant = m_constant;
	params.naturalLength = m_naturalLength;
	params.damping = m_damping;
	params.relativeDamping = m_relativeDamping;
	params.startPosition = m_localStart;
	params.endPosition = m_localEnd;
	params.useLocalPositions = true;
	params.onlyStretch = m_onlyStretch;
}
