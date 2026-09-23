//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics provider conformance: save/restore.
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//
//          Follows the game's protocol (game/server/physics.cpp's save block
//          handler): each physics interface is written through
//          IPhysicsEnvironment::Save in dependency order into one stream; the
//          environment is torn down; a fresh environment is created with the
//          world object re-created by the game (announced through PreRestore);
//          every record is read back through Restore in the same order; then
//          PostRestore. Pointers between records (constraint -> objects,
//          motion controller -> objects) must resolve to the restored objects.
//
//=============================================================================//

#include <math.h>
#include <string.h>

#include "vphysics_conformance.h"
#include "vphysics/constraints.h"
#include "saverestore_host.h"
#include "bspflags.h"

namespace
{
class CNullMotion : public IMotionEvent
{
public:
	CNullMotion() : m_calls( 0 ) {}
	virtual simresult_e Simulate( IPhysicsMotionController *, IPhysicsObject *, float, Vector &linear, AngularImpulse &angular )
	{
		m_calls++;
		linear.Init();
		angular.Init();
		return SIM_NOTHING;
	}
	int m_calls;
};

struct Record_t
{
	PhysInterfaceId_t type;
	void *pOld;
	void *pNew;
	int gameData;
};

bool SaveRecord( IPhysicsEnvironment *pEnv, CHostSave &save, PhysInterfaceId_t type, void *pObject, CUtlVector<Record_t> &records )
{
	physsaveparams_t params = { &save, pObject, type };
	// Sensitivity: "save-dropped" models a provider that saves nothing.
	bool ok = FaultIs( "save-dropped" ) ? true : pEnv->Save( params );
	Record_t record = { type, pObject, NULL, records.Count() };
	records.AddToTail( record );
	return ok;
}

void *RestoreRecord( IPhysicsEnvironment *pEnv, CHostRestore &restore, Record_t &record, const CPhysCollide *pCollide, int *pGameData )
{
	physrestoreparams_t params;
	memset( &params, 0, sizeof( params ) );
	params.pRestore = &restore;
	params.ppObject = &record.pNew;
	params.type = record.type;
	params.pGameData = pGameData;
	params.pName = "restored";
	params.pCollisionModel = pCollide;
	params.pEnvironment = pEnv;
	if ( !pEnv->Restore( params ) )
		return NULL;
	return record.pNew;
}
}

void TestSaveRestore()
{
	// Collision model owned by the test (it outlives both environments, as
	// the model cache does in the game).
	CPhysCollide *pCube = s_pCollision->BBoxToCollide( Vector( -16, -16, -16 ), Vector( 16, 16, 16 ) );
	int gameData[8] = {};

	World_t world;
	CreateWorld( world, NULL );
	IPhysicsEnvironment *pEnv = world.pEnv;
	int material = world.material;
	int wood = s_pProps->GetSurfaceIndex( "wood" );
	if ( wood < 0 )
		wood = material;

	objectparams_t params = DefaultParams( 40.0f, &gameData[0] );
	IPhysicsObject *pMover = pEnv->CreatePolyObject( pCube, wood, Vector( 0, 0, 300 ), QAngle( 0, 30, 0 ), &params );
	pMover->Wake();
	Vector velocity( 50, 0, 20 );
	AngularImpulse spin( 0, 0, 45 );
	pMover->SetVelocity( &velocity, &spin );
	pMover->SetGameFlags( 0x33 );
	pMover->SetContents( CONTENTS_SOLID | CONTENTS_MONSTER );
	pMover->SetCallbackFlags( pMover->GetCallbackFlags() | CALLBACK_GLOBAL_TOUCH_STATIC );
	float damping = 0.3f, rotDamping = 0.4f;
	pMover->SetDamping( &damping, &rotDamping );
	Vector moverInertia = pMover->GetInertia();

	params = DefaultParams( 30.0f, &gameData[1] );
	IPhysicsObject *pPinned = pEnv->CreatePolyObject( pCube, material, Vector( 200, 0, 300 ), vec3_angle, &params );
	pPinned->EnableMotion( false );

	params = DefaultParams( 20.0f, &gameData[2] );
	IPhysicsObject *pSleeper = pEnv->CreatePolyObject( pCube, material, Vector( 400, 0, 300 ), vec3_angle, &params );
	pSleeper->Sleep();

	params = DefaultParams( 25.0f, &gameData[3] );
	IPhysicsObject *pShadowed = pEnv->CreatePolyObject( pCube, material, Vector( 600, 0, 300 ), vec3_angle, &params );
	pShadowed->SetShadow( 300.0f, 200.0f, true, false );
	pShadowed->UpdateShadow( Vector( 650, 0, 300 ), vec3_angle, false, 0.5f );

	params = DefaultParams( 10.0f, &gameData[4] );
	IPhysicsObject *pPendulum = pEnv->CreatePolyObject( pCube, material, Vector( 800, 40, 300 ), vec3_angle, &params );
	pPendulum->Wake();

	constraint_groupparams_t groupParams;
	groupParams.Defaults();
	groupParams.minErrorTicks = 7;
	IPhysicsConstraintGroup *pGroup = pEnv->CreateConstraintGroup( groupParams );
	constraint_ballsocketparams_t ballsocket;
	ballsocket.Defaults();
	ballsocket.InitWithCurrentObjectState( world.pFloor, pPendulum, Vector( 800, 0, 300 ) );
	IPhysicsConstraint *pBall = pEnv->CreateBallsocketConstraint( world.pFloor, pPendulum, pGroup, ballsocket );
	pGroup->Activate();

	springparams_t spring;
	memset( &spring, 0, sizeof( spring ) );
	spring.constant = 500.0f;
	spring.naturalLength = 50.0f;
	spring.damping = 10.0f;
	spring.startPosition = vec3_origin;
	spring.endPosition = vec3_origin;
	spring.useLocalPositions = true;
	IPhysicsSpring *pSpring = pEnv->CreateSpring( pPinned, pSleeper, &spring );

	CNullMotion motion;
	IPhysicsMotionController *pMotion = pEnv->CreateMotionController( &motion );
	pMotion->AttachObject( pMover, true );

	Step( pEnv, kTick );
	Vector moverPosition, pendulumLocal;
	QAngle moverAngles;
	pMover->GetPosition( &moverPosition, &moverAngles );
	Vector moverVelocity = VelocityOf( pMover );
	pPendulum->WorldToLocal( &pendulumLocal, Vector( 800, 0, 300 ) );

	// Save in dependency order: objects, then what links them.
	CSaveRestoreBuffer buffer;
	CHostSave save( &buffer );
	CUtlVector<Record_t> records;
	IPhysicsObject *pObjects[] = { pMover, pPinned, pSleeper, pShadowed, pPendulum };
	bool saved = true;
	for ( int i = 0; i < (int)ARRAYSIZE( pObjects ); i++ )
		saved &= SaveRecord( pEnv, save, PIID_IPHYSICSOBJECT, pObjects[i], records );
	saved &= SaveRecord( pEnv, save, PIID_IPHYSICSCONSTRAINTGROUP, pGroup, records );
	saved &= SaveRecord( pEnv, save, PIID_IPHYSICSCONSTRAINT, pBall, records );
	saved &= SaveRecord( pEnv, save, PIID_IPHYSICSSPRING, pSpring, records );
	saved &= SaveRecord( pEnv, save, PIID_IPHYSICSMOTIONCONTROLLER, pMotion, records );
	Check( TIER_BOOT, "save.write", saved && buffer.Size() > 0 && buffer.Errors() == 0, "size %d errors %d", buffer.Size(), buffer.Errors() );
	IPhysicsObject *pOldFloor = world.pFloor;

	pEnv->DestroyMotionController( pMotion );
	pEnv->DestroySpring( pSpring );
	pEnv->DestroyConstraint( pBall );
	pEnv->DestroyConstraintGroup( pGroup );
	DestroyWorld( world );

	// A fresh level: the game re-creates the world and announces it.
	World_t restored;
	CreateWorld( restored, NULL );
	pEnv = restored.pEnv;
	physprerestoreparams_t pre;
	pre.recreatedObjectCount = 1;
	pre.recreatedObjectList[0].pOldObject = pOldFloor;
	pre.recreatedObjectList[0].pNewObject = restored.pFloor;
	pEnv->PreRestore( pre );
	CHostRestore restore( &buffer );
	bool allRestored = true;
	for ( int i = 0; i < records.Count(); i++ )
	{
		int *pData = records[i].type == PIID_IPHYSICSOBJECT ? &gameData[i] : &gameData[7];
		allRestored &= RestoreRecord( pEnv, restore, records[i], pCube, pData ) != NULL;
	}
	pEnv->PostRestore();
	Check( TIER_BOOT, "restore.read", allRestored && buffer.Errors() == 0 && restore.GetReadPos() == buffer.Size(),
		"all %d errors %d read %d of %d", allRestored, buffer.Errors(), restore.GetReadPos(), buffer.Size() );
	if ( !allRestored )
	{
		DestroyWorld( restored );
		s_pCollision->DestroyCollide( pCube );
		return;
	}

	IPhysicsObject *pNewMover = (IPhysicsObject *)records[0].pNew;
	IPhysicsObject *pNewPinned = (IPhysicsObject *)records[1].pNew;
	IPhysicsObject *pNewSleeper = (IPhysicsObject *)records[2].pNew;
	IPhysicsObject *pNewShadowed = (IPhysicsObject *)records[3].pNew;
	IPhysicsObject *pNewPendulum = (IPhysicsObject *)records[4].pNew;
	IPhysicsConstraint *pNewBall = (IPhysicsConstraint *)records[6].pNew;
	IPhysicsSpring *pNewSpring = (IPhysicsSpring *)records[7].pNew;
	IPhysicsMotionController *pNewMotion = (IPhysicsMotionController *)records[8].pNew;

	int listed = 0;
	restored.pEnv->GetObjectList( &listed );
	Check( TIER_GAMEPLAY, "restore.objects-listed", listed == 1 + (int)ARRAYSIZE( pObjects ), "count %d", listed );

	Vector position;
	QAngle angles;
	pNewMover->GetPosition( &position, &angles );
	float gotDamping = 0, gotRot = 0;
	pNewMover->GetDamping( &gotDamping, &gotRot );
	Check( TIER_GAMEPLAY, "restore.object-transform", NearVec( position, moverPosition, 0.05f ) && Near( angles.y, moverAngles.y, 0.1f ),
		"(%.2f %.2f %.2f) yaw %.2f", position.x, position.y, position.z, angles.y );
	Check( TIER_GAMEPLAY, "restore.object-velocity", NearVec( VelocityOf( pNewMover ), moverVelocity, 0.5f ) && !pNewMover->IsAsleep(),
		"(%.2f %.2f %.2f)", VelocityOf( pNewMover ).x, VelocityOf( pNewMover ).y, VelocityOf( pNewMover ).z );
	Check( TIER_GAMEPLAY, "restore.object-properties", Near( pNewMover->GetMass(), 40.0f, 0.01f ) && pNewMover->GetGameData() == &gameData[0] &&
		pNewMover->GetGameFlags() == 0x33 && pNewMover->GetContents() == ( CONTENTS_SOLID | CONTENTS_MONSTER ) &&
		( pNewMover->GetCallbackFlags() & CALLBACK_GLOBAL_TOUCH_STATIC ) && pNewMover->GetMaterialIndex() == wood &&
		Near( gotDamping, 0.3f, 1e-4f ) && Near( gotRot, 0.4f, 1e-4f ) && NearVec( pNewMover->GetInertia(), moverInertia, moverInertia.x * 0.01f ) &&
		pNewMover->GetCollide() == pCube && pNewMover->IsCollisionEnabled(),
		"mass %.2f flags %x material %d", pNewMover->GetMass(), pNewMover->GetGameFlags(), pNewMover->GetMaterialIndex() );
	Check( TIER_GAMEPLAY, "restore.object-pinned", !pNewPinned->IsMotionEnabled() && !pNewPinned->IsMoveable() );
	Check( TIER_GAMEPLAY, "restore.object-asleep", pNewSleeper->IsAsleep() );

	IPhysicsShadowController *pShadow = pNewShadowed->GetShadowController();
	Vector target;
	float maxSpeed = 0, maxAngular = 0;
	if ( pShadow )
	{
		pShadow->GetTargetPosition( &target, NULL );
		pShadow->GetMaxSpeed( &maxSpeed, &maxAngular );
	}
	Check( TIER_GAMEPLAY, "restore.shadow", pShadow && pShadow->AllowsTranslation() && !pShadow->AllowsRotation() &&
		Near( maxSpeed, 300.0f, 0.1f ) && Near( maxAngular, 200.0f, 0.1f ) && NearVec( target, Vector( 650, 0, 300 ), 0.1f ),
		"shadow %d speed %.1f/%.1f", pShadow != NULL, maxSpeed, maxAngular );

	Check( TIER_GAMEPLAY, "restore.constraint-links", pNewBall && pNewBall->GetReferenceObject() == restored.pFloor &&
		pNewBall->GetAttachedObject() == pNewPendulum );
	Check( TIER_GAMEPLAY, "restore.spring-links", pNewSpring && pNewSpring->GetStartObject() == pNewPinned && pNewSpring->GetEndObject() == pNewSleeper );
	IPhysicsObject *pAttached[4] = {};
	int attachedCount = pNewMotion ? pNewMotion->CountObjects() : 0;
	if ( pNewMotion && attachedCount <= 4 )
		pNewMotion->GetObjects( pAttached );
	Check( TIER_GAMEPLAY, "restore.motion-links", attachedCount == 1 && pAttached[0] == pNewMover, "count %d", attachedCount );

	// The restored constraint still holds: the pendulum keeps its anchor.
	if ( pNewMotion )
		pNewMotion->SetEventHandler( &motion );
	Step( pEnv, 0.5f );
	Vector local;
	pNewPendulum->WorldToLocal( &local, Vector( 800, 0, 300 ) );
	Check( TIER_GAMEPLAY, "restore.constraint-holds", NearVec( local, pendulumLocal, 2.0f ) && PositionOf( pNewPendulum ).z < 299.0f,
		"pivot local (%.2f %.2f %.2f)", local.x, local.y, local.z );
	Check( TIER_GAMEPLAY, "restore.motion-runs", motion.m_calls > 0, "calls %d", motion.m_calls );

	if ( pNewMotion )
		pEnv->DestroyMotionController( pNewMotion );
	if ( pNewSpring )
		pEnv->DestroySpring( pNewSpring );
	if ( pNewBall )
		pEnv->DestroyConstraint( pNewBall );
	if ( records[5].pNew )
		pEnv->DestroyConstraintGroup( (IPhysicsConstraintGroup *)records[5].pNew );
	DestroyWorld( restored );
	s_pCollision->DestroyCollide( pCube );
}
