//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Isolated restitution scenes for the gap corpus (RFC 0005 Q0,
//          RFC 0004 Q-PHYSICS).
//
//          With the Box3D restitution patch (fork johnpanos/box3d 78c90a0,
//          "only touching points bounce") three gameplay checks fail, but only
//          inside the whole parity run. Each is a scene here, run for one
//          provider per process and one scene per process:
//
//            corner      vcollide.model-simulates: an authored model dropped
//                        from z 200 at QAngle( 10, 20, 5 ), landing on a
//                        corner (test_vphysics_conformance.cpp, TestPhyFixture,
//                        lines 1033-1109)
//            tumble      dynamics.tumble-travel-bounded: the cube dropped 64
//                        units at QAngle( 25, 0, 15 ) (test_vphysics_dynamics.cpp,
//                        TestDrop, lines 364-398)
//            held-floor  dynamics.held-floor-quiet: the cube held by the grab
//                        emulation 8 units below the floor (test_vphysics_dynamics.cpp,
//                        TestHeld, lines 887-913)
//            rest        analytic control: the cube released at rest on the
//                        floor must not gain height
//
//          The shared host and the dynamics helpers are compiled into this
//          translation unit unchanged, so the scenes use the parity suite's
//          provider loading, fault decorators, worlds, authored cube, grab
//          emulation and settle rule. Only the scene bodies are re-authored,
//          with their parameters exposed so the oracle can vary them around
//          the canonical values. This program measures; the verdict belongs to
//          tools/quality/corpus/physics_restitution.py, which compares each run
//          with the recorded IVP ranges.
//
//          Output: the host's boot checks (PASS/FAIL lines), one
//          `RUN key=value...` record, and the `CONFORMANCE` line of the boot
//          checks. Exit 0 when the scene ran.
//
//=============================================================================//

// The shared host: module loading, fault decorators, reporting, World_t.
// Its main() is renamed; this file provides the entry point.
#define main VPhysicsConformanceMain
#include "../test_vphysics_conformance.cpp"
#undef main
// The dynamics helpers (feel world, authored cube, grab emulation).
#include "../test_vphysics_dynamics.cpp"

#include <float.h>

namespace
{
struct SceneArgs_t
{
	const char *pScene;
	bool heightSet;
	float height; // corner: drop z; tumble: drop above rest; held-floor: start z; rest: above rest
	bool anglesSet;
	QAngle angles;
	float offsetX, offsetY;
	float depth; // held-floor: target depth below the rest height
	bool trace;
};

// Seeded provider fault for the negative controls: elasticity-max gives the
// scene's bodies a surface property with elasticity 100, which both providers
// clamp to a fully elastic impact. It changes the provider's behavior, never
// the oracle.
int s_superball = -1;

void PrepareFaults()
{
	if ( !FaultIs( "elasticity-max" ) )
		return;
	static const char kSuperball[] =
	    "\"corpus_superball\"\n{\n\t\"base\" \"metal\"\n\t\"elasticity\" \"100\"\n}\n";
	s_pProps->ParseSurfaceData( "corpus_superball.txt", kSuperball );
	s_superball = s_pProps->GetSurfaceIndex( "corpus_superball" );
}

void ApplyMaterialFault( IPhysicsObject *pObject )
{
	if ( pObject && s_superball >= 0 )
		pObject->SetMaterialIndex( s_superball );
}

int ContactCount( IPhysicsObject *pObject )
{
	int count = 0;
	IPhysicsFrictionSnapshot *pSnapshot = pObject->CreateFrictionSnapshot();
	while ( pSnapshot->IsValid() )
	{
		count++;
		pSnapshot->NextFrictionData();
	}
	pObject->DestroyFrictionSnapshot( pSnapshot );
	return count;
}

// Floor slabs: World_t's is 1024 units square (CreateWorld), the feel
// world's 4096 (CreateFeelWorld). A body that leaves one may fall past it.
const float kHostFloorHalf = 512.0f;
const float kFeelFloorHalf = 2048.0f;

bool OverFloor( const Vector &position, float half )
{
	return fabsf( position.x ) < half && fabsf( position.y ) < half;
}

// Lowest point of a body's collision geometry in world space: below 0 it is
// in the floor, whatever its origin height.
float GeometryMinZ( IPhysicsObject *pObject )
{
	Vector position, mins, maxs;
	QAngle angles;
	pObject->GetPosition( &position, &angles );
	s_pCollision->CollideGetAABB( &mins, &maxs, pObject->GetCollide(), position, angles );
	return mins.z;
}

// Per-tick measurements of one body.
struct Trajectory_t
{
	IPhysicsObject *pObject;
	float dt, gravity;
	Vector start;
	int tick;
	bool finite;
	float maxAngular, maxSpeed;
	int firstContactTick, contacts, contactsMax;
	// Rebounds of the center of mass after the first contact: a rise of more
	// than kReboundRise from a trough to the next peak.
	int bounces;
	float apexFirst, apexMax, trough, prevZ;
	bool rising;
	// Mechanical energy (kinetic, rotational, potential above z = 0).
	float energy0, energyMax, energyMinSinceContact, energyRise;
	float maxZGain;
	float floorHalf; // half extent of the floor slab: geometry depth counts only over it
	float geomMinZ;  // lowest collision-geometry point seen over the floor
	int sleepTick;
	int calm, settledTick;

	static float Energy( IPhysicsObject *pObject, float gravity )
	{
		float mass = pObject->GetMass();
		Vector velocity = VelocityOf( pObject );
		AngularImpulse w = AngularVelocityOf( pObject );
		Vector inertia = pObject->GetInertia();
		float wx = DEG2RAD( w.x ), wy = DEG2RAD( w.y ), wz = DEG2RAD( w.z );
		float rotational =
		    0.5f * ( inertia.x * wx * wx + inertia.y * wy * wy + inertia.z * wz * wz );
		return 0.5f * mass * velocity.LengthSqr() + rotational +
		       mass * gravity * PositionOf( pObject ).z;
	}

	void Init( IPhysicsObject *pBody, float step, float g, float half )
	{
		floorHalf = half;
		pObject = pBody;
		dt = step;
		gravity = g;
		start = PositionOf( pBody );
		tick = 0;
		finite = true;
		maxAngular = maxSpeed = 0.0f;
		firstContactTick = -1;
		contacts = contactsMax = 0;
		bounces = 0;
		apexFirst = apexMax = 0.0f;
		trough = prevZ = start.z;
		rising = false;
		energy0 = energyMax = Energy( pBody, g );
		energyMinSinceContact = FLT_MAX;
		energyRise = 0.0f;
		maxZGain = 0.0f;
		geomMinZ = FLT_MAX;
		sleepTick = -1;
		calm = 0;
		settledTick = -1;
	}

	void Update()
	{
		const float kReboundRise = 0.25f;
		Vector position = PositionOf( pObject );
		Vector velocity = VelocityOf( pObject );
		float angular = AngularSpeedOf( pObject );
		finite &= IsFiniteVec( position ) && IsFiniteVec( velocity );
		maxAngular = MAX( maxAngular, angular );
		maxSpeed = MAX( maxSpeed, velocity.Length() );
		maxZGain = MAX( maxZGain, position.z - start.z );
		if ( OverFloor( position, floorHalf ) )
			geomMinZ = MIN( geomMinZ, GeometryMinZ( pObject ) );
		contacts = ContactCount( pObject );
		contactsMax = MAX( contactsMax, contacts );
		if ( firstContactTick < 0 && contacts > 0 )
		{
			firstContactTick = tick;
			trough = position.z;
			rising = false;
		}
		float energy = Energy( pObject, gravity );
		energyMax = MAX( energyMax, energy );
		if ( firstContactTick >= 0 )
		{
			energyMinSinceContact = MIN( energyMinSinceContact, energy );
			energyRise = MAX( energyRise, energy - energyMinSinceContact );
			float dz = position.z - prevZ;
			if ( dz > 1e-4f && !rising )
			{
				rising = true;
				trough = MIN( trough, prevZ );
			}
			else if ( dz < -1e-4f && rising )
			{
				// prevZ was a peak.
				float rise = prevZ - trough;
				if ( rise > kReboundRise )
				{
					if ( !bounces )
						apexFirst = rise;
					apexMax = MAX( apexMax, rise );
					bounces++;
				}
				rising = false;
				trough = position.z;
			}
			else if ( !rising )
				trough = MIN( trough, position.z );
		}
		prevZ = position.z;
		// SettleTracker_t's rule: speed < 1 and spin < 5 deg/s for ten ticks.
		if ( velocity.Length() < 1.0f && angular < 5.0f )
		{
			if ( ++calm == 10 && settledTick < 0 )
				settledTick = tick + 1 - 10;
		}
		else
		{
			calm = 0;
			settledTick = -1;
		}
		if ( sleepTick < 0 && pObject->IsAsleep() )
			sleepTick = tick;
		tick++;
	}

	float SettleSeconds() const { return settledTick < 0 ? -1.0f : settledTick * dt; }
	float SleepSeconds() const { return sleepTick < 0 ? -1.0f : sleepTick * dt; }
};

// One `RUN` record: key=value pairs, printed in order.
struct Record_t
{
	char text[4096];
	int length;
	void Init( const char *pScene )
	{
		length = V_snprintf( text, sizeof( text ), "RUN scene=%s fault=%s", pScene,
		    s_pFault[0] ? s_pFault : "none" );
	}
	void Add( const char *pKey, float value )
	{
		length += V_snprintf( text + length, sizeof( text ) - length, " %s=%.4f", pKey, value );
	}
	void AddTrajectory( const Trajectory_t &t )
	{
		Vector position = PositionOf( t.pObject );
		Add( "finite", t.finite ? 1.0f : 0.0f );
		Add( "final_x", position.x );
		Add( "final_y", position.y );
		Add( "final_z", position.z );
		Add( "final_speed", SpeedOf( t.pObject ) );
		Add( "final_angular", AngularSpeedOf( t.pObject ) );
		Add( "final_tilt", TiltFromFlat( t.pObject ) );
		Add( "max_angular", t.maxAngular );
		Add( "max_speed", t.maxSpeed );
		Add( "settle_s", t.SettleSeconds() );
		Add( "sleep_s", t.SleepSeconds() );
		Add( "first_contact_s", t.firstContactTick < 0 ? -1.0f : t.firstContactTick * t.dt );
		Add( "bounces", (float)t.bounces );
		Add( "apex_first", t.apexFirst );
		Add( "apex_max", t.apexMax );
		Add( "energy_ratio_max", t.energy0 > 0.0f ? t.energyMax / t.energy0 : 0.0f );
		Add( "energy_rise", t.energyRise );
		Add( "energy0", t.energy0 );
		Add( "z_gain", t.maxZGain );
		Add( "geom_min_z", GeometryMinZ( t.pObject ) );
		Add( "geom_min_z_seen", t.geomMinZ < FLT_MAX ? t.geomMinZ : 0.0f );
		Add( "over_floor", OverFloor( position, t.floorHalf ) ? 1.0f : 0.0f );
		Add( "contacts_final", (float)t.contacts );
		Add( "contacts_max", (float)t.contactsMax );
	}
	void Print() { printf( "%s\n", text ); }
};

bool LoadPhy( const char *pPath, vcollide_t &collide )
{
	memset( &collide, 0, sizeof( collide ) );
	int size = 0;
	char *pData = ReadFixtureFile( pPath, &size );
	if ( !Check( TIER_BOOT, "restitution.read-phy",
	         pData != NULL && size > (int)sizeof( phyheader_t ), "%s", pPath ) )
	{
		free( pData );
		return false;
	}
	phyheader_t header;
	memcpy( &header, pData, sizeof( header ) );
	s_pCollision->VCollideLoad(
	    &collide, header.solidCount, pData + sizeof( header ), size - (int)sizeof( header ) );
	free( pData );
	return Check( TIER_BOOT, "restitution.load-phy",
	    collide.solidCount == header.solidCount && collide.solids && collide.pKeyValues,
	    "%s: %d solids", pPath, collide.solidCount );
}

void TraceTick( const char *pScene, int tick, IPhysicsObject *pObject, bool enabled )
{
	if ( !enabled )
		return;
	Vector p = PositionOf( pObject );
	Vector v = VelocityOf( pObject );
	printf( "TICK %s %d z %.4f pos %.4f %.4f vz %.4f speed %.4f w %.4f tilt %.3f contacts %d\n",
	    pScene, tick, p.z, p.x, p.y, v.z, v.Length(), AngularSpeedOf( pObject ),
	    TiltFromFlat( pObject ), ContactCount( pObject ) );
}

//-----------------------------------------------------------------------------
// corner: vcollide.model-simulates. Every solid as the game creates it,
// dropped onto the host's floor (World_t: gravity 600, tick 0.015) and
// stepped 5 seconds. Aggregates over solids are the worst solid.
//-----------------------------------------------------------------------------
void RunCorner( const SceneArgs_t &args, const vcollide_t &vc )
{
	const float dropZ = args.heightSet ? args.height : 200.0f;
	const QAngle angles = args.anglesSet ? args.angles : QAngle( 10, 20, 5 );
	World_t world;
	CreateWorld( world, NULL );
	CUtlVector<IPhysicsObject *> objects;
	objects.SetCount( vc.solidCount );
	for ( int i = 0; i < vc.solidCount; i++ )
		objects[i] = NULL;
	IVPhysicsKeyParser *pParser = s_pCollision->VPhysicsKeyParserCreate( vc.pKeyValues );
	while ( pParser && !pParser->Finished() )
	{
		if ( V_stricmp( pParser->GetCurrentBlockName(), "solid" ) )
		{
			pParser->SkipBlock();
			continue;
		}
		solid_t solid;
		memset( &solid, 0, sizeof( solid ) );
		pParser->ParseSolid( &solid, NULL );
		if ( solid.index < 0 || solid.index >= vc.solidCount || objects[solid.index] )
			continue;
		objectparams_t params =
		    DefaultParams( solid.params.mass > 0 ? solid.params.mass : 1.0f, NULL );
		params.inertia = solid.params.inertia > 0 ? solid.params.inertia : 1.0f;
		params.damping = solid.params.damping;
		params.rotdamping = solid.params.rotdamping;
		params.volume = solid.params.volume;
		int material = s_pProps->GetSurfaceIndex( solid.surfaceprop );
		Vector drop = vc.solidCount > 1 ? Vector( ( solid.index % 6 ) * 150.0f - 375.0f,
		                                      ( solid.index / 6 ) * 150.0f - 225.0f, dropZ )
		                                : Vector( args.offsetX, args.offsetY, dropZ );
		objects[solid.index] = world.pEnv->CreatePolyObject(
		    vc.solids[solid.index], material >= 0 ? material : 0, drop, angles, &params );
		if ( objects[solid.index] )
		{
			ApplyMaterialFault( objects[solid.index] );
			objects[solid.index]->Wake();
		}
	}
	if ( pParser )
		s_pCollision->VPhysicsKeyParserDestroy( pParser );

	CUtlVector<Trajectory_t> tracks;
	int created = 0;
	for ( int i = 0; i < vc.solidCount; i++ )
	{
		if ( !objects[i] )
			continue;
		created++;
		Trajectory_t track;
		track.Init( objects[i], kTick, 600.0f, kHostFloorHalf );
		tracks.AddToTail( track );
	}
	// Step( pEnv, 5.0f ), one tick at a time so the trajectory is sampled.
	const int ticks = (int)( 5.0f / kTick + 0.5f );
	for ( int i = 0; i < ticks; i++ )
	{
		world.pEnv->Simulate( kTick );
		for ( int t = 0; t < tracks.Count(); t++ )
		{
			tracks[t].Update();
		}
		if ( tracks.Count() )
			TraceTick( "corner", i, tracks[0].pObject, args.trace );
	}

	// The check's rule (test_vphysics_conformance.cpp 1087-1108).
	bool finite = true, rested = true;
	int worst = -1, unrested = 0, firstUnrested = -1, offFloor = 0;
	float worstSpeed = -1.0f, minZ = FLT_MAX, maxZ = -FLT_MAX, minGeomZ = FLT_MAX;
	for ( int t = 0; t < tracks.Count(); t++ )
	{
		IPhysicsObject *pObject = tracks[t].pObject;
		Vector position = PositionOf( pObject );
		float speed = VelocityOf( pObject ).Length();
		if ( speed >= worstSpeed )
		{
			worstSpeed = speed;
			worst = t;
		}
		minZ = MIN( minZ, position.z );
		if ( OverFloor( position, kHostFloorHalf ) )
			minGeomZ = MIN( minGeomZ, GeometryMinZ( pObject ) );
		else
			offFloor++;
		maxZ = MAX( maxZ, position.z );
		finite &= IsFiniteVec( position ) && IsFiniteVec( VelocityOf( pObject ) );
		bool restedHere =
		    position.z > -1.0f && position.z < 150.0f && ( vc.solidCount > 1 || speed < 20.0f );
		rested &= restedHere;
		if ( !restedHere )
		{
			unrested++;
			if ( firstUnrested < 0 )
				firstUnrested = t;
		}
		if ( args.trace )
			printf( "SOLID %d z %.4f geom_min_z %.4f speed %.4f travel %.4f max_angular %.4f "
			        "bounces %d apex %.4f %s\n",
			    t, position.z, GeometryMinZ( pObject ), speed,
			    ( position - tracks[t].start ).AsVector2D().Length(), tracks[t].maxAngular,
			    tracks[t].bounces, tracks[t].apexMax, restedHere ? "rested" : "UNRESTED" );
	}
	// The first solid that breaks the rule, else the fastest at the end.
	if ( firstUnrested >= 0 )
		worst = firstUnrested;

	Record_t record;
	record.Init( "corner" );
	record.Add( "solids", (float)vc.solidCount );
	record.Add( "created", (float)created );
	record.Add( "rested", rested ? 1.0f : 0.0f );
	record.Add( "all_finite", finite ? 1.0f : 0.0f );
	record.Add( "worst_speed", worstSpeed );
	record.Add( "unrested", (float)unrested );
	record.Add( "min_final_z", minZ );
	record.Add( "max_final_z", maxZ );
	// The rule above tests origins; a limb resting on another face can put
	// its origin below the floor while its geometry rests on it.
	record.Add( "min_geom_final_z", minGeomZ < FLT_MAX ? minGeomZ : 0.0f );
	record.Add( "off_floor", (float)offFloor );
	// Trajectory of the first solid that breaks the rest rule, else of the
	// fastest at the end (the only one for a single-solid model).
	if ( worst >= 0 )
	{
		const Trajectory_t &t = tracks[worst];
		record.Add( "focus_solid", (float)worst );
		record.Add( "travel", ( PositionOf( t.pObject ) - t.start ).AsVector2D().Length() );
		record.AddTrajectory( t );
	}
	record.Print();
	DestroyWorld( world );
}

//-----------------------------------------------------------------------------
// tumble: dynamics.tumble-travel-bounded. The cube dropped 64 units above
// its rest height at QAngle( 25, 0, 15 ) onto the feel world's concrete floor
// (Portal 2 tick 1/60, gravity 600), stepped 360 ticks.
//-----------------------------------------------------------------------------
void RunTumble( const SceneArgs_t &args, const CubeModel_t &model )
{
	const float restZ = RestHeight( model );
	const float dropHeight = args.heightSet ? args.height : 64.0f;
	const QAngle angles = args.anglesSet ? args.angles : QAngle( 25, 0, 15 );
	FeelWorld_t world;
	CreateFeelWorld( world );
	IPhysicsObject *pCube = CreateFeelCube(
	    world, model, Vector( args.offsetX, args.offsetY, restZ + dropHeight ), angles );
	ApplyMaterialFault( pCube );
	Trajectory_t track;
	track.Init( pCube, kDynTick, kGravity, kFeelFloorHalf );
	for ( int i = 0; i < 360; i++ )
	{
		Tick( world.pEnv );
		track.Update();
		TraceTick( "tumble", i, pCube, args.trace );
	}
	Vector rest = PositionOf( pCube );
	Record_t record;
	record.Init( "tumble" );
	record.Add( "rest_height", restZ );
	record.Add( "travel", ( rest - track.start ).AsVector2D().Length() );
	record.Add( "rest_offset_z", rest.z - restZ );
	record.Add( "impacts", (float)world.events.m_impacts );
	record.Add( "audible_impacts", (float)world.events.m_audibleImpacts );
	record.AddTrajectory( track );
	record.Print();
	DestroyFeelWorld( world );
}

//-----------------------------------------------------------------------------
// held-floor: dynamics.held-floor-quiet. The cube picked up at z 60 by the
// grab emulation (mass 1, rotational damping 10) whose target is 8 units
// below the cube's rest height, 180 ticks; path length and peak spin over
// ticks 60-179 measure buzzing.
//-----------------------------------------------------------------------------
void RunHeldFloor( const SceneArgs_t &args, const CubeModel_t &model )
{
	const float startZ = args.heightSet ? args.height : 60.0f;
	const QAngle angles = args.anglesSet ? args.angles : vec3_angle;
	const float depth = args.depth;
	FeelWorld_t world;
	CreateFeelWorld( world );
	IPhysicsObject *pCube =
	    CreateFeelCube( world, model, Vector( args.offsetX, args.offsetY, startZ ), angles );
	ApplyMaterialFault( pCube );
	HeldCube_t held;
	Grab( world, held, pCube );
	const Vector target( args.offsetX, args.offsetY, RestHeight( model ) - depth );
	float travel = 0.0f, maxAngular = 0.0f, maxSpeed = 0.0f, minZ = FLT_MAX, maxZ = -FLT_MAX;
	int contactTicks = 0;
	Vector last = PositionOf( pCube );
	Trajectory_t track;
	track.Init( pCube, kDynTick, kGravity, kFeelFloorHalf );
	for ( int i = 0; i < 180; i++ )
	{
		held.grab.SetTarget( target, angles );
		Tick( world.pEnv );
		track.Update();
		Vector pos = PositionOf( pCube );
		if ( i >= 60 )
		{
			travel += ( pos - last ).Length();
			maxAngular = MAX( maxAngular, AngularSpeedOf( pCube ) );
			maxSpeed = MAX( maxSpeed, SpeedOf( pCube ) );
			minZ = MIN( minZ, pos.z );
			maxZ = MAX( maxZ, pos.z );
			contactTicks += track.contacts > 0 ? 1 : 0;
		}
		last = pos;
		TraceTick( "held-floor", i, pCube, args.trace );
	}
	Record_t record;
	record.Init( "held-floor" );
	record.Add( "travel", travel );
	record.Add( "quiet_angular", maxAngular );
	record.Add( "quiet_speed", maxSpeed );
	record.Add( "z_spread", maxZ - minZ );
	record.Add( "gap", PositionOf( pCube ).z - RestHeight( model ) );
	record.Add( "contact_ticks", (float)contactTicks );
	record.Add( "impacts", (float)world.events.m_impacts );
	record.AddTrajectory( track );
	record.Print();
	world.pEnv->DestroyMotionController( held.pController );
	DestroyFeelWorld( world );
}

//-----------------------------------------------------------------------------
// rest: analytic control. The cube released with zero velocity on the floor
// (its origin `height` above the rest height, 0 by default) must not gain
// height or energy, and must stay still.
//-----------------------------------------------------------------------------
void RunRest( const SceneArgs_t &args, const CubeModel_t &model )
{
	const float restZ = RestHeight( model );
	const float above = args.heightSet ? args.height : 0.0f;
	FeelWorld_t world;
	CreateFeelWorld( world );
	IPhysicsObject *pCube =
	    CreateFeelCube( world, model, Vector( args.offsetX, args.offsetY, restZ + above ),
	        args.anglesSet ? args.angles : vec3_angle );
	ApplyMaterialFault( pCube );
	Trajectory_t track;
	track.Init( pCube, kDynTick, kGravity, kFeelFloorHalf );
	for ( int i = 0; i < 120; i++ )
	{
		Tick( world.pEnv );
		track.Update();
		TraceTick( "rest", i, pCube, args.trace );
	}
	Record_t record;
	record.Init( "rest" );
	record.Add( "rest_height", restZ );
	record.Add( "travel", ( PositionOf( pCube ) - track.start ).AsVector2D().Length() );
	record.AddTrajectory( track );
	record.Print();
	DestroyFeelWorld( world );
}

bool ParseTriple( const char *pText, float out[3] )
{
	return sscanf( pText, "%f,%f,%f", &out[0], &out[1], &out[2] ) == 3;
}
} // namespace

int main( int argc, char **argv )
{
	const char *pProvider = NULL;
	const char *pPhy = NULL;
	CUtlVector<const char *> surfaceFiles;
	SceneArgs_t args;
	memset( &args, 0, sizeof( args ) );
	args.depth = 8.0f;
	for ( int i = 1; i < argc; i++ )
	{
		float triple[3];
		if ( !V_strcmp( argv[i], "--provider" ) && i + 1 < argc )
			pProvider = argv[++i];
		else if ( !V_strcmp( argv[i], "--surfaceprops" ) && i + 1 < argc )
			surfaceFiles.AddToTail( argv[++i] );
		else if ( !V_strcmp( argv[i], "--phy" ) && i + 1 < argc )
			pPhy = argv[++i];
		else if ( !V_strcmp( argv[i], "--scene" ) && i + 1 < argc )
			args.pScene = argv[++i];
		else if ( !V_strcmp( argv[i], "--height" ) && i + 1 < argc )
		{
			args.height = (float)atof( argv[++i] );
			args.heightSet = true;
		}
		else if ( !V_strcmp( argv[i], "--angles" ) && i + 1 < argc &&
		          ParseTriple( argv[i + 1], triple ) )
		{
			args.angles.Init( triple[0], triple[1], triple[2] );
			args.anglesSet = true;
			i++;
		}
		else if ( !V_strcmp( argv[i], "--offset" ) && i + 1 < argc &&
		          sscanf( argv[i + 1], "%f,%f", &args.offsetX, &args.offsetY ) == 2 )
			i++;
		else if ( !V_strcmp( argv[i], "--depth" ) && i + 1 < argc )
			args.depth = (float)atof( argv[++i] );
		else if ( !V_strcmp( argv[i], "--fault" ) && i + 1 < argc )
			s_pFault = argv[++i];
		else if ( !V_strcmp( argv[i], "--trace" ) )
			args.trace = true;
		else
		{
			fprintf( stderr,
			    "usage: %s --provider <lib.so> --surfaceprops <file>... --phy <model.phy> "
			    "--scene corner|tumble|held-floor|rest [--height h] [--angles p,y,r] [--offset "
			    "x,y] "
			    "[--depth d] [--fault elasticity-max] [--trace]\n",
			    argv[0] );
			return 2;
		}
	}
	const bool known =
	    args.pScene &&
	    ( !V_strcmp( args.pScene, "corner" ) || !V_strcmp( args.pScene, "tumble" ) ||
	        !V_strcmp( args.pScene, "held-floor" ) || !V_strcmp( args.pScene, "rest" ) );
	const bool knownFault = !s_pFault[0] || FaultIs( "elasticity-max" );
	if ( !pProvider || !surfaceFiles.Count() || !pPhy || !known || !knownFault )
	{
		fprintf(
		    stderr, "missing --provider, --surfaceprops or --phy, or unknown --scene/--fault\n" );
		return 2;
	}

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	printf( "PROVIDER %s\nFAULT %s\n", pProvider, s_pFault[0] ? s_pFault : "none" );
	bool ran = false;
	if ( TestModule( pProvider ) )
	{
		for ( int i = 0; i < surfaceFiles.Count(); i++ )
		{
			char *pText = ReadFixtureFile( surfaceFiles[i], NULL );
			if ( !Check( TIER_BOOT, "restitution.read-surfaceprops", pText != NULL, "%s",
			         surfaceFiles[i] ) )
				continue;
			s_pProps->ParseSurfaceData( surfaceFiles[i], pText );
			free( pText );
		}
		Check( TIER_BOOT, "restitution.surfaceprops",
		    s_pProps->GetSurfaceIndex( "default" ) == 0 &&
		        s_pProps->GetSurfaceIndex( "concrete" ) > 0,
		    "default %d concrete %d", s_pProps->GetSurfaceIndex( "default" ),
		    s_pProps->GetSurfaceIndex( "concrete" ) );
		PrepareFaults();
		if ( FaultIs( "elasticity-max" ) )
			Check( TIER_BOOT, "restitution.fault-material", s_superball > 0 );
		vcollide_t collide;
		if ( LoadPhy( pPhy, collide ) )
		{
			if ( !V_strcmp( args.pScene, "corner" ) )
			{
				RunCorner( args, collide );
				ran = true;
			}
			else
			{
				CubeModel_t model = LoadCubeModel( &collide );
				if ( Check( TIER_BOOT, "restitution.cube-model", model.valid,
				         "mass %.2f surfaceprop %s", model.params.mass, model.surfaceprop ) )
				{
					if ( !V_strcmp( args.pScene, "tumble" ) )
						RunTumble( args, model );
					else if ( !V_strcmp( args.pScene, "held-floor" ) )
						RunHeldFloor( args, model );
					else
						RunRest( args, model );
					ran = true;
				}
			}
			s_pCollision->VCollideUnload( &collide );
		}
		s_pPhysics->Shutdown();
		s_pPhysics->Disconnect();
	}
	Check( TIER_BOOT, "restitution.scene-ran", ran, "%s", args.pScene );
	int checks = 0, failed = 0;
	for ( int t = 0; t < TIER_COUNT; t++ )
	{
		checks += s_checks[t];
		failed += s_failed[t];
	}
	printf( "CONFORMANCE %d %d\n", checks, failed );
	fflush( stdout );
	return failed ? 1 : 0;
}
