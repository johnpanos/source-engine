//========= F-Stop port =======================================================//
//
// Purpose: Client side of the F-Stop blob NPCs; see c_npc_surface.h.
//
//=============================================================================//

#include "cbase.h"
#include "c_npc_surface.h"
#include "blob_networkbypass.h"
#include "dt_utlvector_recv.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "blobulator/Implicit/ImpTiler.h"
#include "vstdlib/jobgraph_parallel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// CS:GO c_surfacerender.cpp defaults: the mesh resolution, the lone-particle
// surface radius and the field reach, in multiples of the NPC's sphere radius.
static ConVar r_surface_draw_isosurface( "r_surface_draw_isosurface", "1", FCVAR_CHEAT, "Draw the blob NPCs as an isosurface" );
static ConVar r_surface_blr_scale( "r_surface_blr_scale", "1.0", FCVAR_CHEAT, "Scale all blob surface rendering parameters" );
static ConVar r_surface_blr_cubewidth( "r_surface_blr_cubewidth", "0.8", FCVAR_CHEAT, "Blob surface cube width (coarseness of the mesh)" );
static ConVar r_surface_blr_render_radius( "r_surface_blr_render_radius", "1.3", FCVAR_CHEAT, "How far from a particle centre the blob surface is" );
static ConVar r_surface_blr_cutoff_radius( "r_surface_blr_cutoff_radius", "3.3", FCVAR_CHEAT, "How far a blob particle's field extends" );
// The staged blue bounce-gel material keeps Portal 2's color on the supported
// VertexLitGeneric family; Portal 2's PaintBlob shader is not ported here.
static ConVar r_surface_shader( "r_surface_shader", "fstop/blob_surface_bounce",
	FCVAR_CHEAT, "Material the blob NPCs draw with" );
static ConVar r_surface_blr_parallel( "r_surface_blr_parallel", "1", 0,
    "Polygonize the blob surface's tiles on the compute pool (0: one after another on the "
    "main thread; the surface is the same)" );

namespace
{
// The tiler's parallel-for over the engine's compute pool. The tiler joins the
// tiles' triangles in tile order afterwards, so the order they run in does not
// matter.
struct BlobTileBatch
{
	void *pContext;
	void ( *pProcess )( void *, int );
	void Process( int &nTile ) { pProcess( pContext, nTile ); }
};

void BlobTilesParallelFor( void *pContext, int nCount, void ( *pProcess )( void *, int ) )
{
	CUtlVector<int> tiles;
	tiles.SetCount( nCount );
	for ( int i = 0; i < nCount; ++i )
		tiles[i] = i;
	BlobTileBatch batch = { pContext, pProcess };
	// False only before any tile ran (no pool, a bad batch): run them here.
	if ( !JobGraphParallelProcess( "C_NPC_Surface::PolygonizeTiles", tiles.Base(), (unsigned)nCount,
	         &batch, &BlobTileBatch::Process ) )
	{
		for ( int i = 0; i < nCount; ++i )
			pProcess( pContext, i );
	}
}
} // namespace

IMPLEMENT_CLIENTCLASS_DT( C_NPC_Surface, DT_NPC_Surface, CNPC_Surface )
	RecvPropUtlVector( RECVINFO_UTLVECTOR( m_iParticlePositionIndex ), MAX_SURFACE_ELEMENTS,
		RecvPropInt( NULL, 0, sizeof( uint16 ) ) ),
	RecvPropInt( RECVINFO( m_nActiveParticles ) ),
	RecvPropFloat( RECVINFO( m_flRadius ) ),
END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT( C_NPC_BlobFountain, DT_NPC_BlobFountain, CNPC_BlobFountain )
END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT( C_NPC_BlobDemoMonster, DT_NPC_BlobDemoMonster, CNPC_BlobDemoMonster )
	RecvPropUtlVector( RECVINFO_UTLVECTOR( m_flSurfaceV ), MAX_SURFACE_ELEMENTS,
		RecvPropFloat( NULL, 0, sizeof( float ) ) ),
END_RECV_TABLE()

// Per-draw particle scratch, shared by all blobs (drawn one at a time on the
// render thread that owns the entity draw).
static CUtlVector<ImpParticleWithOneInterpolant, CUtlMemoryAligned<ImpParticleWithOneInterpolant, 16> > s_Particles;

C_NPC_Surface::C_NPC_Surface()
	: m_nActiveParticles( 0 ), m_flRadius( 0.0f ), m_vecRenderMins( vec3_origin ), m_vecRenderMaxs( vec3_origin )
{
}

void C_NPC_Surface::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
	if ( updateType == DATA_UPDATE_CREATED )
	{
		Warning( "F-Stop blob created: active=%d indices=%d radius=%.2f\n",
			m_nActiveParticles, m_iParticlePositionIndex.Count(), m_flRadius );
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

int C_NPC_Surface::GatherParticles()
{
	s_Particles.RemoveAll();
	if ( !g_pBlobNetworkBypass )
		return 0;

	const Vector &vecOrigin = GetRenderOrigin();
	int nCount = MIN( m_nActiveParticles, m_iParticlePositionIndex.Count() );
	for ( int i = 0; i < nCount; ++i )
	{
		int nIndex = m_iParticlePositionIndex[i];
		if ( nIndex < 0 || nIndex >= BLOB_MAX_LEVEL_PARTICLES
			|| !g_pBlobNetworkBypass->bCurrentlyInUse.IsBitSet( nIndex ) )
			continue;
		float flRadius = BLOBPARTICLERADIUS_INTERP( nIndex );
		if ( flRadius <= 0.0f )
			continue;
		ImpParticleWithOneInterpolant &particle = s_Particles[s_Particles.AddToTail()];
		particle.center = Point3D( BLOBPARTICLEPOS_INTERP( nIndex ) - vecOrigin );
		particle.setFieldScale( flRadius );
		particle.interpolants1 = Point3D( 1.0f, 1.0f, 1.0f );
	}
	return s_Particles.Count();
}

// The particles wander from the NPC's origin, so the render bounds follow them
// every frame (the field reaches the cutoff radius past each centre).
void C_NPC_Surface::ClientThink()
{
	BaseClass::ClientThink();

	int nCount = GatherParticles();
	if ( nCount == 0 )
	{
		m_vecRenderMins = m_vecRenderMaxs = vec3_origin;
		RemoveFromLeafSystem();
		UpdateVisibility();
		return;
	}
	Vector mins( FLT_MAX, FLT_MAX, FLT_MAX ), maxs( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	for ( int i = 0; i < nCount; ++i )
	{
		const Vector &center = s_Particles[i].center.AsVector();
		float flReach = m_flRadius * r_surface_blr_scale.GetFloat() * r_surface_blr_cutoff_radius.GetFloat()
			* s_Particles[i].scale;
		mins = mins.Min( center - Vector( flReach, flReach, flReach ) );
		maxs = maxs.Max( center + Vector( flReach, flReach, flReach ) );
	}
	m_vecRenderMins = mins;
	m_vecRenderMaxs = maxs;
	CollisionProp()->MarkSurroundingBoundsDirty();
	AddToLeafSystem();
	UpdateVisibility();
}

void C_NPC_Surface::GetRenderBounds( Vector &mins, Vector &maxs )
{
	mins = m_vecRenderMins;
	maxs = m_vecRenderMaxs;
}

int C_NPC_Surface::DrawModel( int flags )
{
	static bool s_bReportedDraw = false;
	if ( !s_bReportedDraw )
	{
		Warning( "F-Stop blob draw: active=%d indices=%d bypass=%s radius=%.2f\n",
			m_nActiveParticles, m_iParticlePositionIndex.Count(),
			g_pBlobNetworkBypass ? "yes" : "no", m_flRadius );
		s_bReportedDraw = true;
	}
	if ( !r_surface_draw_isosurface.GetBool() || m_flRadius <= 0.0f )
		return 0;
	int nCount = GatherParticles();
	if ( nCount == 0 )
		return 0;

	IMaterial *pMaterial = materials->FindMaterial( r_surface_shader.GetString(), TEXTURE_GROUP_OTHER, true );
	if ( !pMaterial || IsErrorMaterial( pMaterial ) )
		return 0;

	const Vector &vecOrigin = GetRenderOrigin();
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->Bind( pMaterial, this );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();
	pRenderContext->Translate( vecOrigin.x, vecOrigin.y, vecOrigin.z );

	float flScale = m_flRadius * r_surface_blr_scale.GetFloat();
	ImpTiler *pTiler = ImpTilerFactory::factory->getTiler();
	pTiler->SetCubeWidth( flScale * r_surface_blr_cubewidth.GetFloat() );
	pTiler->SetRenderRadius( flScale * r_surface_blr_render_radius.GetFloat() );
	pTiler->SetCutoffRadius( flScale * r_surface_blr_cutoff_radius.GetFloat() );
	IMatRenderContext *pContext = pRenderContext;
	pTiler->SetRenderContext( &pContext );
	pTiler->SetParallelFor( r_surface_blr_parallel.GetBool() ? &BlobTilesParallelFor : NULL );

	pTiler->beginFrame( Point3D( 0.0f, 0.0f, 0.0f ), true, false );
	for ( int i = 0; i < nCount; ++i )
		pTiler->insertParticle( &s_Particles[i] );
	pTiler->drawSurface( false );
	pTiler->endFrame( false );
	ImpTilerFactory::factory->returnTiler( pTiler );

	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->PopMatrix();
	return 1;
}
