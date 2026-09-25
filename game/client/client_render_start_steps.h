//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The client's render-start simulation (OnRenderStart) as declared
//          frame nodes (RFC 0003 R21/R30).
//
//          Each host node is OnRenderStart's original statement block, in its
//          original order. The previous-frame bone setup and the new particle
//          effects' simulation are each a gather host node, a batch of
//          independent items, and a commit host node; the batch replaces
//          their internal ParallelProcess / job-graph dispatch.
//
//          Declarations: legacy blocks call into arbitrary entity and game
//          system code, so every host node that is not a cohort's own gather
//          or commit declares FRAME_DOMAIN_ALL (unaudited). A batch declares
//          what its items touch. The resulting edges leave no pair of nodes
//          unordered: each batch's gather is immediately followed by its
//          commit in the host order, so no host node can run beside it
//          (the declared frame graph test checks this for this table).
//
//          This header is included by game/client/cdll_client_int.cpp after
//          the client declarations it uses, and by the render-start oracle
//          test with recording stand-ins of the same names.
//
//=============================================================================//

#ifndef CLIENT_RENDER_START_STEPS_H
#define CLIENT_RENDER_START_STEPS_H

#include "jobsystem/declared_frame_graph.h"

// Resource domains of the render-start region.
enum ClientRenderStartDomain_t
{
	RS_DOMAIN_ANIMATION_STATE = 1, // entity sequences, cycles, pose parameters, layers
	RS_DOMAIN_ENTITY_TRANSFORMS,   // abs origins/angles, move parents
	RS_DOMAIN_BONE_CACHE,          // bone caches and the previous-frame setup list
	RS_DOMAIN_PARTICLE_STATE,      // new particle effects' collections and the sim list
	RS_DOMAIN_WORLD_QUERIES,       // world/partition traces from particle operators
};

// OnRenderStart's blocks. A class so that the classes that befriend
// OnRenderStart can befriend its blocks too.
class CRenderStartSteps
{
public:
	static void Begin( void * );
	static void Interpolate( void * );
	static void InvalidateBones( void * );
	static void Camera( void * );
	static void View( void * );
	static void Ropes( void * );
	static void ClientSideAnimations( void * );
	static void DataChanged( void * );
	static void ResetOverlays( void * );
	static void Simulate( void * );
	static void TempEnts( void * );
	static void AimEnts( void * );
	static void Tools( void * );
	static void Replay( void * );
	static void VisibleEntities( void * );
	static void Bones( void * );
	static void BonesGather( void * );
	static unsigned BonesCount( void * );
	static void BonesItem( void *, unsigned iItem );
	static void BonesRunnerBegin( void * );
	static void BonesRunnerEnd( void * );
	static void BonesCommit( void * );
	static void Particles( void * );
	static void ParticlesGather( void * );
	static unsigned ParticlesCount( void * );
	static void ParticlesItem( void *, unsigned iItem );
	static void ParticlesCommit( void * );
	static void RunHostNodes( const jobsystem::FrameNodeDesc *pNodes, int nNodes );
};

//-----------------------------------------------------------------------------
// Host blocks (OnRenderStart's statement blocks, unchanged)
//-----------------------------------------------------------------------------

inline void CRenderStartSteps::Begin( void * )
{
#ifdef PORTAL
	g_pPortalRender->UpdatePortalPixelVisibility(); //updating this one or two lines before querying again just isn't cutting it. Update as soon as it's cheap to do so.
#endif

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );
	C_BaseEntity::SetAbsQueriesValid( false );

	Rope_ResetCounters();
}

inline void CRenderStartSteps::Interpolate( void * )
{
	// Interpolate server entities and move aiments.
	{
		PREDICTION_TRACKVALUECHANGESCOPE( "interpolation" );
		C_BaseEntity::InterpolateServerEntities();
	}
}

inline void CRenderStartSteps::InvalidateBones( void * )
{
	{
		// vprof node for this bloc of math
		VPROF( "OnRenderStart: dirty bone caches");
		// Invalidate any bone information.
		C_BaseAnimating::InvalidateBoneCaches();

		C_BaseEntity::SetAbsQueriesValid( true );
		C_BaseEntity::EnableAbsRecomputations( true );

		// Enable access to all model bones except view models.
		// This is necessary for aim-ent computation to occur properly
		C_BaseAnimating::PushAllowBoneAccess( true, false, "OnRenderStart->CViewRender::SetUpView" ); // pops in CViewRender::SetUpView

		// FIXME: This needs to be done before the player moves; it forces
		// aiments the player may be attached to to forcibly update their position
		C_BaseEntity::MarkAimEntsDirty();
	}
}

inline void CRenderStartSteps::Camera( void * )
{
	// Make sure the camera simulation happens before OnRenderStart, where it's used.
	// NOTE: the only thing that happens in CAM_Think is thirdperson related code.
	input->CAM_Think();
}

inline void CRenderStartSteps::View( void * )
{
	// This will place the player + the view models + all parent
	// entities	at the correct abs position so that their attachment points
	// are at the correct location
	view->OnRenderStart();
}

inline void CRenderStartSteps::Ropes( void * )
{
	RopeManager()->OnRenderStart();
}

inline void CRenderStartSteps::ClientSideAnimations( void * )
{
	// This will place all entities in the correct position in world space and in the KD-tree
	C_BaseAnimating::UpdateClientSideAnimations();

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );
}

inline void CRenderStartSteps::DataChanged( void * )
{
	// Process OnDataChanged events.
	ProcessOnDataChangedEvents();
}

inline void CRenderStartSteps::ResetOverlays( void * )
{
	// Reset the overlay alpha. Entities can change the state of this in their think functions.
	g_SmokeFogOverlayAlpha = 0;

	// This must occur prior to SimulatEntities,
	// which is where the client thinks for c_colorcorrection + c_colorcorrectionvolumes
	// update the color correction weights.
	// FIXME: The place where IGameSystem::Update is called should be in here
	// so we don't have to explicitly call ResetColorCorrectionWeights + SimulateEntities, etc.
	g_pColorCorrectionMgr->ResetColorCorrectionWeights();
}

inline void CRenderStartSteps::Simulate( void * )
{
	// Simulate all the entities.
	SimulateEntities();
	PhysicsSimulate();
}

inline void CRenderStartSteps::TempEnts( void * )
{
	{
		VPROF_("Client TempEnts", 0, VPROF_BUDGETGROUP_CLIENT_SIM, false, BUDGETFLAG_CLIENT);
		// This creates things like temp entities.
		engine->FireEvents();

		// Update temp entities
		tempents->Update();

		// Update temp ent beams...
		beams->UpdateTempEntBeams();

		// Lock the frame from beam additions
		SetBeamCreationAllowed( false );
	}
}

inline void CRenderStartSteps::AimEnts( void * )
{
	// Now that the view model's position is setup and aiments are marked dirty, update
	// their positions so they're in the leaf system correctly.
	C_BaseEntity::CalcAimEntPositions();
}

inline void CRenderStartSteps::Tools( void * )
{
	// For entities marked for recording, post bone messages to IToolSystems
	if ( ToolsEnabled() )
	{
		C_BaseEntity::ToolRecordEntities();
	}
}

inline void CRenderStartSteps::Replay( void * )
{
#if defined( REPLAY_ENABLED )
	// This will record any ragdolls if Replay mode is enabled on the server
	CReplayRagdollRecorder::Instance().Think();
	CReplayRagdollCache::Instance().Think();
#endif
}

inline void CRenderStartSteps::VisibleEntities( void * )
{
	// Finally, link all the entities into the leaf system right before rendering.
	C_BaseEntity::AddVisibleEntities();
}

//-----------------------------------------------------------------------------
// Previous-frame bone setup (C_BaseAnimating::ThreadedBoneSetup)
//-----------------------------------------------------------------------------

// Legacy mode: the whole cohort, with its own dispatch (cl_threaded_bone_setup,
// cl_bone_job_graph).
inline void CRenderStartSteps::Bones( void * )
{
	C_BaseAnimating::ThreadedBoneSetup();
}

inline void CRenderStartSteps::BonesGather( void * )
{
	C_BaseAnimating::ThreadedBoneSetupBegin();
}
inline unsigned CRenderStartSteps::BonesCount( void * )
{
	return C_BaseAnimating::ThreadedBoneSetupCount();
}
inline void CRenderStartSteps::BonesItem( void *, unsigned iItem )
{
	C_BaseAnimating::ThreadedBoneSetupItem( iItem );
}
inline void CRenderStartSteps::BonesRunnerBegin( void * )
{
	C_BaseAnimating::ThreadedBoneSetupRunnerBegin();
}
inline void CRenderStartSteps::BonesRunnerEnd( void * )
{
	C_BaseAnimating::ThreadedBoneSetupRunnerEnd();
}
inline void CRenderStartSteps::BonesCommit( void * )
{
	C_BaseAnimating::ThreadedBoneSetupEnd();
}

//-----------------------------------------------------------------------------
// New particle effects (CParticleMgr::Simulate)
//-----------------------------------------------------------------------------

// Legacy mode: the whole cohort, with its own dispatch (r_threaded_particles,
// r_particle_job_graph).
inline void CRenderStartSteps::Particles( void * )
{
	// Update particle effects (eventually, the effects should use Simulate() instead of having
	// their own update system).
	{
		// Enable FP exceptions here when FP_EXCEPTIONS_ENABLED is defined,
		// to help track down bad math.
		FPExceptionEnabler enableExceptions;
		VPROF_BUDGET( "ParticleMgr()->Simulate", VPROF_BUDGETGROUP_PARTICLE_SIMULATION );
		ParticleMgr()->Simulate( gpGlobals->frametime );
	}
}

inline void CRenderStartSteps::ParticlesGather( void * )
{
	// Update particle effects (eventually, the effects should use Simulate() instead of having
	// their own update system).
	{
		// Enable FP exceptions here when FP_EXCEPTIONS_ENABLED is defined,
		// to help track down bad math.
		FPExceptionEnabler enableExceptions;
		VPROF_BUDGET( "ParticleMgr()->Simulate", VPROF_BUDGETGROUP_PARTICLE_SIMULATION );
		ParticleMgr()->SimulateBegin( gpGlobals->frametime );
	}
}
inline unsigned CRenderStartSteps::ParticlesCount( void * )
{
	return ParticleMgr()->SimulateBatchCount();
}
inline void CRenderStartSteps::ParticlesItem( void *, unsigned iItem )
{
	ParticleMgr()->SimulateBatchItem( iItem );
}
inline void CRenderStartSteps::ParticlesCommit( void * )
{
	{
		FPExceptionEnabler enableExceptions;
		VPROF_BUDGET( "ParticleMgr()->Simulate", VPROF_BUDGETGROUP_PARTICLE_SIMULATION );
		ParticleMgr()->SimulateEnd();
	}
}

//-----------------------------------------------------------------------------
// Node table, in OnRenderStart's order
//-----------------------------------------------------------------------------

static const jobsystem::FrameAccess s_RenderStartUnaudited[] = {
	{ jobsystem::FRAME_DOMAIN_ALL, true },
};
// Items set up bones from animation state and transforms; each entity's bone
// cache is its own (C_BaseAnimating::SetupBones locks it per entity).
static const jobsystem::FrameAccess s_RenderStartBonesBatch[] = {
	{ RS_DOMAIN_ANIMATION_STATE, false },
	{ RS_DOMAIN_ENTITY_TRANSFORMS, false },
	{ RS_DOMAIN_BONE_CACHE, true },
};
static const jobsystem::FrameAccess s_RenderStartBonesGatherCommit[] = {
	{ RS_DOMAIN_BONE_CACHE, true },
};
// Items simulate their own collection; operators may trace the world and the
// partition. Control points were gathered before the batch.
static const jobsystem::FrameAccess s_RenderStartParticlesBatch[] = {
	{ RS_DOMAIN_PARTICLE_STATE, true },
	{ RS_DOMAIN_WORLD_QUERIES, false },
};
// Gather reads attachments (bones) and entity state for control points and
// updates dirty partition entries; commit publishes bounds to the leaf system.
static const jobsystem::FrameAccess s_RenderStartParticlesGatherCommit[] = {
	{ RS_DOMAIN_PARTICLE_STATE, true },
	{ RS_DOMAIN_WORLD_QUERIES, true },
	{ RS_DOMAIN_BONE_CACHE, true },
	{ RS_DOMAIN_ENTITY_TRANSFORMS, false },
};

#define RENDER_START_HOST( fn, access )                                                            \
	{ #fn, jobsystem::FRAME_NODE_HOST, NULL, &CRenderStartSteps::fn, NULL, NULL, NULL, NULL, access,     \
		ARRAYSIZE( access ) }

static const jobsystem::FrameNodeDesc s_RenderStartNodes[] = {
	RENDER_START_HOST( Begin, s_RenderStartUnaudited ),
	RENDER_START_HOST( Interpolate, s_RenderStartUnaudited ),
	RENDER_START_HOST( InvalidateBones, s_RenderStartUnaudited ),
	RENDER_START_HOST( Camera, s_RenderStartUnaudited ),
	RENDER_START_HOST( View, s_RenderStartUnaudited ),
	RENDER_START_HOST( Ropes, s_RenderStartUnaudited ),
	RENDER_START_HOST( ClientSideAnimations, s_RenderStartUnaudited ),
	RENDER_START_HOST( DataChanged, s_RenderStartUnaudited ),
	RENDER_START_HOST( ResetOverlays, s_RenderStartUnaudited ),
	RENDER_START_HOST( Simulate, s_RenderStartUnaudited ),
	RENDER_START_HOST( BonesGather, s_RenderStartBonesGatherCommit ),
	{ "BonesBatch", jobsystem::FRAME_NODE_BATCH, NULL, NULL, &CRenderStartSteps::BonesCount,
		&CRenderStartSteps::BonesItem, &CRenderStartSteps::BonesRunnerBegin, &CRenderStartSteps::BonesRunnerEnd,
		s_RenderStartBonesBatch, ARRAYSIZE( s_RenderStartBonesBatch ) },
	RENDER_START_HOST( BonesCommit, s_RenderStartBonesGatherCommit ),
	RENDER_START_HOST( TempEnts, s_RenderStartUnaudited ),
	RENDER_START_HOST( ParticlesGather, s_RenderStartParticlesGatherCommit ),
	{ "ParticlesBatch", jobsystem::FRAME_NODE_BATCH, NULL, NULL, &CRenderStartSteps::ParticlesCount,
		&CRenderStartSteps::ParticlesItem, NULL, NULL, s_RenderStartParticlesBatch,
		ARRAYSIZE( s_RenderStartParticlesBatch ) },
	RENDER_START_HOST( ParticlesCommit, s_RenderStartParticlesGatherCommit ),
	RENDER_START_HOST( AimEnts, s_RenderStartUnaudited ),
	RENDER_START_HOST( Tools, s_RenderStartUnaudited ),
	RENDER_START_HOST( Replay, s_RenderStartUnaudited ),
	RENDER_START_HOST( VisibleEntities, s_RenderStartUnaudited ),
};

// The same blocks with each cohort as one legacy call (cl_render_start_graph 0).
static const jobsystem::FrameNodeDesc s_RenderStartLegacyNodes[] = {
	RENDER_START_HOST( Begin, s_RenderStartUnaudited ),
	RENDER_START_HOST( Interpolate, s_RenderStartUnaudited ),
	RENDER_START_HOST( InvalidateBones, s_RenderStartUnaudited ),
	RENDER_START_HOST( Camera, s_RenderStartUnaudited ),
	RENDER_START_HOST( View, s_RenderStartUnaudited ),
	RENDER_START_HOST( Ropes, s_RenderStartUnaudited ),
	RENDER_START_HOST( ClientSideAnimations, s_RenderStartUnaudited ),
	RENDER_START_HOST( DataChanged, s_RenderStartUnaudited ),
	RENDER_START_HOST( ResetOverlays, s_RenderStartUnaudited ),
	RENDER_START_HOST( Simulate, s_RenderStartUnaudited ),
	RENDER_START_HOST( Bones, s_RenderStartUnaudited ),
	RENDER_START_HOST( TempEnts, s_RenderStartUnaudited ),
	RENDER_START_HOST( Particles, s_RenderStartUnaudited ),
	RENDER_START_HOST( AimEnts, s_RenderStartUnaudited ),
	RENDER_START_HOST( Tools, s_RenderStartUnaudited ),
	RENDER_START_HOST( Replay, s_RenderStartUnaudited ),
	RENDER_START_HOST( VisibleEntities, s_RenderStartUnaudited ),
};

#undef RENDER_START_HOST

// Runs a table of host nodes in order on the calling thread.
inline void CRenderStartSteps::RunHostNodes( const jobsystem::FrameNodeDesc *pNodes, int nNodes )
{
	for ( int i = 0; i < nNodes; i++ )
	{
		Assert( pNodes[i].kind == jobsystem::FRAME_NODE_HOST );
		pNodes[i].run( pNodes[i].context );
	}
}

#endif // CLIENT_RENDER_START_STEPS_H
