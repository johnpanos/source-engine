//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client rendering for race-mode checkpoints.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_baseanimating.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imesh.h"
#include "portal2_engine_compat.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_race_checkpoint_inactive_color( "cl_race_checkpoint_inactive_color", "27 202 241 255", 0 );
ConVar cl_race_checkpoint_active_color( "cl_race_checkpoint_active_color", "215 155 54 255", 0 );

// Reconstruction note: neither 2010 client declares a client class or entity
// factory for this type, and the server entity sends no table of its own, so
// m_bCheckpointActive is only ever its initial value in those builds.
class C_PortalRaceCheckpoint : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_PortalRaceCheckpoint, C_BaseAnimating );

	C_PortalRaceCheckpoint();
	virtual ~C_PortalRaceCheckpoint();

	// Portal 2 port: CS:GO renderable API bridged onto DrawModel( int ).
	PORTAL2_DRAWMODEL_BRIDGE();
	virtual int DrawModel( int flags, const RenderableInstance_t &instance );
	virtual bool ShouldDraw( void );

private:

	bool InitMaterials( void );

	IMaterial		*m_pCheckpointMaterial;
	IMaterialVar	*m_pMyColor;

	bool			m_bCheckpointActive;
};

// Reconstruction note: the 2010 constructor initializes no members; client
// entity allocation zero-fills them (C_BaseEntity::operator new).
C_PortalRaceCheckpoint::C_PortalRaceCheckpoint()
{
}

C_PortalRaceCheckpoint::~C_PortalRaceCheckpoint()
{
}

bool C_PortalRaceCheckpoint::ShouldDraw( void )
{
	return true;
}

int C_PortalRaceCheckpoint::DrawModel( int flags, const RenderableInstance_t &instance )
{
	if ( !InitMaterials() )
		return 0;

	// Draw a camera-independent billboard in front of the hologram
	Vector vecOrigin = GetAbsOrigin();
	Vector vecForward, vecRight, vecUp;
	GetVectors( &vecForward, &vecRight, &vecUp );

	Vector vecVertex;

	vecOrigin -= vecForward * 8.0f;
	vecUp *= 60.0f;
	vecRight *= 35.0f;

	int r = 0, g = 0, b = 0, a = 255;
	sscanf( m_bCheckpointActive ? cl_race_checkpoint_active_color.GetString() : cl_race_checkpoint_inactive_color.GetString(), "%d %d %d %d", &r, &g, &b, &a );
	Color meshColor( r, g, b, a );

	float flColors[3] = { meshColor.r() / 255.0f, meshColor.g() / 255.0f, meshColor.b() / 255.0f };
	m_pMyColor->SetVecValue( flColors, 3 );

	float pColors[4];
	m_pMyColor->GetLinearVecValue( pColors, 3 );
	pColors[3] = 1.0f;

	SetRenderColor( meshColor.r(), meshColor.g(), meshColor.b() );

	CMatRenderContextPtr pRenderContext( materials );

	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_pCheckpointMaterial );
	CMeshBuilder meshBuilder;

	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	vecVertex = vecOrigin;
	vecVertex += vecUp;
	vecVertex -= vecRight;

	meshBuilder.Color4fv( pColors );
	meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
	meshBuilder.Position3fv( vecVertex.Base() );
	meshBuilder.AdvanceVertex();

	vecVertex = vecOrigin;
	vecVertex += vecUp;
	vecVertex += vecRight;

	meshBuilder.Color4fv( pColors );
	meshBuilder.TexCoord2f( 0, 1.0f, 0.0f );
	meshBuilder.Position3fv( vecVertex.Base() );
	meshBuilder.AdvanceVertex();

	vecVertex = vecOrigin;
	vecVertex -= vecUp;
	vecVertex += vecRight;

	meshBuilder.Color4fv( pColors );
	meshBuilder.TexCoord2f( 0, 0.0f, 1.0f );
	meshBuilder.Position3fv( vecVertex.Base() );
	meshBuilder.AdvanceVertex();

	vecVertex = vecOrigin;
	vecVertex -= vecUp;
	vecVertex -= vecRight;

	meshBuilder.Color4fv( pColors );
	meshBuilder.TexCoord2f( 0, 1.0f, 1.0f );
	meshBuilder.Position3fv( vecVertex.Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	return BaseClass::DrawModel( flags );
}

bool C_PortalRaceCheckpoint::InitMaterials( void )
{
	if ( m_pCheckpointMaterial == NULL )
	{
		m_pCheckpointMaterial = materials->FindMaterial( "effects/projected_wall", NULL, false );
		if ( m_pCheckpointMaterial == NULL )
			return false;
	}

	bool bFound = false;
	if ( m_pMyColor == NULL )
	{
		m_pMyColor = m_pCheckpointMaterial->FindVar( "$color", &bFound, true );
		return bFound;
	}

	return true;
}
