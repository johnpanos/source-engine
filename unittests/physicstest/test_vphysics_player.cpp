//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics provider conformance: the player controller (the
//          physics side of player movement: target tracking, teleport
//          correction, per-axis speed limits, push limits, ground-relative
//          motion, contact queries and attachment state).
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//
//          Every scenario drives the controller the way game movement does:
//          each tick the target advances by velocity * tick and Update is
//          called with secondsToArrival = one tick before the step.
//
//=============================================================================//

#include <math.h>
#include <string.h>

#include "vphysics_conformance.h"
#include "vphysics/player_controller.h"

namespace
{
const Vector kPlayerMins( -16, -16, 0 );
const Vector kPlayerMaxs( 16, 16, 72 );
const float kPlayerMass = 85.0f;

// Sensitivity: "player-inert" models a provider whose controller ignores
// game updates.
void Drive( IPhysicsPlayerController *pController, const Vector &target, const Vector &velocity, IPhysicsObject *pGround )
{
	if ( FaultIs( "player-inert" ) )
		return;
	pController->Update( target, velocity, kTick, pGround != NULL, pGround );
}

class CRefuseMoves : public IPhysicsPlayerControllerEvent
{
public:
	CRefuseMoves() : m_asked( 0 ) {}
	virtual int ShouldMoveTo( IPhysicsObject *pObject, const Vector &position )
	{
		m_asked++;
		return 0;
	}
	int m_asked;
};

struct Player_t
{
	CPhysCollide *pCollide;
	IPhysicsObject *pObject;
	IPhysicsPlayerController *pController;
};

bool CreatePlayer( World_t &world, const Vector &position, Player_t &player )
{
	player.pCollide = s_pCollision->BBoxToCollide( kPlayerMins, kPlayerMaxs );
	// As CBasePlayer::SetupVPhysicsShadow: no rotation, no drag.
	objectparams_t params = DefaultParams( kPlayerMass, NULL );
	params.inertia = 1e24f;
	params.dragCoefficient = 0;
	player.pObject = player.pCollide ? world.pEnv->CreatePolyObject( player.pCollide, world.material, position, vec3_angle, &params ) : NULL;
	player.pController = player.pObject ? world.pEnv->CreatePlayerController( player.pObject ) : NULL;
	return player.pController != NULL;
}

void DestroyPlayer( World_t &world, Player_t &player )
{
	if ( player.pController )
		world.pEnv->DestroyPlayerController( player.pController );
	if ( player.pObject )
		world.pEnv->DestroyObject( player.pObject );
	memset( &player, 0, sizeof( player ) );
}

// Walks for the given time; returns the final target.
Vector Walk( World_t &world, Player_t &player, Vector target, const Vector &velocity, float seconds, IPhysicsObject *pGround = NULL )
{
	int ticks = (int)( seconds / kTick + 0.5f );
	for ( int i = 0; i < ticks; i++ )
	{
		target += velocity * kTick;
		Drive( player.pController, target, velocity, pGround );
		Step( world.pEnv, kTick );
	}
	return target;
}

void TestAttachment()
{
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return;
	Player_t player;
	if ( !Check( TIER_GAMEPLAY, "player.create", CreatePlayer( world, Vector( 0, 0, 0 ), player ) ) )
	{
		DestroyWorld( world );
		return;
	}
	float speedDamping = 0, rotDamping = 0;
	player.pObject->GetDamping( &speedDamping, &rotDamping );
	bool flagged = ( player.pObject->GetCallbackFlags() & CALLBACK_IS_PLAYER_CONTROLLER ) != 0;
	Check( TIER_GAMEPLAY, "player.attach-state", flagged && player.pController->GetObject() == player.pObject &&
		Near( rotDamping, 100.0f, 1e-3f ) && !player.pObject->IsDragEnabled(), "flag %d rot %.2f drag %d", flagged, rotDamping,
		player.pObject->IsDragEnabled() );
	Check( TIER_GAMEPLAY, "player.default-limits", Near( player.pController->GetPushMassLimit(), VPHYSICS_MAX_MASS, 1.0f ) &&
		Near( player.pController->GetPushSpeedLimit(), 1e4f, 1.0f ) && !player.pController->WasFrozen() );
	player.pController->SetPushMassLimit( 123.0f );
	player.pController->SetPushSpeedLimit( 45.0f );
	Check( TIER_GAMEPLAY, "player.set-limits", player.pController->GetPushMassLimit() == 123.0f && player.pController->GetPushSpeedLimit() == 45.0f );

	// Moving the controller to another object moves the attachment.
	IPhysicsObject *pOther = CreateCube( world, Vector( 200, 0, 16 ) );
	float otherRot = 0;
	pOther->GetDamping( NULL, &otherRot );
	float originalRot = otherRot;
	player.pController->SetObject( pOther );
	pOther->GetDamping( NULL, &otherRot );
	float restoredRot = 0;
	player.pObject->GetDamping( NULL, &restoredRot );
	Check( TIER_GAMEPLAY, "player.set-object", player.pController->GetObject() == pOther &&
		( pOther->GetCallbackFlags() & CALLBACK_IS_PLAYER_CONTROLLER ) &&
		!( player.pObject->GetCallbackFlags() & CALLBACK_IS_PLAYER_CONTROLLER ) && Near( otherRot, 100.0f, 1e-3f ) &&
		Near( restoredRot, DefaultParams( kPlayerMass, NULL ).rotdamping, 1e-3f ),
		"other rot %.2f restored %.2f", otherRot, restoredRot );

	// Destroying the controller restores the object.
	world.pEnv->DestroyPlayerController( player.pController );
	player.pController = NULL;
	pOther->GetDamping( NULL, &otherRot );
	Check( TIER_GAMEPLAY, "player.detach-restores", !( pOther->GetCallbackFlags() & CALLBACK_IS_PLAYER_CONTROLLER ) &&
		Near( otherRot, originalRot, 1e-3f ), "rot %.2f expected %.2f", otherRot, originalRot );
	world.pEnv->DestroyObject( pOther );
	DestroyPlayer( world, player );
	DestroyWorld( world );
}

void TestWalking()
{
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return;
	Player_t player;
	if ( !CreatePlayer( world, Vector( -200, 0, 0 ), player ) )
	{
		DestroyWorld( world );
		return;
	}
	Step( world.pEnv, 0.25f );
	Vector start = PositionOf( player.pObject );
	Vector velocity( 200, 0, 0 );
	Vector target = Walk( world, player, start, velocity, 1.0f );
	Vector end = PositionOf( player.pObject );
	// Only the requested axis is controlled (the others have no speed budget).
	Check( TIER_GAMEPLAY, "player.tracks-target", Near( end.x, target.x, 8.0f ) && Near( end.y, start.y, 4.0f ),
		"end (%.2f %.2f %.2f) target (%.2f %.2f %.2f)", end.x, end.y, end.z, target.x, target.y, target.z );
	float walked = end.x - start.x;
	ObsFloats( "player.walk-displacement", "a4", 1, &walked );

	// The per-step velocity change is limited per axis by the request.
	Vector impulse;
	player.pController->GetLastImpulse( &impulse );
	Check( TIER_GAMEPLAY, "player.impulse-limited", fabsf( impulse.x ) <= velocity.x + 0.1f && fabsf( impulse.y ) < 1.0f,
		"impulse (%.3f %.3f %.3f)", impulse.x, impulse.y, impulse.z );

	Vector shadowVelocity;
	player.pController->GetShadowVelocity( &shadowVelocity );
	Check( TIER_GAMEPLAY, "player.shadow-velocity", Near( shadowVelocity.x, velocity.x, 60.0f ), "(%.2f %.2f %.2f)",
		shadowVelocity.x, shadowVelocity.y, shadowVelocity.z );
	Vector shadowPosition;
	QAngle shadowAngles;
	player.pController->GetShadowPosition( &shadowPosition, &shadowAngles );
	Check( TIER_GAMEPLAY, "player.shadow-position", NearVec( shadowPosition, end, 8.0f ) );

	// Zero input releases the object to physics: it coasts rather than
	// tracking (or teleporting to) the far target.
	Vector stopAt = PositionOf( player.pObject );
	for ( int i = 0; i < 60; i++ )
	{
		Drive( player.pController, stopAt + Vector( 500, 0, 0 ), vec3_origin, NULL );
		Step( world.pEnv, kTick );
	}
	Vector released = PositionOf( player.pObject );
	Check( TIER_GAMEPLAY, "player.zero-velocity-releases", released.x < stopAt.x + 150.0f, "moved %.2f", released.x - stopAt.x );

	// StepUp raises the object by exactly the step.
	Vector before = PositionOf( player.pObject );
	player.pController->StepUp( 18.0f );
	Vector after = PositionOf( player.pObject );
	Check( TIER_GAMEPLAY, "player.step-up", Near( after.z - before.z, 18.0f, 0.01f ) && Near( after.x, before.x, 0.01f ),
		"dz %.3f", after.z - before.z );
	DestroyPlayer( world, player );
	DestroyWorld( world );
}

void TestTeleport()
{
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return;
	Player_t player;
	if ( !CreatePlayer( world, Vector( 0, 0, 0 ), player ) )
	{
		DestroyWorld( world );
		return;
	}
	Step( world.pEnv, 0.25f );
	// A target past the 24-unit error bound is reached by teleport.
	Vector far = PositionOf( player.pObject ) + Vector( 0, 150, 0 );
	Drive( player.pController, far, Vector( 0, 10, 0 ), NULL );
	Step( world.pEnv, kTick );
	Vector at = PositionOf( player.pObject );
	Check( TIER_GAMEPLAY, "player.teleports-far", NearVec( at, far, 2.0f ), "at (%.2f %.2f %.2f)", at.x, at.y, at.z );

	// The event handler can refuse the teleport.
	CRefuseMoves refuse;
	player.pController->SetEventHandler( &refuse );
	Vector refused = PositionOf( player.pObject ) + Vector( 150, 0, 0 );
	Drive( player.pController, refused, Vector( 10, 0, 0 ), NULL );
	Step( world.pEnv, kTick );
	at = PositionOf( player.pObject );
	Check( TIER_GAMEPLAY, "player.handler-refuses", refuse.m_asked > 0 && at.x < refused.x - 100.0f, "asked %d x %.2f", refuse.m_asked,
		at.x - ( refused.x - 150.0f ) );
	player.pController->SetEventHandler( NULL );
	DestroyPlayer( world, player );
	DestroyWorld( world );
}

// CBasePlayer::SetVCollisionState (noclip off, ducking, a portal teleport)
// teleports the shadow and sets its velocity; movement then targets that same
// position. IVP holds a game-set velocity in IVP_Core::speed_change until the
// next step, and Update's speed budget excludes it: the controller removes at
// most the offered speed per axis, so the shadow keeps the rest for the step.
void TestCollisionStateVelocity()
{
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return;
	Player_t player;
	if ( !CreatePlayer( world, Vector( 0, 0, 400 ), player ) )
	{
		DestroyWorld( world );
		return;
	}
	Step( world.pEnv, kTick );
	Vector at = PositionOf( player.pObject );
	Vector velocity( 500, -600, 700 );
	player.pObject->SetPosition( at, vec3_angle, true );
	player.pObject->SetVelocity( &velocity, NULL );
	// Airborne, the game offers the controller its full speed on every axis.
	const float kBudget = 175.0f;
	Drive( player.pController, at, Vector( kBudget, kBudget, kBudget ), NULL );
	Step( world.pEnv, kTick );
	Vector kept( velocity.x - kBudget, velocity.y + kBudget, velocity.z - kBudget );
	Vector moved = PositionOf( player.pObject ) - at;
	Vector shadowVelocity;
	player.pController->GetShadowVelocity( &shadowVelocity );
	Check( TIER_GAMEPLAY, "player.set-velocity-budget", NearVec( shadowVelocity, kept, 20.0f ) &&
		NearVec( moved, kept * kTick, 1.0f ), "moved (%.2f %.2f %.2f) velocity (%.2f %.2f %.2f)", moved.x, moved.y,
		moved.z, shadowVelocity.x, shadowVelocity.y, shadowVelocity.z );
	ObsVector( "player.set-velocity-step", "a1", moved );
	DestroyPlayer( world, player );
	DestroyWorld( world );
}

// The player walks into a 50 kg crate. At an over-limit contact the push
// velocity is clamped: the player's own speed drops to zero on contact;
// within the limit it keeps pushing at walking speed. Returns the lowest
// player speed over the first ticks in contact.
float PushCrate( float massLimit, bool *pInContact )
{
	*pInContact = false;
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return -1.0f;
	Player_t player;
	float slowest = -1.0f;
	if ( CreatePlayer( world, Vector( -150, 0, 0 ), player ) )
	{
		IPhysicsObject *pCrate = CreateCube( world, Vector( -100, 0, 16 ), 50.0f );
		player.pController->SetPushMassLimit( massLimit );
		Step( world.pEnv, 0.25f );
		Vector target = PositionOf( player.pObject ), velocity( 100, 0, 0 );
		int contactTicks = 0;
		for ( int i = 0; i < (int)( 0.45f / kTick ) && contactTicks < 4; i++ )
		{
			target += velocity * kTick;
			Drive( player.pController, target, velocity, NULL );
			Step( world.pEnv, kTick );
			if ( PositionOf( pCrate ).x > -99.5f || contactTicks )
			{
				// The crate has been reached: sample while in contact.
				float speed = VelocityOf( player.pObject ).x;
				slowest = contactTicks ? MIN( slowest, speed ) : speed;
				contactTicks++;
			}
			*pInContact = *pInContact || player.pController->IsInContact();
		}
		world.pEnv->DestroyObject( pCrate );
	}
	DestroyPlayer( world, player );
	DestroyWorld( world );
	return slowest;
}

void TestPushing()
{
	bool pushContact = false, heldContact = false;
	float pushed = PushCrate( 1000.0f, &pushContact );
	float held = PushCrate( 10.0f, &heldContact );
	Check( TIER_GAMEPLAY, "player.push-mass-limit", pushed > 40.0f && held >= 0.0f && held < 5.0f, "slowest pushing %.2f held %.2f", pushed, held );
	Check( TIER_GAMEPLAY, "player.in-contact", pushContact );
	// The pushing speed at first contact depends on the contact tick; the
	// clamped speed is compared.
	ObsFloats( "player.push-held-speed", "a5", 1, &held );

	// Standing alone on static ground is not contact with a simulated object.
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return;
	Player_t player;
	if ( CreatePlayer( world, Vector( 0, 0, 0 ), player ) )
	{
		Step( world.pEnv, 0.5f );
		Check( TIER_GAMEPLAY, "player.static-not-contact", !player.pController->IsInContact() );
	}
	DestroyPlayer( world, player );
	DestroyWorld( world );
}

// Standing on a platform carried at constant velocity (a translating ground,
// so its velocity is the same at every point).
void TestGround()
{
	World_t world;
	if ( !CreateWorld( world, NULL ) )
		return;
	CPhysCollide *pDeck = s_pCollision->BBoxToCollide( Vector( -128, -128, -8 ), Vector( 128, 128, 0 ) );
	objectparams_t params = DefaultParams( 1000.0f, NULL );
	IPhysicsObject *pPlatform = world.pEnv->CreatePolyObject( pDeck, world.material, Vector( 0, 0, 100 ), vec3_angle, &params );
	pPlatform->EnableGravity( false );
	pPlatform->SetInertia( Vector( 1e8f, 1e8f, 1e8f ) );
	Vector deckVelocity( 0, 60, 0 );
	pPlatform->SetVelocity( &deckVelocity, NULL );
	Player_t player;
	if ( CreatePlayer( world, Vector( 0, 0, 100 ), player ) )
	{
		Step( world.pEnv, kTick );
		Vector start = PositionOf( player.pObject );
		// Walk slowly along x relative to the platform.
		Vector velocity( 20, 0, 0 );
		Vector target = start;
		for ( int i = 0; i < 60; i++ )
		{
			pPlatform->SetVelocity( &deckVelocity, NULL );
			target = PositionOf( player.pObject ) + velocity * kTick;
			if ( i == 0 )
				target = start + velocity * kTick;
			Drive( player.pController, target, velocity, pPlatform );
			Step( world.pEnv, kTick );
		}
		// Only the walked axis has a speed budget, so any carry comes from
		// friction (IVP's player hops on its contacts, which makes that carry
		// history-dependent). The ground contract itself is exact: shadow
		// velocity is reported relative to the ground under the target.
		Vector relative, own, deck;
		player.pController->GetShadowVelocity( &relative );
		player.pObject->GetVelocity( &own, NULL );
		pPlatform->GetVelocity( &deck, NULL );
		Check( TIER_GAMEPLAY, "player.ground-relative-velocity", NearVec( relative, own - deck, 1.0f ),
			"shadow (%.2f %.2f %.2f) own (%.2f %.2f %.2f) ground (%.2f %.2f %.2f)", relative.x, relative.y, relative.z,
			own.x, own.y, own.z, deck.x, deck.y, deck.z );
		Vector walked = PositionOf( player.pObject ) - start;
		Check( TIER_GAMEPLAY, "player.walks-on-ground", walked.x > 2.0f, "walked %.2f", walked.x );

		// Deleting the ground under the controller is safe.
		world.pEnv->DestroyObject( pPlatform );
		pPlatform = NULL;
		Walk( world, player, PositionOf( player.pObject ), velocity, 0.25f );
		Check( TIER_GAMEPLAY, "player.ground-deleted", IsFiniteVec( PositionOf( player.pObject ) ) );
	}
	DestroyPlayer( world, player );
	if ( pPlatform )
		world.pEnv->DestroyObject( pPlatform );
	s_pCollision->DestroyCollide( pDeck );
	DestroyWorld( world );
}
}

void TestPlayerController()
{
	TestAttachment();
	TestWalking();
	TestTeleport();
	TestCollisionStateVelocity();
	TestPushing();
	TestGround();
}
