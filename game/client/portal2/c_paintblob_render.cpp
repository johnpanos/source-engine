//========= Portal 2 reconstruction ============================================//
//
// Purpose: Paint blob rendering (blobulator isosurface and fast spheres).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_paintblob_render.h"
#include "paint_sprayer_shared.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "view.h"
#include "viewrender.h"
#include "engine/IEngineTrace.h"
#include "engine/ivdebugoverlay.h"
#include "debugoverlay_shared.h"
#include "tier0/vprof.h"
#include "tier1/callqueue.h"
#include "tier1/utlenvelope.h"
#include "mathlib/ssemath.h"
#include "blobulator/Point3D.h"
#include "blobulator/SmartArray.h"
#include "blobulator/Implicit/ImpTiler.h"
#include "blobulator/Implicit/ImpRenderer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar mat_wireframe;

ConVar r_paintblob_draw_isosurface( "r_paintblob_draw_isosurface", "1", 0, "Draws the surface as an isosurface" );

// Rotation
ConVar r_paintblob_rotate( "r_paintblob_rotate", "0", 0, "Whether to rotate for transparency" );
ConVar r_paintblob_rotate_by90( "r_paintblob_rotate_by90", "0", 0, "Whether to only rotate in 90 degree increments" );

ConVar r_paintblob_blr_scale( "r_paintblob_blr_scale", "1.0", 0, "Scale all surface rendering parameters." );
ConVar r_paintblob_blr_cubewidth( "r_paintblob_blr_cubewidth", "0.8", 0, "Set cubewidth (coarseness of the mesh)" );
ConVar r_paintblob_blr_render_radius( "r_paintblob_blr_render_radius", "1.3", 0, "Set render radius (how far from particle center surface will be)" );
ConVar r_paintblob_blr_cutoff_radius( "r_paintblob_blr_cutoff_radius", "5.5", 0, "Set cutoff radius (how far field extends from each particle)" );

ConVar r_paintblob_calc_uv_and_tan( "r_paintblob_calc_uv_and_tan", "1", FCVAR_ARCHIVE, "Calculate UVs and Tangents" );
ConVar r_paintblob_calc_tan_only( "r_paintblob_calc_tan_only", "0", FCVAR_ARCHIVE, "Calculate Only Tangents" );
ConVar r_paintblob_calc_color( "r_paintblob_calc_color", "0", FCVAR_ARCHIVE, "Just interpolate colors" );
ConVar r_paintblob_calc_hifreq_color( "r_paintblob_calc_hifreq_color", "0", FCVAR_ARCHIVE, "Experimental hi-freq colors" );
ConVar r_paintblob_calc_tile_color( "r_paintblob_calc_tile_color", "0", FCVAR_ARCHIVE, "Shows color of the tile" );

ConVar r_paintblob_tile_index_to_draw( "r_paintblob_tile_index_to_draw", "-1", 0, "-1 to display all tiles. Otherwise the index of the tile to draw." );
ConVar r_paintblob_debug_draw_margin( "r_paintblob_debug_draw_margin", "0", 0, "If tiler is disabled, whether to draw the margin." );
ConVar r_paintblob_debug_draw_tile_boundaries( "r_paintblob_debug_draw_tile_boundaries", "0", 0, "Whether to draw outlines of all tiles." );

ConVar r_paintblob_wireframe( "r_paintblob_wireframe", "0", 0, "Draw wireframe" );
ConVar r_paintblob_material( "r_paintblob_material", "-1", 0, "Choose a material from 0 to N" );
ConVar r_paintblob_shader( "r_paintblob_shader", "", 0, "Choose a shader" );

ConVar r_paintblob_distance_fading_enabled( "r_paintblob_distance_fading_enabled", "1", 0 );
ConVar r_paintblob_start_fading_distance( "r_paintblob_start_fading_distance", "150.f", 0 );
ConVar r_paintblob_max_fading_distance( "r_paintblob_max_fading_distance", "300.f", 0 );

ConVar r_threaded_blobulator( "r_threaded_blobulator", "1", 0, "If enabled, blobulator will use material thread." );
ConVar r_no_low_res_blobs( "r_no_low_res_blobs", "1", 0, "If enabled, the blobs will never get their poly count reduced." );

ConVar r_paintblob_only_mainview_highres( "r_paintblob_only_mainview_highres", "1", 0, "If enabled, only the main view is high resolution, all other views are low res." );
ConVar r_paintblob_only_mainview_displayed( "r_paintblob_only_mainview_displayed", "0", 0, "If enabled, only the main view will be displayed. Shadows, reflection and refraction views won't have blobs visible." );

ConVar r_paintblob_force_single_pass( "r_paintblob_force_single_pass", "0", 0, "1 to force rendering of the blob in a single pass, 0 to render in two passes." );

BlobParticles_t g_PaintblobRenderParticles;
const QAngle g_PaintblobRenderAnglesAngles( 0.0f, 0.0f, 0.0f );

typedef CUtlEnvelope< ImpParticleWithFourInterpolants > BlobParticlesEnvelope_t;

namespace NPaintRenderer
{

//-----------------------------------------------------------------------------
// Low poly sphere used to draw the blobs when the isosurface is disabled.
// Each vertex is: position (unit sphere), texcoord, normal.
//-----------------------------------------------------------------------------
float g_FastSpherePosData[51][8] =
{
	{ 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
	{ -0.0f, 0.5f, 0.866f, 0.25f, 0.1667f, -0.0f, 0.5f, 0.866f },
	{ 0.5f, 0.0f, 0.866f, 0.0f, 0.1667f, 0.5f, 0.0f, 0.866f },
	{ 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f },
	{ -0.5f, -0.0f, 0.866f, 0.5f, 0.1667f, -0.5f, -0.0f, 0.866f },
	{ 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f },
	{ 0.5f, 0.0f, -0.866f, 0.0f, 0.8333f, 0.5f, 0.0f, -0.866f },
	{ -0.0f, 0.5f, -0.866f, 0.25f, 0.8333f, -0.0f, 0.5f, -0.866f },
	{ 0.0f, 0.0f, -1.0f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f },
	{ -0.5f, -0.0f, -0.866f, 0.5f, 0.8333f, -0.5f, -0.0f, -0.866f },
	{ 0.0f, -0.5f, 0.866f, 0.75f, 0.1667f, 0.0f, -0.5f, 0.866f },
	{ 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f },
	{ 0.5f, 0.0f, 0.866f, 1.0f, 0.1667f, 0.5f, 0.0f, 0.866f },
	{ 0.0f, -0.5f, -0.866f, 0.75f, 0.8333f, 0.0f, -0.5f, -0.866f },
	{ 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f },
	{ 0.5f, 0.0f, -0.866f, 1.0f, 0.8333f, 0.5f, 0.0f, -0.866f },
	{ 0.6124f, 0.6124f, 0.5f, 0.125f, 0.3333f, 0.6124f, 0.6124f, 0.5f },
	{ 0.866f, 0.0f, 0.5f, 0.0f, 0.3333f, 0.866f, 0.0f, 0.5f },
	{ -0.0f, 0.866f, 0.5f, 0.25f, 0.3333f, -0.0f, 0.866f, 0.5f },
	{ 0.866f, 0.0f, -0.5f, 0.0f, 0.6667f, 0.866f, 0.0f, -0.5f },
	{ 0.6124f, 0.6124f, -0.5f, 0.125f, 0.6667f, 0.6124f, 0.6124f, -0.5f },
	{ -0.0f, 0.866f, -0.5f, 0.25f, 0.6667f, -0.0f, 0.866f, -0.5f },
	{ -0.6124f, 0.6124f, 0.5f, 0.375f, 0.3333f, -0.6124f, 0.6124f, 0.5f },
	{ -0.866f, -0.0f, 0.5f, 0.5f, 0.3333f, -0.866f, -0.0f, 0.5f },
	{ -0.6124f, 0.6124f, -0.5f, 0.375f, 0.6667f, -0.6124f, 0.6124f, -0.5f },
	{ -0.866f, -0.0f, -0.5f, 0.5f, 0.6667f, -0.866f, -0.0f, -0.5f },
	{ -0.6124f, -0.6124f, 0.5f, 0.625f, 0.3333f, -0.6124f, -0.6124f, 0.5f },
	{ 0.0f, -0.866f, 0.5f, 0.75f, 0.3333f, 0.0f, -0.866f, 0.5f },
	{ -0.6124f, -0.6124f, -0.5f, 0.625f, 0.6667f, -0.6124f, -0.6124f, -0.5f },
	{ 0.0f, -0.866f, -0.5f, 0.75f, 0.6667f, 0.0f, -0.866f, -0.5f },
	{ 0.6124f, -0.6124f, 0.5f, 0.875f, 0.3333f, 0.6124f, -0.6124f, 0.5f },
	{ 0.866f, 0.0f, 0.5f, 1.0f, 0.3333f, 0.866f, 0.0f, 0.5f },
	{ 0.6124f, -0.6124f, -0.5f, 0.875f, 0.6667f, 0.6124f, -0.6124f, -0.5f },
	{ 0.866f, 0.0f, -0.5f, 1.0f, 0.6667f, 0.866f, 0.0f, -0.5f },
	{ 0.9239f, 0.3827f, 0.0f, 0.0625f, 0.5f, 0.9239f, 0.3827f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f },
	{ 0.7071f, 0.7071f, 0.0f, 0.125f, 0.5f, 0.7071f, 0.7071f, 0.0f },
	{ 0.3827f, 0.9239f, 0.0f, 0.1875f, 0.5f, 0.3827f, 0.9239f, 0.0f },
	{ -0.0f, 1.0f, 0.0f, 0.25f, 0.5f, -0.0f, 1.0f, 0.0f },
	{ -0.3827f, 0.9239f, 0.0f, 0.3125f, 0.5f, -0.3827f, 0.9239f, 0.0f },
	{ -0.7071f, 0.7071f, 0.0f, 0.375f, 0.5f, -0.7071f, 0.7071f, 0.0f },
	{ -0.9239f, 0.3827f, 0.0f, 0.4375f, 0.5f, -0.9239f, 0.3827f, 0.0f },
	{ -1.0f, -0.0f, 0.0f, 0.5f, 0.5f, -1.0f, -0.0f, 0.0f },
	{ -0.9239f, -0.3827f, 0.0f, 0.5625f, 0.5f, -0.9239f, -0.3827f, 0.0f },
	{ -0.7071f, -0.7071f, 0.0f, 0.625f, 0.5f, -0.7071f, -0.7071f, 0.0f },
	{ -0.3827f, -0.9239f, 0.0f, 0.6875f, 0.5f, -0.3827f, -0.9239f, 0.0f },
	{ 0.0f, -1.0f, 0.0f, 0.75f, 0.5f, 0.0f, -1.0f, 0.0f },
	{ 0.3827f, -0.9239f, 0.0f, 0.8125f, 0.5f, 0.3827f, -0.9239f, 0.0f },
	{ 0.7071f, -0.7071f, 0.0f, 0.875f, 0.5f, 0.7071f, -0.7071f, 0.0f },
	{ 0.9239f, -0.3827f, 0.0f, 0.9375f, 0.5f, 0.9239f, -0.3827f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f },
};

int g_FastSphereTriData[84][3] =
{
	{ 0, 1, 2 },
	{ 0, 3, 1 },
	{ 3, 4, 1 },
	{ 5, 6, 7 },
	{ 5, 7, 8 },
	{ 8, 7, 9 },
	{ 3, 10, 4 },
	{ 3, 11, 10 },
	{ 11, 12, 10 },
	{ 8, 9, 13 },
	{ 8, 13, 14 },
	{ 14, 13, 15 },
	{ 2, 16, 17 },
	{ 2, 1, 16 },
	{ 1, 18, 16 },
	{ 6, 19, 20 },
	{ 6, 20, 7 },
	{ 7, 20, 21 },
	{ 1, 22, 18 },
	{ 1, 4, 22 },
	{ 4, 23, 22 },
	{ 7, 21, 24 },
	{ 7, 24, 9 },
	{ 9, 24, 25 },
	{ 4, 26, 23 },
	{ 4, 10, 26 },
	{ 10, 27, 26 },
	{ 9, 25, 28 },
	{ 9, 28, 13 },
	{ 13, 28, 29 },
	{ 10, 30, 27 },
	{ 10, 12, 30 },
	{ 12, 31, 30 },
	{ 13, 29, 32 },
	{ 13, 32, 15 },
	{ 15, 32, 33 },
	{ 17, 34, 35 },
	{ 17, 16, 34 },
	{ 16, 36, 34 },
	{ 19, 35, 34 },
	{ 19, 34, 20 },
	{ 20, 34, 36 },
	{ 16, 37, 36 },
	{ 16, 18, 37 },
	{ 18, 38, 37 },
	{ 20, 36, 37 },
	{ 20, 37, 21 },
	{ 21, 37, 38 },
	{ 18, 39, 38 },
	{ 18, 22, 39 },
	{ 22, 40, 39 },
	{ 21, 38, 39 },
	{ 21, 39, 24 },
	{ 24, 39, 40 },
	{ 22, 41, 40 },
	{ 22, 23, 41 },
	{ 23, 42, 41 },
	{ 24, 40, 41 },
	{ 24, 41, 25 },
	{ 25, 41, 42 },
	{ 23, 43, 42 },
	{ 23, 26, 43 },
	{ 26, 44, 43 },
	{ 25, 42, 43 },
	{ 25, 43, 28 },
	{ 28, 43, 44 },
	{ 26, 45, 44 },
	{ 26, 27, 45 },
	{ 27, 46, 45 },
	{ 28, 44, 45 },
	{ 28, 45, 29 },
	{ 29, 45, 46 },
	{ 27, 47, 46 },
	{ 27, 30, 47 },
	{ 30, 48, 47 },
	{ 29, 46, 47 },
	{ 29, 47, 32 },
	{ 32, 47, 48 },
	{ 30, 49, 48 },
	{ 30, 31, 49 },
	{ 31, 50, 49 },
	{ 32, 48, 49 },
	{ 32, 49, 33 },
	{ 33, 49, 50 },
};


void Paintblob_DrawFastSphere( CMeshBuilder &meshBuilder, const Vector &center, float radius, float r, float g, float b )
{
	int offset = meshBuilder.GetCurrentVertex();

	Vector pos;
	for ( int i = 0; i < ARRAYSIZE( g_FastSpherePosData ); ++i )
	{
		const float *pVertData = g_FastSpherePosData[i];

		// Reconstruction note: the binary adds the table position to the center
		// as well as the scaled normal; kept as is.
		pos.x = pVertData[0] + center.x + radius * pVertData[5];
		pos.y = pVertData[1] + center.y + radius * pVertData[6];
		pos.z = pVertData[2] + center.z + radius * pVertData[7];

		meshBuilder.Position3fv( pos.Base() );
		meshBuilder.Normal3fv( &pVertData[5] );
		meshBuilder.TexCoord2fv( 0, &pVertData[3] );
		meshBuilder.Color3f( r, g, b );
		meshBuilder.AdvanceVertex();
	}

	for ( int i = 0; i < ARRAYSIZE( g_FastSphereTriData ); ++i )
	{
		meshBuilder.FastIndex( offset + g_FastSphereTriData[i][0] );
		meshBuilder.FastIndex( offset + g_FastSphereTriData[i][1] );
		meshBuilder.FastIndex( offset + g_FastSphereTriData[i][2] );
	}
}


struct PaintBlob_DrawSpheres_sortParticles_t
{
	int no;
	float dist;

	// Reconstruction note: the comparator's member names are not recorded; the
	// binary sorts the spheres back to front (largest distance first).
	struct C
	{
		static bool IsLessThan( const PaintBlob_DrawSpheres_sortParticles_t &a, const PaintBlob_DrawSpheres_sortParticles_t &b )
		{
			return a.dist > b.dist;
		}
	};
};


// Spheres per mesh batch (keeps the index count below 32768)
static const int MAX_SPHERES_PER_BATCH = 130;

void Paintblob_DrawSpheres( IMaterial *pMaterial, float flRadius )
{
	Point3D eye( view->GetViewSetup()->origin );

	SmartArray< PaintBlob_DrawSpheres_sortParticles_t, false, 16 > sort_particles;
	sort_particles.ensureCapacity( g_PaintblobRenderParticles.Count() );
	sort_particles.size = g_PaintblobRenderParticles.Count();
	for ( int i = 0; i < g_PaintblobRenderParticles.Count(); ++i )
	{
		sort_particles[i].no = i;
		sort_particles[i].dist = eye.length( g_PaintblobRenderParticles[i].center );
	}

	// draw back to front
	sort_particles.sort< PaintBlob_DrawSpheres_sortParticles_t::C >();

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->Bind( pMaterial );

	IMesh *pMesh = pRenderContext->GetDynamicMesh( true );

	int vertMax = MAX_SPHERES_PER_BATCH;

	int j = 0;
	while ( j < g_PaintblobRenderParticles.Count() )
	{
		int total = MIN( g_PaintblobRenderParticles.Count() - j, vertMax );

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, total * ARRAYSIZE( g_FastSpherePosData ), total * ARRAYSIZE( g_FastSphereTriData ) * 3 );

		int i = 0;
		while ( j < g_PaintblobRenderParticles.Count() && i < vertMax )
		{
			ImpParticleWithOneInterpolant *imp_particle = &g_PaintblobRenderParticles[ sort_particles[j].no ];
			if ( imp_particle->scale > 0.01 )
			{
				Paintblob_DrawFastSphere( meshBuilder, imp_particle->center.AsVector(), imp_particle->scale * flRadius,
					imp_particle->interpolants1[0], imp_particle->interpolants1[1], imp_particle->interpolants1[2] );
				++i;
			}
			++j;
		}

		meshBuilder.End();
		pMesh->Draw();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Blend the cached lighting cube towards the lighting at the render
//			origin, rejecting invalid samples
//-----------------------------------------------------------------------------
void Paintblob_SafeLightCubeUpdate( const Vector &vecRenderOrigin, Vector4D *cachedCubeColours )
{
	Vector boxColors[6];
	// Portal 2 port: IVEngineClient has no ComputeLightingCube here; see portal2_engine_compat.h.
	Portal2Engine::ComputeLightingCube( vecRenderOrigin, false, boxColors );

	bool invalidOrigin = enginetrace->PointOutsideWorld( vecRenderOrigin );
	const char *pOutsideWorldMsg = invalidOrigin ? "(lighting origin is !OUTSIDE! the world)\n" : "";

	for ( int i = 0; i < 6; ++i )
	{
		if ( !IsFinite( boxColors[i].x ) || !IsFinite( boxColors[i].y ) || !IsFinite( boxColors[i].z ) )
		{
			DevWarning( "Isosurface lighting cube with infinite values!\n%s", pOutsideWorldMsg );
			return;
		}
	}

	for ( int i = 0; i < 6; ++i )
	{
		if ( boxColors[i].x < -0.1f || boxColors[i].y < -0.1f || boxColors[i].z < -0.1f )
		{
			DevWarning( "Isosurface lighting cube negative: %f %f %f\n%s", boxColors[i].x, boxColors[i].y, boxColors[i].z, pOutsideWorldMsg );
		}

		cachedCubeColours[i].AsVector3D() = Lerp( clamp( 2.0f * gpGlobals->frametime, 0.0f, 1.0f ), cachedCubeColours[i].AsVector3D(), boxColors[i] );
	}

	bool bBadLightCube = false;
	for ( int i = 0; i < 6; ++i )
	{
		if ( !IsFinite( cachedCubeColours[i].x ) || !IsFinite( cachedCubeColours[i].y ) || !IsFinite( cachedCubeColours[i].z ) ||
			 cachedCubeColours[i].x < -0.01f || cachedCubeColours[i].y < -0.01f || cachedCubeColours[i].z < -0.01f )
		{
			bBadLightCube = true;
			break;
		}
	}

	if ( bBadLightCube )
	{
		DevWarning( "Isosurface lighting cube bad. Resetting.\n%s", pOutsideWorldMsg );

		// Reconstruction note: the i386 binary clears only the first 4 bytes here
		// (a memset sized by the pointer); the whole cube is reset as intended.
		memset( cachedCubeColours, 0, 6 * sizeof( Vector4D ) );
	}
}


IMaterial *GetDrawMaterial()
{
	if ( r_paintblob_wireframe.GetBool() || mat_wireframe.GetBool() )
	{
		if ( r_paintblob_material.GetInt() < 0 )
			return materials->FindMaterial( "shadertest/wireframe", TEXTURE_GROUP_OTHER, true );

		return materials->FindMaterial( "shadertest/wireframevertexcolornocull", TEXTURE_GROUP_OTHER, true );
	}

	const char *str = r_paintblob_shader.GetString();
	if ( str[0] != '\0' )
		return materials->FindMaterial( str, TEXTURE_GROUP_OTHER, true );

	switch ( r_paintblob_material.GetInt() )
	{
	case 0:
		return materials->FindMaterial( "models/debug/debugwhite", TEXTURE_GROUP_OTHER, true );
	case 1:
		return materials->FindMaterial( "models/debug/debugwhite2", TEXTURE_GROUP_OTHER, true );
	case 2:
		return materials->FindMaterial( "models/debug/debugwhite3", TEXTURE_GROUP_OTHER, true );
	case 3:
		return materials->FindMaterial( "debug/debugvertexcolor", TEXTURE_GROUP_OTHER, true );
	case 4:
		return materials->FindMaterial( "debug/env_cubemap_model", TEXTURE_GROUP_OTHER, true );
	case 5:
		return materials->FindMaterial( "models/blob/env_cubemap_model_translucent_fountain", TEXTURE_GROUP_OTHER, true );
	case 6:
		return materials->FindMaterial( "models/debug/debugmesh", TEXTURE_GROUP_OTHER, true );
	case 7:
		return materials->FindMaterial( "models/debug/debugmesh_transparent", TEXTURE_GROUP_OTHER, true );
	case 8:
		return materials->FindMaterial( "models/ihvtest/tongue_bumped", TEXTURE_GROUP_OTHER, true );
	case 9:
		return materials->FindMaterial( "models/debug/debugbumps", TEXTURE_GROUP_OTHER, true );
	case 10:
		return materials->FindMaterial( "debug/env_cubemap_model_translucent_no_bumps", TEXTURE_GROUP_OTHER, true );
	case 11:
		return materials->FindMaterial( "models/shadertest/predator", TEXTURE_GROUP_OTHER, true );
	}

	return NULL;
}


void SetUpImpRendererUserDefinedFuncs()
{
	// Reconstruction note: empty in both 2010 builds.
}


//-----------------------------------------------------------------------------
// Purpose: Rotate the particles around the render origin and set up their
//			per-particle tangent frame (interpolants 2-4)
//-----------------------------------------------------------------------------
void TransformParticles( float angle, const Vector &center, Vector &transformedCenter, Vector &transformedEye,
						 VMatrix &rotationMatrix, VMatrix &invRotationMatrix, ImpParticleWithFourInterpolants *blobParticles, int blobCount )
{
	for ( int i = 0; i < blobCount; ++i )
	{
		ImpParticleWithFourInterpolants *imp_particle = &blobParticles[i];

		Vector vParticle = imp_particle->center.AsVector();
		Vector transformedParticle = rotationMatrix.ApplyRotation( vParticle - center );

		// keep the field scale stored in w
		Point3D pParticle( transformedParticle );
		imp_particle->center.sse_vec = SetWSIMD( pParticle.sse_vec, imp_particle->center.sse_vec );

		Point3D pCenter( transformedCenter );
		Point3D vec = imp_particle->center - pCenter;
		imp_particle->interpolants2 = vec.normalize();
		imp_particle->interpolants2[3] = 0.0f;

		imp_particle->interpolants4.set( 0.0f, 0.0f, -1.0f );
		imp_particle->interpolants3 = imp_particle->interpolants2.crossProduct( imp_particle->interpolants4 );
		imp_particle->interpolants3[3] = 0.0f;
		imp_particle->interpolants3.normalize();
		imp_particle->interpolants4 = imp_particle->interpolants2.crossProduct( imp_particle->interpolants3 );
		imp_particle->interpolants4[3] = 0.0f;
		imp_particle->interpolants4.normalize();
	}
}


//-----------------------------------------------------------------------------
// Purpose: State shared between the two passes of an isosurface draw
//-----------------------------------------------------------------------------
class CDrawInfo
{
public:
	CDrawInfo( ImpTiler *pTiler, float fAngle, view_id_t viewId )
		: m_pTiler( pTiler ), m_fAngle( fAngle ), m_ViewId( viewId )
	{
	}

	~CDrawInfo()
	{
	}

	ImpTiler *GetTiler() const
	{
		return m_pTiler;
	}

	float GetAngle() const
	{
		return m_fAngle;
	}

	CMatRenderContextPtr &SetRenderContext( IMaterialSystem *pMaterialSystem )
	{
		m_pRenderContext.GetFrom( pMaterialSystem );
		return m_pRenderContext;
	}

	CMatRenderContextPtr &GetRenderContext()
	{
		return m_pRenderContext;
	}

	view_id_t GetViewId() const
	{
		return m_ViewId;
	}

private:
	CDrawInfo( const CDrawInfo & );
	CDrawInfo &operator=( const CDrawInfo & );

	ImpTiler *m_pTiler;
	float m_fAngle;
	CMatRenderContextPtr m_pRenderContext;
	view_id_t m_ViewId;
};


IMaterial *GetMaterial( IMaterial *pMaterial )
{
	IMaterial *pSpecialMaterial = GetDrawMaterial();
	if ( pSpecialMaterial && !IsErrorMaterial( pSpecialMaterial ) )
		return pSpecialMaterial;

	return pMaterial;
}


void SetupRenderContext( IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, CDrawInfo &drawInfo )
{
	CMatRenderContextPtr &pRenderContext = drawInfo.SetRenderContext( materials );
	pRenderContext->MatrixMode( MATERIAL_MODEL );
	pRenderContext->Bind( GetMaterial( pMaterial ), pClientRenderable );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();
	pRenderContext->Translate( vecRenderOrigin.x, vecRenderOrigin.y, vecRenderOrigin.z );
	pRenderContext->Rotate( drawInfo.GetAngle(), 0.0f, 0.0f, 1.0f );
}


void CleanupRenderContext( CDrawInfo &drawInfo )
{
	drawInfo.GetRenderContext()->PopMatrix();
}


//-----------------------------------------------------------------------------
// Purpose: First pass: build the isosurface tiles for the particles
//-----------------------------------------------------------------------------
CDrawInfo *Paintblob_BeginDraw( IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth,
								view_id_t viewID, ImpParticleWithFourInterpolants *blobParticles, int blobCount, bool bUseTwoPassesToDraw )
{
	VPROF_BUDGET( "Paintblob_DrawIsoSurface", "Paintblob_render" );

	if ( r_paintblob_only_mainview_displayed.GetBool() && viewID != VIEW_MAIN )
		return NULL;

	Vector transformedCenter = vecRenderOrigin;

	VMatrix rotationMatrix;
	VMatrix invRotationMatrix;
	Vector transformedEye( 0.0f, 0.0f, 0.0f );
	float fAngle = 0.0f;

	if ( r_paintblob_rotate.GetBool() )
	{
		// rotate so that the transparent surface is sorted towards the eye
		fAngle = view->GetViewSetup()->angles[YAW] + 180.0f;
		if ( r_paintblob_rotate_by90.GetBool() )
		{
			fAngle = (float)( (int)( ( fAngle + 45.0f ) / 90.0f ) * 90 );
		}

		rotationMatrix = SetupMatrixAxisRot( Vector( 0.0f, 0.0f, 1.0f ), -fAngle );
		invRotationMatrix = SetupMatrixAxisRot( Vector( 0.0f, 0.0f, 1.0f ), fAngle );

		Vector eye = view->GetViewSetup()->origin - vecRenderOrigin;
		transformedEye = rotationMatrix.ApplyRotation( eye );
	}
	else
	{
		rotationMatrix.Identity();
		invRotationMatrix.Identity();
	}

	float flPaintResolution = r_paintblob_blr_cubewidth.GetFloat();
	const float flMaxPaintScale = 2.5f;
	float flPaintResolutionLow = flPaintResolution * flMaxPaintScale;
	if ( !r_no_low_res_blobs.GetBool() )
	{
		flPaintResolution = flPaintResolutionLow;
	}
	if ( r_paintblob_only_mainview_highres.GetBool() && viewID != VIEW_MAIN )
	{
		flPaintResolution = flPaintResolutionLow;
	}

	TransformParticles( fAngle, vecRenderOrigin, transformedCenter, transformedEye, rotationMatrix, invRotationMatrix, blobParticles, blobCount );

	ImpTiler *pTiler = ImpTilerFactory::factory->getTiler();
	pTiler->SetCubeWidth( flCubeWidth * r_paintblob_blr_scale.GetFloat() * flPaintResolution );
	pTiler->SetRenderRadius( flCubeWidth * r_paintblob_blr_scale.GetFloat() * r_paintblob_blr_render_radius.GetFloat() );
	pTiler->SetCutoffRadius( flCubeWidth * r_paintblob_blr_scale.GetFloat() * r_paintblob_blr_cutoff_radius.GetFloat() );
	pTiler->setTileIndexToDraw( r_paintblob_tile_index_to_draw.GetInt() );

	CDrawInfo *pDrawInfo = new CDrawInfo( pTiler, fAngle, viewID );

	// with two passes the render context is set up when the surface is drawn
	if ( !bUseTwoPassesToDraw )
	{
		SetupRenderContext( pClientRenderable, vecRenderOrigin, pMaterial, *pDrawInfo );
		pTiler->SetRenderContext( &pDrawInfo->GetRenderContext() );
	}

	pTiler->beginFrame( Point3D( 0.0f, 0.0f, 0.0f ), !r_paintblob_debug_draw_margin.GetBool(), bUseTwoPassesToDraw );

	for ( int i = 0; i < blobCount; ++i )
	{
		pTiler->insertParticle( &blobParticles[i] );
	}

	if ( r_paintblob_rotate.GetBool() )
	{
		pTiler->drawSurfaceSorted( Point3D( transformedEye ), bUseTwoPassesToDraw );
	}
	else
	{
		pTiler->drawSurface( bUseTwoPassesToDraw );
	}

	if ( r_paintblob_debug_draw_tile_boundaries.GetBool() )
	{
		for ( int i = 0; i < pTiler->getNoTiles(); ++i )
		{
			Vector overlayCenter = invRotationMatrix.ApplyRotation( pTiler->getTileOffset( i ).AsVector() );
			Vector mins = -0.5f * pTiler->getRenderDim().AsVector();
			Vector maxs = 0.5f * pTiler->getRenderDim().AsVector();
			debugoverlay->AddBoxOverlay( overlayCenter + vecRenderOrigin, mins, maxs, QAngle( 0.0f, fAngle, 0.0f ), 0, 255, 0, 0, 0.0f );
		}
	}

	return pDrawInfo;
}


//-----------------------------------------------------------------------------
// Purpose: Second pass: draw the surface and release the tiler
//-----------------------------------------------------------------------------
void PaintBlob_WaitEndOfDraw( IClientRenderable *pClientRenderable, const Vector vecRenderOrigin, IMaterial *pMaterial, CDrawInfo *pDrawInfo, bool bUseTwoPassesToDraw )
{
	ImpTiler *pTiler = pDrawInfo->GetTiler();

	if ( bUseTwoPassesToDraw )
	{
		SetupRenderContext( pClientRenderable, vecRenderOrigin, pMaterial, *pDrawInfo );
		pTiler->SetRenderContext( &pDrawInfo->GetRenderContext() );
	}

	pTiler->endFrame( bUseTwoPassesToDraw );
	ImpTilerFactory::factory->returnTiler( pTiler );

	CleanupRenderContext( *pDrawInfo );

	delete pDrawInfo;
}


void Paintblob_CullOutOfViewParticles()
{
	int iParticlesRemoved = 0;
	for ( int i = 0; i < g_PaintblobRenderParticles.Count() - iParticlesRemoved; ++i )
	{
		ImpParticleWithOneInterpolant *imp_particle = &g_PaintblobRenderParticles[i];
		Vector vCenter = imp_particle->center.AsVector();
		if ( R_CullSphere( view->GetFrustum(), 5, &vCenter, 24.0f * imp_particle->scale ) )
		{
			// replace it with the last particle that is still in the list
			ImpParticleWithOneInterpolant *last_particle = &g_PaintblobRenderParticles[ g_PaintblobRenderParticles.Count() - 1 - iParticlesRemoved ];
			*imp_particle = *last_particle;
			++iParticlesRemoved;
			--i;
		}
	}

	if ( iParticlesRemoved != 0 )
	{
		g_PaintblobRenderParticles.SetCountNonDestructively( g_PaintblobRenderParticles.Count() - iParticlesRemoved );
	}
}


void Paintblob_DrawIsoSurface_Synchronous( IClientRenderable *pClientRenderable, const Vector vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth,
										   view_id_t viewID, BlobParticlesEnvelope_t &particles, int blobCount, bool bUseTwoPassesToDraw )
{
	CDrawInfo *pDrawInfo = Paintblob_BeginDraw( pClientRenderable, vecRenderOrigin, pMaterial, flCubeWidth, viewID, particles, blobCount, bUseTwoPassesToDraw );

	// Reconstruction note: the binary does not check for a skipped view here.
	if ( pDrawInfo )
	{
		PaintBlob_WaitEndOfDraw( pClientRenderable, vecRenderOrigin, pMaterial, pDrawInfo, bUseTwoPassesToDraw );
	}
}


ConVar cl_debug_paint_clip_box( "cl_debug_paint_clip_box", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

void Paintblob_Draw( int renderMode, IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, const PortalMatrixList_t &portalMatrixList, bool bSurfaceNoParticleCull )
{
	// the volume behind a portal that clips the blob surface
	Vector backwardPortalMins( 0.0f, -32.0f, -56.0f );
	Vector backwardPortalMaxs( -64.0f, 32.0f, 56.0f );

	if ( cl_debug_paint_clip_box.GetBool() )
	{
		for ( int i = 0; i < portalMatrixList.Count(); ++i )
		{
			Vector pos;
			QAngle ang;
			MatrixAngles( portalMatrixList[i], ang, pos );
			NDebugOverlay::BoxAngles( pos, backwardPortalMins, backwardPortalMaxs, ang, 255, 255, 0, 0, 0.1f );
		}
	}

	if ( !bSurfaceNoParticleCull )
	{
		Paintblob_CullOutOfViewParticles();
	}

	if ( renderMode == BLOB_RENDER_FAST_SPHERE || !r_paintblob_draw_isosurface.GetBool() )
	{
		Paintblob_DrawSpheres( pMaterial, flCubeWidth );
		return;
	}

	CMatRenderContextPtr pRenderContext( materials );
	ICallQueue *pQueue = pRenderContext->GetCallQueue();

	bool bUseTwoPassesToDraw = true;

	// the two pass draw keeps the fragments in the vertex buffer until the second pass
	const int mTotalNumberOfFragments = 256;
	const int mNumberOfRendererInUse = ImpRendererFactory::s_pInstance->GetNumberOfAcquiredTileRenderers();
	const int mNumberOfJobsPerRenderer = 4;
	if ( mNumberOfRendererInUse * mNumberOfJobsPerRenderer > mTotalNumberOfFragments / 2 )
	{
		Warning( "[Blobulator] Switching to one pass as fragment vertex buffer is already half full.\n" );
		bUseTwoPassesToDraw = false;
	}

	if ( r_paintblob_force_single_pass.GetBool() )
	{
		bUseTwoPassesToDraw = false;
	}

	view_id_t viewId = CurrentViewID();

	if ( bUseTwoPassesToDraw )
	{
		// tessellate now, draw on the material thread
		CDrawInfo *pDrawInfo = Paintblob_BeginDraw( NULL, vecRenderOrigin, NULL, flCubeWidth, viewId, g_PaintblobRenderParticles.Base(), g_PaintblobRenderParticles.Count(), true );
		if ( pDrawInfo )
		{
			if ( r_threaded_blobulator.GetBool() && pQueue )
			{
				pQueue->QueueCall( PaintBlob_WaitEndOfDraw, pClientRenderable, vecRenderOrigin, pMaterial, pDrawInfo, true );
			}
			else
			{
				PaintBlob_WaitEndOfDraw( pClientRenderable, vecRenderOrigin, pMaterial, pDrawInfo, true );
			}
		}
	}
	else
	{
		BlobParticlesEnvelope_t particles( g_PaintblobRenderParticles.Base(), g_PaintblobRenderParticles.Count() );
		if ( r_threaded_blobulator.GetBool() && pQueue )
		{
			pQueue->QueueCall( Paintblob_DrawIsoSurface_Synchronous, pClientRenderable, vecRenderOrigin, pMaterial, flCubeWidth, viewId, particles, g_PaintblobRenderParticles.Count(), false );
		}
		else
		{
			Paintblob_DrawIsoSurface_Synchronous( pClientRenderable, vecRenderOrigin, pMaterial, flCubeWidth, viewId, particles, g_PaintblobRenderParticles.Count(), false );
		}
	}
}

} // namespace NPaintRenderer
