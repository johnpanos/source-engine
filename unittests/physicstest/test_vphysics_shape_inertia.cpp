//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: vphysics.shape-inertia.v1 capability contract (RFC 0013), run with
//          the parallel-step contract by `vphysics_conformance --bench
//          contract`. Every check is named inertia.*; the physics benchmark
//          gate `shape-inertia` requires them (tools/quality/physics_bench.py,
//          quality/budgets/physics-v1.json). The contract record is
//          unittests/physicstest/contracts/vphysics.shape-inertia.v1.md.
//
//          The sensitivity faults wrap the capability as a provider that
//          breaks one clause:
//            inertia-model-ignored     accepts the model but keeps legacy objects
//            inertia-accepts-populated accepts a model change after objects exist
//            inertia-tensor-diagonal   reports tensors without their products
//            inertia-clamped           applies rotInertiaLimit under the shape model
//
//=============================================================================//
#include "vphysics_conformance.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "vphysics/performance.h"
#include "vphysics/shape_inertia.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const float kMetersPerInch = 0.0254f;
const float kInchesPerMeterSq = 1.0f / ( 0.0254f * 0.0254f );

typedef float Tensor_t[3][3];

// The capability as seen through the active fault.
class CFaultShapeInertia : public IPhysicsShapeInertia
{
public:
	explicit CFaultShapeInertia( IPhysicsShapeInertia *pInner ) : m_pInner( pInner ) {}
	virtual bool SetInertiaModel( IPhysicsEnvironment *pEnvironment, physics_inertia_model_t model )
	{
		if ( FaultIs( "inertia-model-ignored" ) )
			return pEnvironment != NULL;
		bool accepted = m_pInner->SetInertiaModel( pEnvironment, model );
		if ( !accepted && FaultIs( "inertia-accepts-populated" ) && pEnvironment )
			return true;
		return accepted;
	}
	virtual physics_inertia_model_t GetInertiaModel( const IPhysicsEnvironment *pEnvironment ) const
	{
		return m_pInner->GetInertiaModel( pEnvironment );
	}
	virtual bool GetInertiaTensor( const IPhysicsObject *pObject, float tensor[3][3] ) const
	{
		bool ok = m_pInner->GetInertiaTensor( pObject, tensor );
		if ( ok && FaultIs( "inertia-tensor-diagonal" ) )
		{
			for ( int i = 0; i < 3; i++ )
			{
				for ( int j = 0; j < 3; j++ )
				{
					if ( i != j )
						tensor[i][j] = 0.0f;
				}
			}
		}
		return ok;
	}

private:
	IPhysicsShapeInertia *m_pInner;
};

IPhysicsShapeInertia *s_pInertia;

IPhysicsEnvironment *CreateInertiaEnvironment( physics_inertia_model_t model )
{
	IPhysicsEnvironment *pEnv = s_pPhysics->CreateEnvironment();
	if ( !pEnv )
		return NULL;
	pEnv->SetGravity( vec3_origin );
	physics_performanceparams_t perf;
	perf.Defaults();
	pEnv->SetPerformanceSettings( &perf );
	if ( model != PHYSICS_INERTIA_LEGACY && !s_pInertia->SetInertiaModel( pEnv, model ) )
	{
		s_pPhysics->DestroyEnvironment( pEnv );
		return NULL;
	}
	return pEnv;
}

// A dynamic body with no damping, drag or gravity. Under "inertia-clamped" the
// host applies the legacy rotInertiaLimit to its diagonal, as a provider that
// kept the clip would.
IPhysicsObject *CreateBody( IPhysicsEnvironment *pEnv, const CPhysCollide *pCollide, float mass,
    const Vector *pMassCenter = NULL, float rotInertiaLimit = 0.05f )
{
	objectparams_t params = DefaultParams( mass, NULL );
	params.damping = 0.0f;
	params.rotdamping = 0.0f;
	params.dragCoefficient = 0.0f;
	params.rotInertiaLimit = rotInertiaLimit;
	params.massCenterOverride = const_cast<Vector *>( pMassCenter );
	IPhysicsObject *pObject =
	    pEnv->CreatePolyObject( pCollide, 0, vec3_origin, vec3_angle, &params );
	if ( !pObject )
		return NULL;
	pObject->EnableMotion( true );
	pObject->EnableGravity( false );
	pObject->EnableDrag( false );
	pObject->Wake();
	if ( FaultIs( "inertia-clamped" ) &&
	     s_pInertia->GetInertiaModel( pEnv ) == PHYSICS_INERTIA_SHAPE )
	{
		Vector inertia = pObject->GetInertia();
		float minimum = inertia.Length() * rotInertiaLimit;
		for ( int i = 0; i < 3; i++ )
			inertia[i] = MAX( inertia[i], minimum );
		pObject->SetInertia( inertia );
	}
	return pObject;
}

// A solid box of the given size (inches) turned by `angles` inside its
// collide, so its principal axes are not the object axes.
CPhysCollide *RotatedBoxCollide( const Vector &size, const QAngle &angles, matrix3x4_t *pRotation )
{
	AngleMatrix( angles, *pRotation );
	Vector corners[8];
	Vector *pCorners[8];
	for ( int i = 0; i < 8; i++ )
	{
		Vector local( ( i & 1 ) ? size.x : -size.x, ( i & 2 ) ? size.y : -size.y,
		    ( i & 4 ) ? size.z : -size.z );
		VectorRotate( local * 0.5f, *pRotation, corners[i] );
		pCorners[i] = &corners[i];
	}
	CPhysConvex *pConvex = s_pCollision->ConvexFromVerts( pCorners, 8 );
	return pConvex ? s_pCollision->ConvertConvexToCollide( &pConvex, 1 ) : NULL;
}

// Solid box principal inertia (kg*m^2) about its center along its own axes.
Vector BoxInertia( const Vector &size, float mass )
{
	Vector m = size * kMetersPerInch;
	return Vector( mass * ( m.y * m.y + m.z * m.z ) / 12.0f,
	    mass * ( m.x * m.x + m.z * m.z ) / 12.0f, mass * ( m.x * m.x + m.y * m.y ) / 12.0f );
}

// R diag(d) R^T.
void RotatedTensor( const matrix3x4_t &rotation, const Vector &principal, Tensor_t out )
{
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			out[i][j] = 0.0f;
			for ( int k = 0; k < 3; k++ )
				out[i][j] += rotation[i][k] * principal[k] * rotation[j][k];
		}
	}
}

// Largest element difference relative to the largest element of `expected`.
float TensorError( const Tensor_t actual, const Tensor_t expected )
{
	float scale = 0.0f, error = 0.0f;
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			scale = MAX( scale, fabsf( expected[i][j] ) );
			error = MAX( error, fabsf( actual[i][j] - expected[i][j] ) );
		}
	}
	return scale > 0.0f ? error / scale : error;
}

float OffDiagonal( const Tensor_t tensor )
{
	return MAX( MAX( fabsf( tensor[0][1] ), fabsf( tensor[0][2] ) ), fabsf( tensor[1][2] ) );
}

bool Tensor( const IPhysicsObject *pObject, Tensor_t out )
{
	memset( out, 0, sizeof( Tensor_t ) );
	return pObject && s_pInertia->GetInertiaTensor( pObject, out );
}

Vector Multiply( const Tensor_t tensor, const Vector &v )
{
	return Vector( tensor[0][0] * v.x + tensor[0][1] * v.y + tensor[0][2] * v.z,
	    tensor[1][0] * v.x + tensor[1][1] * v.y + tensor[1][2] * v.z,
	    tensor[2][0] * v.x + tensor[2][1] * v.y + tensor[2][2] * v.z );
}

bool Invert( const Tensor_t m, Tensor_t out )
{
	double c[3][3];
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			int i1 = ( i + 1 ) % 3, i2 = ( i + 2 ) % 3, j1 = ( j + 1 ) % 3, j2 = ( j + 2 ) % 3;
			c[i][j] = (double)m[i1][j1] * m[i2][j2] - (double)m[i1][j2] * m[i2][j1];
		}
	}
	double det = m[0][0] * c[0][0] + m[0][1] * c[0][1] + m[0][2] * c[0][2];
	if ( det <= 0.0 )
		return false;
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
			out[i][j] = (float)( c[j][i] / det );
	}
	return true;
}

void CheckInterface()
{
	int dummy = 0;
	IPhysicsEnvironment *pForeign = reinterpret_cast<IPhysicsEnvironment *>( &dummy );
	Tensor_t sentinel;
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
			sentinel[i][j] = 7.0f;
	}
	bool untouched =
	    !s_pInertia->GetInertiaTensor( NULL, sentinel ) &&
	    !s_pInertia->GetInertiaTensor( reinterpret_cast<IPhysicsObject *>( &dummy ), sentinel ) &&
	    sentinel[1][2] == 7.0f;
	Check( TIER_GAMEPLAY, "inertia.rejects-foreign",
	    !s_pInertia->SetInertiaModel( NULL, PHYSICS_INERTIA_SHAPE ) &&
	        !s_pInertia->SetInertiaModel( pForeign, PHYSICS_INERTIA_SHAPE ) &&
	        s_pInertia->GetInertiaModel( NULL ) == PHYSICS_INERTIA_LEGACY &&
	        s_pInertia->GetInertiaModel( pForeign ) == PHYSICS_INERTIA_LEGACY && untouched,
	    "tensor untouched %d", untouched );

	IPhysicsEnvironment *pEnv = s_pPhysics->CreateEnvironment();
	bool unknown = !s_pInertia->SetInertiaModel( pEnv, (physics_inertia_model_t)7 );
	physics_inertia_model_t initial = s_pInertia->GetInertiaModel( pEnv );
	Check( TIER_GAMEPLAY, "inertia.rejects-unknown-model",
	    unknown && initial == PHYSICS_INERTIA_LEGACY );

	CPhysCollide *pBox = s_pCollision->BBoxToCollide( Vector( -24, -12, -4 ), Vector( 24, 12, 4 ) );
	IPhysicsObject *pObject = CreateBody( pEnv, pBox, 20.0f );
	Tensor_t tensor;
	bool read = Tensor( pObject, tensor );
	Vector inertia = pObject->GetInertia();
	Check( TIER_GAMEPLAY, "inertia.default-legacy",
	    initial == PHYSICS_INERTIA_LEGACY && read && tensor[0][0] == inertia.x &&
	        tensor[1][1] == inertia.y && tensor[2][2] == inertia.z && OffDiagonal( tensor ) == 0.0f,
	    "diag (%g %g %g) inertia (%g %g %g) off %g", tensor[0][0], tensor[1][1], tensor[2][2],
	    inertia.x, inertia.y, inertia.z, OffDiagonal( tensor ) );
	bool rejected = !s_pInertia->SetInertiaModel( pEnv, PHYSICS_INERTIA_SHAPE );
	Check( TIER_GAMEPLAY, "inertia.rejects-populated",
	    rejected && s_pInertia->GetInertiaModel( pEnv ) == PHYSICS_INERTIA_LEGACY );
	pEnv->DestroyObject( pObject );
	s_pPhysics->DestroyEnvironment( pEnv );

	pEnv = s_pPhysics->CreateEnvironment();
	bool set = s_pInertia->SetInertiaModel( pEnv, PHYSICS_INERTIA_SHAPE );
	Check( TIER_GAMEPLAY, "inertia.selects",
	    set && s_pInertia->GetInertiaModel( pEnv ) == PHYSICS_INERTIA_SHAPE );
	s_pPhysics->DestroyEnvironment( pEnv );
	s_pCollision->DestroyCollide( pBox );
}

// The tensor is the solid's: exact for a box, with products of inertia for a
// rotated one, about an overridden mass center, and without rotInertiaLimit.
void CheckTensorValues()
{
	IPhysicsEnvironment *pShape = CreateInertiaEnvironment( PHYSICS_INERTIA_SHAPE );
	IPhysicsEnvironment *pLegacy = CreateInertiaEnvironment( PHYSICS_INERTIA_LEGACY );
	if ( !Check( TIER_GAMEPLAY, "inertia.environments", pShape && pLegacy ) )
	{
		if ( pShape )
			s_pPhysics->DestroyEnvironment( pShape );
		if ( pLegacy )
			s_pPhysics->DestroyEnvironment( pLegacy );
		return;
	}
	const float kMass = 20.0f;
	Vector size( 48, 24, 8 );
	Vector principal = BoxInertia( size, kMass );

	CPhysCollide *pBox = s_pCollision->BBoxToCollide( size * -0.5f, size * 0.5f );
	IPhysicsObject *pBoxObject = CreateBody( pShape, pBox, kMass );
	Tensor_t tensor, expected;
	matrix3x4_t identity;
	SetIdentityMatrix( identity );
	RotatedTensor( identity, principal, expected );
	Tensor( pBoxObject, tensor );
	Check( TIER_GAMEPLAY, "inertia.box-tensor", TensorError( tensor, expected ) < 0.005f,
	    "diag (%g %g %g) solid (%g %g %g) off %g", tensor[0][0], tensor[1][1], tensor[2][2],
	    principal.x, principal.y, principal.z, OffDiagonal( tensor ) );

	matrix3x4_t rotation;
	CPhysCollide *pRotated = RotatedBoxCollide( size, QAngle( 30, 40, 20 ), &rotation );
	IPhysicsObject *pRotatedObject = pRotated ? CreateBody( pShape, pRotated, kMass ) : NULL;
	RotatedTensor( rotation, principal, expected );
	Tensor( pRotatedObject, tensor );
	Check( TIER_GAMEPLAY, "inertia.rotated-products",
	    pRotatedObject && TensorError( tensor, expected ) < 0.01f,
	    "error %.4f off (%g %g %g) expected (%g %g %g)", TensorError( tensor, expected ),
	    tensor[0][1], tensor[0][2], tensor[1][2], expected[0][1], expected[0][2], expected[1][2] );

	// Mass center moved 10 in along x: the parallel-axis shift.
	Vector center( 10, 0, 0 );
	IPhysicsObject *pShifted = CreateBody( pShape, pBox, kMass, &center );
	float d = center.x * kMetersPerInch;
	RotatedTensor( identity, principal + Vector( 0, kMass * d * d, kMass * d * d ), expected );
	Tensor( pShifted, tensor );
	Check( TIER_GAMEPLAY, "inertia.mass-center", TensorError( tensor, expected ) < 0.01f,
	    "diag (%g %g %g) expected (%g %g %g)", tensor[0][0], tensor[1][1], tensor[2][2],
	    expected[0][0], expected[1][1], expected[2][2] );

	// A 64x4x4 rod: the shape model keeps its thin-axis inertia, while the
	// legacy model raises it to rotInertiaLimit * |I|.
	Vector rodSize( 64, 4, 4 );
	CPhysCollide *pRod = s_pCollision->BBoxToCollide( rodSize * -0.5f, rodSize * 0.5f );
	IPhysicsObject *pRodShape = CreateBody( pShape, pRod, kMass );
	IPhysicsObject *pRodLegacy = CreateBody( pLegacy, pRod, kMass );
	Vector rodSolid = BoxInertia( rodSize, kMass );
	Vector shapeRod = pRodShape->GetInertia();
	Vector legacyRod = pRodLegacy->GetInertia();
	Check( TIER_GAMEPLAY, "inertia.no-rot-limit",
	    fabsf( shapeRod.x - rodSolid.x ) < 0.01f * rodSolid.x, "thin axis %g solid %g (legacy %g)",
	    shapeRod.x, rodSolid.x, legacyRod.x );
	Check( TIER_GAMEPLAY, "inertia.legacy-rot-limit",
	    legacyRod.x >= 0.05f * legacyRod.Length() * 0.99f && legacyRod.x > 2.0f * rodSolid.x,
	    "legacy thin axis %g, |I| %g", legacyRod.x, legacyRod.Length() );

	// The per-axis API scales the diagonal and keeps the coupling.
	Tensor_t before;
	Tensor( pRotatedObject, before );
	Vector saved = pRotatedObject->GetInertia();
	pRotatedObject->SetInertia( Vector( 1e14f, 1e14f, 1e14f ) );
	Vector locked = pRotatedObject->GetInertia();
	pRotatedObject->SetInertia( saved );
	Tensor( pRotatedObject, tensor );
	float roundTrip = TensorError( tensor, before );
	pRotatedObject->SetInertia( saved * 2.0f );
	Tensor_t doubled;
	Tensor( pRotatedObject, doubled );
	for ( int i = 0; i < 3; i++ )
	{
		for ( int j = 0; j < 3; j++ )
			expected[i][j] = 2.0f * before[i][j];
	}
	float scaled = TensorError( doubled, expected );
	pRotatedObject->SetInertia( saved );
	Check( TIER_GAMEPLAY, "inertia.set-inertia-keeps-coupling",
	    locked.x == 1e14f && roundTrip < 1e-4f && scaled < 1e-4f && OffDiagonal( before ) > 0.0f,
	    "round trip %g doubled %g", roundTrip, scaled );

	pRotatedObject->SetMass( 2.0f * kMass );
	Tensor( pRotatedObject, tensor );
	float massScaled = TensorError( tensor, expected );
	pRotatedObject->SetMass( kMass );
	Check( TIER_GAMEPLAY, "inertia.set-mass-scales", massScaled < 1e-4f, "error %g", massScaled );

	// The per-axis inverse and the helpers use the full tensor.
	Tensor( pRotatedObject, tensor );
	Tensor_t inverse;
	bool inverted = Invert( tensor, inverse );
	Vector invInertia = pRotatedObject->GetInvInertia();
	float invError = 0.0f;
	for ( int i = 0; i < 3; i++ )
		invError = MAX( invError, fabsf( invInertia[i] - inverse[i][i] ) / inverse[i][i] );
	Check( TIER_GAMEPLAY, "inertia.inverse-diagonal", inverted && invError < 1e-3f, "error %g",
	    invError );

	Vector force( 0, 0, 1000 ), at( 20, 10, 0 ), linear;
	AngularImpulse torque, angular;
	pRotatedObject->CalculateForceOffset( force, at, &linear, &torque );
	pRotatedObject->CalculateVelocityOffset( force, at, &linear, &angular );
	Vector wanted = Multiply( inverse, torque );
	Check( TIER_GAMEPLAY, "inertia.velocity-offset",
	    ( angular - wanted ).Length() < 1e-3f * wanted.Length(),
	    "angular (%g %g %g) expected (%g %g %g)", angular.x, angular.y, angular.z, wanted.x,
	    wanted.y, wanted.z );

	AngularImpulse spin( 90, 45, 30 );
	pRotatedObject->SetVelocity( &vec3_origin, &spin );
	Vector w = spin * ( M_PI_F / 180.0f );
	float energy = 0.5f * DotProduct( w, Multiply( tensor, w ) ) * kInchesPerMeterSq;
	float reported = pRotatedObject->GetEnergy();
	Check( TIER_GAMEPLAY, "inertia.energy", fabsf( reported - energy ) < 0.01f * energy,
	    "energy %g expected %g", reported, energy );

	// TransferObject keeps the object's model.
	Tensor( pRotatedObject, before );
	bool moved = pShape->TransferObject( pRotatedObject, pLegacy );
	Tensor( pRotatedObject, tensor );
	Check( TIER_GAMEPLAY, "inertia.transfer-keeps-model",
	    moved && TensorError( tensor, before ) < 1e-5f && OffDiagonal( tensor ) > 0.0f,
	    "moved %d error %g", moved, TensorError( tensor, before ) );

	s_pPhysics->DestroyEnvironment( pShape );
	s_pPhysics->DestroyEnvironment( pLegacy );
	s_pCollision->DestroyCollide( pBox );
	s_pCollision->DestroyCollide( pRod );
	if ( pRotated )
		s_pCollision->DestroyCollide( pRotated );
}

// The solver rotates with the tensor: a box whose principal axes are turned
// inside its collide, spun about an object axis, keeps the world angular
// momentum the tensor implies while its angular velocity wanders. Under the
// legacy model the object axes are principal, so it would just spin.
void CheckDynamics()
{
	IPhysicsEnvironment *pShape = CreateInertiaEnvironment( PHYSICS_INERTIA_SHAPE );
	if ( !pShape )
		return;
	pShape->SetSimulationTimestep( kTick );
	matrix3x4_t rotation;
	CPhysCollide *pRotated =
	    RotatedBoxCollide( Vector( 48, 24, 8 ), QAngle( 30, 40, 20 ), &rotation );
	IPhysicsObject *pObject = pRotated ? CreateBody( pShape, pRotated, 20.0f ) : NULL;
	if ( !Check( TIER_GAMEPLAY, "inertia.dynamics-body", pObject != NULL ) )
	{
		s_pPhysics->DestroyEnvironment( pShape );
		return;
	}
	AngularImpulse spin( 0, 0, 360 );
	pObject->SetVelocity( &vec3_origin, &spin );
	Step( pShape, kTick );
	Tensor_t tensor;
	Tensor( pObject, tensor );

	Vector momentum0, omega0;
	float maxMomentumAngle = 0.0f, minRatio = 1.0f, maxRatio = 1.0f, maxWander = 0.0f;
	for ( int i = 0; i < 200; i++ )
	{
		AngularImpulse local = AngularVelocityOf( pObject ) * ( M_PI_F / 180.0f );
		Vector localMomentum = Multiply( tensor, local ), momentum, omega;
		pObject->LocalToWorldVector( &momentum, localMomentum );
		pObject->LocalToWorldVector( &omega, local );
		if ( i == 0 )
		{
			momentum0 = momentum;
			omega0 = omega;
		}
		else
		{
			float cosine =
			    DotProduct( momentum, momentum0 ) / ( momentum.Length() * momentum0.Length() );
			maxMomentumAngle =
			    MAX( maxMomentumAngle, RAD2DEG( acosf( clamp( cosine, -1.0f, 1.0f ) ) ) );
			float ratio = momentum.Length() / momentum0.Length();
			minRatio = MIN( minRatio, ratio );
			maxRatio = MAX( maxRatio, ratio );
			float wander = DotProduct( omega, omega0 ) / ( omega.Length() * omega0.Length() );
			maxWander = MAX( maxWander, RAD2DEG( acosf( clamp( wander, -1.0f, 1.0f ) ) ) );
		}
		Step( pShape, kTick );
	}
	Check( TIER_GAMEPLAY, "inertia.dynamics-momentum",
	    maxMomentumAngle < 3.0f && minRatio > 0.8f && maxRatio < 1.03f,
	    "max angle %.3f |L|/|L0| %.4f..%.4f", maxMomentumAngle, minRatio, maxRatio );
	Check( TIER_GAMEPLAY, "inertia.dynamics-wanders", maxWander > 10.0f, "max angle(w, w0) %.3f",
	    maxWander );
	s_pPhysics->DestroyEnvironment( pShape );
	s_pCollision->DestroyCollide( pRotated );
}
}

int RunShapeInertiaContract()
{
	IPhysicsShapeInertia *pInner = (IPhysicsShapeInertia *)s_pPhysics->QueryInterface(
	    VPHYSICS_SHAPE_INERTIA_INTERFACE_VERSION );
	if ( !pInner )
	{
		printf( "UNSUPPORTED %s\n", VPHYSICS_SHAPE_INERTIA_INTERFACE_VERSION );
		return 3;
	}
	CFaultShapeInertia faulted( pInner );
	s_pInertia = &faulted;
	CheckInterface();
	CheckTensorValues();
	CheckDynamics();
	s_pInertia = NULL;
	return 0;
}
