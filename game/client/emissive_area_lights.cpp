//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Emissive surfaces as area lights (emissive_area_lights.h).
//
//          A model's emitters are built once per model and skin
//          (render/emissive_area_lights.h): its LOD 0 triangles (.vvd through
//          the model cache, the .vtx read from the file system) whose
//          material is $selfillum, each sampled over a small mip of its base
//          texture and selfillum mask decoded on the CPU, grouped by
//          dominant bone, material and body part, then fitted to rectangles
//          in the model's pose space.
//
//          Each frame (the first view's PreRender): every drawn animating
//          entity's emitters, at the material's current $selfillumtint and
//          the entity's render color, placed by its last bones (the entity
//          transform when it has none), and every registered source's
//          lights, are ranked at the view; the lit ones are placed again from
//          fresh bones and published to the engine.
//
//=============================================================================//
#include "cbase.h"
#include "emissive_area_lights.h"

#include "bitmap/imageformat.h"
#include "c_baseanimating.h"
#include "cdll_client_int.h"
#include "datacache/imdlcache.h"
#include "debugoverlay_shared.h"
#include "filesystem.h"
#include "igamesystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "model_types.h"
#include "optimize.h"
#include "render/emissive_area_lights.h"
#include "studio.h"
#include "tier1/utlbuffer.h"
#include "view.h"
#include "vtf/vtf.h"
#include "vtf/vtf_sample.h"
#include "materialsystem/selfillum_emission.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The frame's budget: the light set's 64 on desktop (the render core
// evaluates them per pixel; the most important 8 also take the CPU lightmap
// and model paths' dlight slots), the RFC 0011 budget of 4 on Android.
#if defined( ANDROID )
#define AREA_LIGHTS_DEFAULT "4"
#else
#define AREA_LIGHTS_DEFAULT "64"
#endif
static ConVar r_area_lights( "r_area_lights", AREA_LIGHTS_DEFAULT, 0,
    "Most emissive surfaces that light their surroundings at once, the most important at the "
    "view (0: none; at most 64; the first 8 also light models and the lightmaps the render core "
    "does not draw)." );
static ConVar r_area_lights_scale( "r_area_lights_scale", "1", FCVAR_ARCHIVE,
    "Global multiplier of the radiance published by emissive area-light sources." );
static ConVar r_area_lights_debug( "r_area_lights_debug", "0", FCVAR_CHEAT,
    "1: draw each emissive area light (green lit, red not); 2: also log each model's emitters "
    "as they are built; 3: log the next frame's candidates once, then 1." );
static ConVar cl_surface_core_emission( "cl_surface_core_emission", "1", FCVAR_CHEAT,
    "Self-illuminated surfaces emit area light on core receivers (0: lighting control)." );
static ConVar cl_surface_core_emission_strength( "cl_surface_core_emission_strength", "16",
    FCVAR_CHEAT, "Core self-illumination source radiance scale, matching the fizzler policy.", true,
    0.0f, false, 0.0f );

static ConVar cl_surface_core_emission_filter( "cl_surface_core_emission_filter", "", FCVAR_CHEAT,
    "Core source light control: empty includes all; exact material name or @panels isolates a "
    "source cohort." );

bool EmissiveAreaLights_CoreSurfaceMode()
{
	static ConVarRef coreWorld( "r_core_world" );
	return coreWorld.IsValid() && coreWorld.GetInt() == 1;
}

float EmissiveAreaLights_SurfaceStrength( const char *material )
{
	if ( !EmissiveAreaLights_CoreSurfaceMode() )
		return 1.0f;
	const char *filter = cl_surface_core_emission_filter.GetString();
	if ( !cl_surface_core_emission.GetBool() ||
	     ( filter[0] && Q_stricmp( filter, material ? material : "@panels" ) != 0 ) )
		return 0.0f;
	return cl_surface_core_emission_strength.GetFloat();
}

namespace
{

// The largest texture dimension sampled (a lower mip of bigger textures).
const int kMaxSampleDimension = 256;
// Models whose emitters are built in one frame; the rest wait a frame.
const int kMaxBuildsPerFrame = 4;
// Candidates considered per frame: every emitter near enough is ranked (this
// only bounds a pathological map).
const int kMaxCandidates = 4096;

CUtlVector<IEmissiveAreaLightSource *> s_Sources;

//-----------------------------------------------------------------------------
// Textures decoded for sampling (vtf/vtf_sample.h), per level.
//-----------------------------------------------------------------------------
struct SampleTexture_t : vtf_sample::Texture
{
	bool m_bTried = false;
	uint64 m_Revision = 0;
};

std::map<std::pair<std::string, int>, SampleTexture_t> s_Textures;
uint64 s_NextSampleRevision = 2;

const SampleTexture_t &LoadSampleTexture( const char *pTextureName, int frame = 0 )
{
	SampleTexture_t &texture = s_Textures[{ pTextureName, frame }];
	if ( texture.m_bTried )
		return texture;
	texture.m_bTried = true;
	texture.m_Revision = s_NextSampleRevision++;
	char path[MAX_PATH];
	Q_snprintf( path, sizeof( path ), "materials/%s.vtf", pTextureName );
	CUtlBuffer buf;
	if ( g_pFullFileSystem->ReadFile( path, "GAME", buf ) )
		vtf_sample::Decode( buf, kMaxSampleDimension, texture, frame );
	return texture;
}

// The loader the shared selfillum rule samples through (materialsystem/
// selfillum_emission.h).
struct SampleLoader_t
{
	int frame = 0;
	const vtf_sample::Texture *operator()( const char *pName ) const
	{
		return &LoadSampleTexture( pName, frame );
	}
};

typedef selfillum_emission::Emission Emission_t;

//-----------------------------------------------------------------------------
// A model's emitters, per model and skin.
//-----------------------------------------------------------------------------
struct ModelEmitter_t
{
	int m_nBone;
	int m_nBodyPart;
	int m_nSubModel;
	// Its material, referenced while the emitter lives. The tint var is looked
	// up each frame: a material reload (a shader config change) recreates vars.
	IMaterial *m_pMaterial;
	area_light::Rect m_Rect; // pose space
	float m_Radiance[3];     // at tint 1
};

struct ModelEmitters_t
{
	bool m_bBuilt = false;
	std::vector<ModelEmitter_t> m_Emitters;
};

std::map<std::pair<const model_t *, int>, ModelEmitters_t> s_Models;

// The map's self-illuminated world faces and overlays (the engine's
// world_emitters), with their materials for the live tint.
struct WorldEmitter_t
{
	area_light::AreaLight m_Light; // at tint 1
	IMaterial *m_pMaterial;        // referenced
};
std::vector<WorldEmitter_t> s_World;
bool s_bWorldFetched = false;

// World emitters' keys: above every entity's (entity indices stay below 2^15).
inline int WorldEmitterKey( int nIndex )
{
	return 0x800000 | ( nIndex & 0x7fffff );
}

void FetchWorldEmitters()
{
	s_bWorldFetched = true;
	const int nCount = arealights->GetWorldEmitters( NULL, 0 );
	if ( nCount <= 0 )
		return;
	std::vector<area_light::WorldEmitterInfo> infos( static_cast<size_t>( nCount ) );
	arealights->GetWorldEmitters( infos.data(), nCount );
	for ( const area_light::WorldEmitterInfo &info : infos )
	{
		IMaterial *pMaterial = materials->FindMaterial( info.material, TEXTURE_GROUP_WORLD, false );
		if ( !pMaterial || pMaterial->IsErrorMaterial() )
			continue;
		pMaterial->IncrementReferenceCount();
		s_World.push_back( WorldEmitter_t{ info.light, pMaterial } );
	}
}

IMaterial *FindModelMaterial( const studiohdr_t *pHdr, int nTexture )
{
	for ( int cd = 0; cd < pHdr->numcdtextures; ++cd )
	{
		char name[MAX_PATH];
		Q_snprintf( name, sizeof( name ), "%s%s", pHdr->pCdtexture( cd ),
		    pHdr->pTexture( nTexture )->pszName() );
		IMaterial *pMaterial = materials->FindMaterial( name, TEXTURE_GROUP_MODEL, false );
		if ( pMaterial && !pMaterial->IsErrorMaterial() )
			return pMaterial;
	}
	return NULL;
}

// The mesh vertex's dominant bone.
int DominantBone( const mstudiovertex_t &vertex )
{
	const mstudioboneweight_t &weights = vertex.m_BoneWeights;
	int nBest = 0;
	for ( int i = 1; i < weights.numbones && i < MAX_NUM_BONES_PER_VERT; ++i )
		if ( weights.weight[i] > weights.weight[nBest] )
			nBest = i;
	return weights.bone[nBest];
}

void BuildModelEmitters( const model_t *pModel, int nSkin, ModelEmitters_t &out )
{
	out.m_bBuilt = true;
	studiohdr_t *pHdr = modelinfo->GetStudiomodel( pModel );
	if ( !pHdr || pHdr->numskinfamilies <= 0 || pHdr->numskinref <= 0 )
		return;
	nSkin = clamp( nSkin, 0, pHdr->numskinfamilies - 1 );
	const short *pSkinRef = pHdr->pSkinref( nSkin * pHdr->numskinref );

	// The skin's self-illuminated materials; most models have none.
	std::vector<IMaterial *> selfIllum( size_t( pHdr->numskinref ), (IMaterial *)NULL );
	std::vector<Emission_t> emission( size_t( pHdr->numskinref ) );
	bool bAny = false;
	for ( int i = 0; i < pHdr->numskinref; ++i )
	{
		IMaterial *pMaterial = FindModelMaterial( pHdr, pSkinRef[i] );
		SampleLoader_t load;
		if ( emission[size_t( i )].Init( pMaterial, load ) )
		{
			selfIllum[size_t( i )] = pMaterial;
			bAny = true;
		}
	}
	if ( !bAny )
		return;

	vertexFileHeader_t *pVvd = mdlcache->GetVertexData( modelinfo->GetCacheHandle( pModel ) );
	if ( !pVvd )
		return;
	const mstudiovertex_t *pVertices = (const mstudiovertex_t *)pVvd->GetVertexData();

	char vtxPath[MAX_PATH];
	Q_StripExtension( modelinfo->GetModelName( pModel ), vtxPath, sizeof( vtxPath ) );
	Q_strncat( vtxPath, ".dx90.vtx", sizeof( vtxPath ), COPY_ALL_CHARACTERS );
	CUtlBuffer vtx;
	if ( !g_pFullFileSystem->ReadFile( vtxPath, "GAME", vtx ) )
		return;
	OptimizedModel::FileHeader_t *pVtx = (OptimizedModel::FileHeader_t *)vtx.Base();
	if ( vtx.TellPut() < (int)sizeof( *pVtx ) || pVtx->version != OPTIMIZED_MODEL_FILE_VERSION ||
	     pVtx->checkSum != pHdr->checksum || pVtx->numBodyParts != pHdr->numbodyparts )
		return;

	// Group ids: dominant bone, material, body part and submodel.
	std::map<std::vector<int>, int> groupIds;
	std::vector<std::vector<int>> groups;
	std::vector<emissive::Triangle> triangles;
	for ( int b = 0; b < pHdr->numbodyparts; ++b )
	{
		mstudiobodyparts_t *pBodyPart = pHdr->pBodypart( b );
		OptimizedModel::BodyPartHeader_t *pVtxBodyPart = pVtx->pBodyPart( b );
		for ( int m = 0; m < pBodyPart->nummodels && m < pVtxBodyPart->numModels; ++m )
		{
			mstudiomodel_t *pStudioModel = pBodyPart->pModel( m );
			OptimizedModel::ModelHeader_t *pVtxModel = pVtxBodyPart->pModel( m );
			if ( pVtxModel->numLODs <= 0 )
				continue;
			OptimizedModel::ModelLODHeader_t *pLod = pVtxModel->pLOD( 0 );
			const int nModelBase = pStudioModel->vertexindex / (int)sizeof( mstudiovertex_t );
			for ( int k = 0; k < pStudioModel->nummeshes && k < pLod->numMeshes; ++k )
			{
				mstudiomesh_t *pMesh = pStudioModel->pMesh( k );
				if ( pMesh->material < 0 || pMesh->material >= pHdr->numskinref ||
				     !selfIllum[size_t( pMesh->material )] )
					continue;
				const Emission_t &fetch = emission[size_t( pMesh->material )];
				OptimizedModel::MeshHeader_t *pVtxMesh = pLod->pMesh( k );
				// Version 49 models carry larger strip headers (as datacache
				// marks them).
				if ( pHdr->version == 49 )
					pVtxMesh->flags |= OptimizedModel::MESH_IS_MDL49;
				for ( int g = 0; g < pVtxMesh->numStripGroups; ++g )
				{
					OptimizedModel::StripGroupHeader_t *pGroup = pVtxMesh->pStripGroup( g );
					if ( pHdr->version == 49 )
						pGroup->flags |= OptimizedModel::STRIPGROUP_IS_MDL49;
					for ( int s = 0; s < pGroup->numStrips; ++s )
					{
						OptimizedModel::StripHeader_t *pStrip = pGroup->pStrip( s );
						const bool bList =
						    ( pStrip->flags & OptimizedModel::STRIP_IS_TRILIST ) != 0;
						const int nTriangles =
						    bList ? pStrip->numIndices / 3 : pStrip->numIndices - 2;
						for ( int t = 0; t < nTriangles; ++t )
						{
							int corner[3];
							for ( int c = 0; c < 3; ++c )
							{
								const int index =
								    pStrip->indexOffset + ( bList ? 3 * t + c : t + c );
								const int local =
								    pGroup->pVertex( *pGroup->pIndex( index ) )->origMeshVertID;
								corner[c] = nModelBase + pMesh->vertexoffset + local;
							}
							if ( corner[0] == corner[1] || corner[1] == corner[2] ||
							     corner[0] == corner[2] )
								continue;
							const int nBone = DominantBone( pVertices[corner[0]] );
							std::vector<int> key = { nBone, pMesh->material, b, m };
							auto found = groupIds.find( key );
							if ( found == groupIds.end() )
							{
								found = groupIds.emplace( key, int( groups.size() ) ).first;
								groups.push_back( key );
							}
							// Face the triangle the way its vertex normals do: the
							// emitters' fronts come from these normals, and the strip
							// winding convention says nothing reliable about them.
							const Vector edge1 = pVertices[corner[1]].m_vecPosition -
							                     pVertices[corner[0]].m_vecPosition;
							const Vector edge2 = pVertices[corner[2]].m_vecPosition -
							                     pVertices[corner[0]].m_vecPosition;
							const Vector vertexNormals = pVertices[corner[0]].m_vecNormal +
							                             pVertices[corner[1]].m_vecNormal +
							                             pVertices[corner[2]].m_vecNormal;
							if ( DotProduct( CrossProduct( edge1, edge2 ), vertexNormals ) < 0.0f )
								V_swap( corner[1], corner[2] );
							emissive::Triangle triangle;
							triangle.group = found->second;
							float uv[3][2];
							for ( int c = 0; c < 3; ++c )
							{
								const mstudiovertex_t &vertex = pVertices[corner[c]];
								for ( int a = 0; a < 3; ++a )
									triangle.p[c][a] = vertex.m_vecPosition[a];
								uv[c][0] = vertex.m_vecTexCoord.x;
								uv[c][1] = vertex.m_vecTexCoord.y;
							}
							emissive::SampleTriangle( uv, fetch.m_pBase->width,
							    fetch.m_pBase->height, fetch, triangle.radiance );
							triangles.push_back( triangle );
						}
					}
				}
			}
		}
	}

	for ( const emissive::Emitter &emitter : emissive::BuildEmitters( triangles ) )
	{
		const std::vector<int> &key = groups[size_t( emitter.group )];
		ModelEmitter_t record;
		record.m_nBone = key[0];
		record.m_nBodyPart = key[2];
		record.m_nSubModel = key[3];
		record.m_pMaterial = selfIllum[size_t( key[1] )];
		record.m_pMaterial->IncrementReferenceCount();
		record.m_Rect = emitter.rect;
		for ( int k = 0; k < 3; ++k )
			record.m_Radiance[k] = emitter.radiance[k];
		out.m_Emitters.push_back( record );
		if ( r_area_lights_debug.GetInt() >= 2 )
			Msg( "arealight emitter %s skin %d bone %d material %s area %.2f %s radiance %.3f %.3f "
			     "%.3f\n",
			    modelinfo->GetModelName( pModel ), nSkin, record.m_nBone,
			    selfIllum[size_t( key[1] )]->GetName(), area_light::Area( record.m_Rect ),
			    record.m_Rect.twoSided ? "two-sided" : "one-sided", record.m_Radiance[0],
			    record.m_Radiance[1], record.m_Radiance[2] );
	}
}

// Places a pose-space rectangle by a pose-to-world transform.
void PlaceRect(
    const area_light::Rect &pose, const matrix3x4_t &poseToWorld, area_light::Rect &out )
{
	Vector center, u, v;
	VectorTransform(
	    Vector( pose.center[0], pose.center[1], pose.center[2] ), poseToWorld, center );
	VectorRotate( Vector( pose.halfU[0], pose.halfU[1], pose.halfU[2] ), poseToWorld, u );
	VectorRotate( Vector( pose.halfV[0], pose.halfV[1], pose.halfV[2] ), poseToWorld, v );
	for ( int k = 0; k < 3; ++k )
	{
		out.center[k] = center[k];
		out.halfU[k] = u[k];
		out.halfV[k] = v[k];
	}
	out.twoSided = pose.twoSided;
}

// Geometry is immutable for the level. Integrated images are cached per
// selected frame; placement, tint and alpha remain the entity's live state.
struct CoreSurfaceGroup_t
{
	std::vector<area_light::EmissiveTriangle> triangles;
	IMaterial *material = NULL;
	int entity = -1;
	std::map<int, std::vector<emissive::Emitter>> frames;
};
std::map<int, std::vector<CoreSurfaceGroup_t>> s_CoreSurfaces;
std::map<std::vector<int>, int> s_CoreKeys;
int s_NextCoreKey = 0xc00000;

std::vector<CoreSurfaceGroup_t> &CoreSurfaces( area_light::IAreaLights4 &geometry, int modelIndex )
{
	auto found = s_CoreSurfaces.find( modelIndex );
	if ( found != s_CoreSurfaces.end() )
		return found->second;
	auto &groups = s_CoreSurfaces[modelIndex];
	const int count = geometry.GetEmissiveTriangles( modelIndex, NULL, 0 );
	if ( count <= 0 )
		return groups;
	std::vector<area_light::EmissiveTriangle> triangles(
	    size_t( count ), area_light::EmissiveTriangle{} );
	geometry.GetEmissiveTriangles( modelIndex, triangles.data(), count );
	std::map<int, size_t> indices;
	for ( const auto &triangle : triangles )
	{
		auto group = indices.find( triangle.group );
		if ( group == indices.end() )
		{
			IMaterial *material =
			    materials->FindMaterial( triangle.material, TEXTURE_GROUP_WORLD, false );
			if ( !material || material->IsErrorMaterial() )
				continue;
			material->IncrementReferenceCount();
			group = indices.emplace( triangle.group, groups.size() ).first;
			groups.emplace_back();
			groups.back().material = material;
			groups.back().entity = triangle.entity;
		}
		groups[group->second].triangles.push_back( triangle );
	}
	return groups;
}

const std::vector<emissive::Emitter> &CoreSurfaceFrame( CoreSurfaceGroup_t &group, int frame )
{
	auto found = group.frames.find( frame );
	if ( found != group.frames.end() )
		return found->second;
	SampleLoader_t load{ frame };
	SampleLoader_t staticImages;
	Emission_t emission;
	auto &emitters = group.frames[frame];
	const bool unlit = !group.material->GetMaterialVarFlag( MATERIAL_VAR_SELFILLUM ) &&
	                   emissive::UnlitSource( group.material->GetShaderName() );
	// Base frames animate; independently bound self-illumination masks stay at frame 0.
	const bool selfIllum = emission.Init( group.material, staticImages );
	if ( unlit || selfIllum )
		emission.m_pBase = selfillum_emission::TextureOfVar( group.material, "$basetexture", load );
	if ( ( !unlit && !selfIllum ) || !emission.m_pBase )
	{
		Warning( "core emissive source %s: frame %d has no supported emission image\n",
		    group.material->GetName(), frame );
		return emitters;
	}
	bool foundVar = false;
	IMaterialVar *referenceVar = group.material->FindVar( "$alphatestreference", &foundVar, false );
	const float reference = foundVar && referenceVar && referenceVar->GetFloatValue() > 0
	                            ? referenceVar->GetFloatValue()
	                            : 0.5f;
	const bool alphaTest = group.material->GetMaterialVarFlag( MATERIAL_VAR_ALPHATEST );
	const bool translucent = group.material->GetMaterialVarFlag( MATERIAL_VAR_TRANSLUCENT );
	emitters = emissive::BuildMappedEmitters( group.triangles, emission.m_pBase->width,
	    emission.m_pBase->height,
	    [&]( float u, float v, float out[3] )
	    {
		    if ( !unlit )
		    {
			    emission( u, v, out );
			    return;
		    }
		    float alpha;
		    emission.m_pBase->Fetch( u, v, out, &alpha );
		    emissive::UnlitRadiance( out, alpha, translucent, alphaTest, reference, out );
	    } );
	if ( group.material->GetMaterialVarFlag( MATERIAL_VAR_NOCULL ) )
		for ( auto &emitter : emitters )
			emitter.rect.twoSided = true;
	if ( r_area_lights_debug.GetInt() >= 2 )
		for ( const auto &emitter : emitters )
			Msg( "core surface emitter material %s frame %d proxy entity %d group %d "
			     "center %.3f %.3f %.3f radiance %.6f %.6f %.6f area %.3f\n",
			    group.material->GetName(), frame, group.entity, emitter.group,
			    emitter.rect.center[0], emitter.rect.center[1], emitter.rect.center[2],
			    emitter.radiance[0], emitter.radiance[1], emitter.radiance[2],
			    area_light::Area( emitter.rect ) );
	return emitters;
}

// The pose-to-world transform of an emitter's bone: the entity's last bones,
// or its transform (the bind pose) when it has none.
void PoseToWorld(
    C_BaseAnimating *pAnim, const studiohdr_t *pHdr, int nBone, bool bFreshBones, matrix3x4_t &out )
{
	if ( nBone >= 0 && nBone < pHdr->numbones && ( bFreshBones || pAnim->IsBoneCacheValid() ) )
	{
		ConcatTransforms( pAnim->GetBone( nBone ), pHdr->pBone( nBone )->poseToBone, out );
		return;
	}
	MatrixCopy( pAnim->EntityToWorldTransform(), out );
	const float flScale = pAnim->GetModelScale();
	if ( flScale != 1.0f )
		MatrixScaleBy( flScale, out );
}

struct Candidate_t
{
	emissive::Candidate m_Candidate;
	int m_nKey;
	bool m_bCoreOnly = false;
	C_BaseAnimating *m_pAnim; // null for a source's light
	const ModelEmitter_t *m_pEmitter;
	float m_Tint[3];
};

// Whether the view can see a placed rectangle: its bounds are in the view's
// potentially visible set and the world does not block the line from the view
// to its center. The rectangle lies on its surface, so a line that ends
// within kSeenTolerance of the center reaches it.
const float kSeenTolerance = 8.0f;
bool InView( const area_light::Rect &rect, const Vector &vecView )
{
	float corners[4][3];
	area_light::Corners( rect, corners );
	Vector mins( corners[0][0], corners[0][1], corners[0][2] ), maxs = mins;
	for ( int c = 1; c < 4; ++c )
	{
		const Vector corner( corners[c][0], corners[c][1], corners[c][2] );
		VectorMin( mins, corner, mins );
		VectorMax( maxs, corner, maxs );
	}
	const Vector pad( 4.0f, 4.0f, 4.0f );
	if ( !engine->IsBoxVisible( mins - pad, maxs + pad ) )
		return false;
	const Vector center( rect.center[0], rect.center[1], rect.center[2] );
	CTraceFilterWorldOnly filter;
	trace_t tr;
	UTIL_TraceLine( vecView, center, MASK_OPAQUE, &filter, &tr );
	return tr.fraction >= 1.0f || tr.endpos.DistToSqr( center ) <= Square( kSeenTolerance );
}

float Linear( unsigned char c )
{
	return SrgbGammaToLinear( c / 255.0f );
}

//-----------------------------------------------------------------------------
// The per-frame publisher.
//-----------------------------------------------------------------------------
class CEmissiveAreaLights : public CAutoGameSystemPerFrame
{
public:
	CEmissiveAreaLights()
	    : CAutoGameSystemPerFrame( "CEmissiveAreaLights" ), m_nFrame( -1 ), m_nPublished( 0 )
	{
	}

	virtual void LevelShutdownPostEntity()
	{
		for ( auto &entry : s_Models )
			for ( ModelEmitter_t &emitter : entry.second.m_Emitters )
				emitter.m_pMaterial->DecrementReferenceCount();
		s_Models.clear();
		for ( WorldEmitter_t &emitter : s_World )
			emitter.m_pMaterial->DecrementReferenceCount();
		s_World.clear();
		for ( auto &entry : s_CoreSurfaces )
			for ( auto &group : entry.second )
				group.material->DecrementReferenceCount();
		s_CoreSurfaces.clear();
		s_CoreKeys.clear();
		s_NextCoreKey = 0xc00000;
		s_bWorldFetched = false;
		s_Textures.clear();
		m_Lit.clear();
		m_nPublished = 0;
	}

	virtual void PreRender();
	void SetGeometry( area_light::IAreaLights4 *provider ) { m_Geometry = provider; }

private:
	void Publish( const area_light::AreaLight *pLights, const int *pKeys, int nCount,
	    const bool *pCoreOnly = NULL )
	{
		if ( nCount == 0 && m_nPublished == 0 )
			return;
		arealights->SetFrameAreaLights( pLights, pKeys, pCoreOnly, nCount );
		m_nPublished = nCount;
	}

	int m_nFrame;
	int m_nPublished;
	std::vector<int> m_Lit; // the keys lit last frame
	area_light::IAreaLights4 *m_Geometry = NULL; // composition-owned, main-thread borrower
};

CEmissiveAreaLights s_EmissiveAreaLights;

void CEmissiveAreaLights::PreRender()
{
	// Once per frame (every view runs PreRender), ranked at the main view.
	if ( m_nFrame == gpGlobals->framecount )
		return;
	m_nFrame = gpGlobals->framecount;
	if ( !arealights )
		return;
	const int nBudget = MIN( r_area_lights.GetInt(), area_light::kMaxFrameAreaLights );
	if ( nBudget <= 0 || !engine->IsInGame() )
	{
		Publish( NULL, NULL, 0 );
		m_Lit.clear();
		return;
	}

	const Vector &vecView = MainViewOrigin();
	const float view[3] = { vecView.x, vecView.y, vecView.z };
	std::vector<Candidate_t> candidates;
	candidates.reserve( 64 );
	int nBuilds = 0;
	const bool core = EmissiveAreaLights_CoreSurfaceMode();
	const float coreStrength =
	    cl_surface_core_emission.GetBool() ? cl_surface_core_emission_strength.GetFloat() : 0.0f;
	if ( core && coreStrength > 0.0f && !m_Geometry )
		Error( "Core emissive surfaces require engine provider %s",
		    area_light::kAreaLightsGeometryVersion );

	auto wasLit = [this]( int nKey )
	{
		for ( int key : m_Lit )
			if ( key == nKey )
				return true;
		return false;
	};

	for ( C_BaseEntity *pEnt = ClientEntityList().FirstBaseEntity(); pEnt;
	    pEnt = ClientEntityList().NextBaseEntity( pEnt ) )
	{
		if ( (int)candidates.size() >= kMaxCandidates )
			break;
		C_BaseAnimating *pAnim = pEnt->GetBaseAnimating();
		if ( !pAnim || pAnim->IsDormant() || pAnim->IsEffectActive( EF_NODRAW ) ||
		     pAnim->GetRenderMode() == kRenderNone )
			continue;
		const model_t *pModel = pAnim->GetModel();
		if ( !pModel || modelinfo->GetModelType( pModel ) != mod_studio )
			continue;
		ModelEmitters_t &model = s_Models[std::make_pair( pModel, pAnim->GetSkin() )];
		if ( !model.m_bBuilt )
		{
			if ( nBuilds >= kMaxBuildsPerFrame )
				continue;
			++nBuilds;
			BuildModelEmitters( pModel, pAnim->GetSkin(), model );
		}
		if ( model.m_Emitters.empty() )
			continue;
		// Too far to light anything the view sees.
		if ( vecView.DistToSqr( pAnim->GetAbsOrigin() ) >
		     Square( area_light::kMaxReach + modelinfo->GetModelRadius( pModel ) + 512.0f ) )
			continue;
		const studiohdr_t *pHdr = modelinfo->GetStudiomodel( pModel );
		const color32 color = pAnim->GetRenderColor();
		const float render[3] = { Linear( color.r ), Linear( color.g ), Linear( color.b ) };
		for ( size_t e = 0; e < model.m_Emitters.size(); ++e )
		{
			const ModelEmitter_t &emitter = model.m_Emitters[e];
			// Only the drawn submodel of its body part.
			const mstudiobodyparts_t *pBodyPart = pHdr->pBodypart( emitter.m_nBodyPart );
			if ( pBodyPart->nummodels > 1 &&
			     ( pAnim->GetBody() / MAX( pBodyPart->base, 1 ) ) % pBodyPart->nummodels !=
			         emitter.m_nSubModel )
				continue;
			Candidate_t candidate;
			candidate.m_nKey = EmissiveAreaLights_EntityKey( pAnim->entindex(), int( e ) );
			candidate.m_pAnim = pAnim;
			candidate.m_pEmitter = &emitter;
			candidate.m_bCoreOnly = core;
			float tint[3] = { 1.0f, 1.0f, 1.0f };
			static unsigned int s_nTintToken = 0;
			IMaterialVar *pTint =
			    emitter.m_pMaterial->FindVarFast( "$selfillumtint", &s_nTintToken );
			if ( pTint && pTint->IsDefined() )
				pTint->GetVecValue( tint, 3 );
			area_light::AreaLight &light = candidate.m_Candidate.light;
			for ( int k = 0; k < 3; ++k )
			{
				candidate.m_Tint[k] = MAX( tint[k], 0.0f ) * render[k];
				light.radiance[k] =
				    emitter.m_Radiance[k] * candidate.m_Tint[k] *
				    ( core ? EmissiveAreaLights_SurfaceStrength( emitter.m_pMaterial->GetName() ) *
				                 color.a / 255.0f
				           : 1.0f );
			}
			matrix3x4_t poseToWorld;
			PoseToWorld( pAnim, pHdr, emitter.m_nBone, false, poseToWorld );
			PlaceRect( emitter.m_Rect, poseToWorld, light.rect );
			light.reach = area_light::Reach( light.rect, light.radiance );
			if ( !( light.reach > 0.0f ) )
				continue;
			candidate.m_Candidate.wasLit = wasLit( candidate.m_nKey );
			candidates.push_back( candidate );
		}
	}

	// Self-illuminated world faces and overlays, at their material's live tint.
	if ( !core && !s_bWorldFetched )
		FetchWorldEmitters();
	for ( size_t w = 0; !core && w < s_World.size() && (int)candidates.size() < kMaxCandidates;
	    ++w )
	{
		const WorldEmitter_t &emitter = s_World[w];
		Candidate_t candidate;
		area_light::AreaLight &light = candidate.m_Candidate.light;
		light = emitter.m_Light;
		if ( area_light::DistanceTo( light.rect, view ) > area_light::kMaxReach + 512.0f )
			continue;
		float tint[3] = { 1.0f, 1.0f, 1.0f };
		static unsigned int s_nWorldTintToken = 0;
		IMaterialVar *pTint =
		    emitter.m_pMaterial->FindVarFast( "$selfillumtint", &s_nWorldTintToken );
		if ( pTint && pTint->IsDefined() )
			pTint->GetVecValue( tint, 3 );
		for ( int k = 0; k < 3; ++k )
			light.radiance[k] *= MAX( tint[k], 0.0f );
		light.reach = area_light::Reach( light.rect, light.radiance );
		if ( !( light.reach > 0.0f ) )
			continue;
		candidate.m_nKey = WorldEmitterKey( int( w ) );
		candidate.m_pAnim = NULL;
		candidate.m_pEmitter = NULL;
		candidate.m_Candidate.wasLit = wasLit( candidate.m_nKey );
		candidates.push_back( candidate );
	}

	// Core world/overlay and moving brush sources use the selected image,
	// geometry and entity state of this frame, without invoking a proxy again.
	if ( core && coreStrength > 0.0f && m_Geometry )
	{
		auto append = [&]( int modelIndex, C_BaseEntity *placement )
		{
			for ( auto &group : CoreSurfaces( *m_Geometry, modelIndex ) )
			{
				const float sourceStrength =
				    EmissiveAreaLights_SurfaceStrength( group.material->GetName() );
				if ( sourceStrength <= 0.0f )
					continue;
				C_BaseEntity *state = placement ? placement
				                      : group.entity >= 0
				                          ? ClientEntityList().GetEnt( group.entity )
				                          : NULL;
				if ( state && ( state->IsDormant() || state->IsEffectActive( EF_NODRAW ) ||
				                  state->GetRenderMode() == kRenderNone ) )
					continue;
				bool found = false;
				IMaterialVar *frameVar = group.material->FindVar( "$frame", &found, false );
				const int frame = state               ? state->GetTextureFrameIndex()
				                  : found && frameVar ? frameVar->GetIntValue()
				                                      : 0;
				float tint[3] = { 1, 1, 1 };
				IMaterialVar *tintVar = group.material->FindVar( "$selfillumtint", &found, false );
				if ( found && tintVar && tintVar->IsDefined() )
					tintVar->GetVecValue( tint, 3 );
				IMaterialVar *alphaVar = group.material->FindVar( "$alpha", &found, false );
				const float alpha = found && alphaVar ? alphaVar->GetFloatValue() : 1.0f;
				const color32 color =
				    state ? state->GetRenderColor() : color32{ 255, 255, 255, 255 };
				const float scale = sourceStrength * MAX( alpha, 0.0f ) * color.a / 255.0f;
				int bin = 0;
				for ( const auto &emitter : CoreSurfaceFrame( group, frame ) )
				{
					Candidate_t candidate;
					candidate.m_pAnim = NULL;
					candidate.m_pEmitter = NULL;
					candidate.m_bCoreOnly = true;
					const std::vector<int> identity = {
					    placement ? placement->entindex() : -1, modelIndex, emitter.group, bin++ };
					auto key = s_CoreKeys.find( identity );
					if ( key == s_CoreKeys.end() )
						key = s_CoreKeys.emplace( identity, s_NextCoreKey++ ).first;
					candidate.m_nKey = key->second;
					auto &light = candidate.m_Candidate.light;
					light.rect = emitter.rect;
					if ( placement )
						PlaceRect( emitter.rect, placement->EntityToWorldTransform(), light.rect );
					for ( int k = 0; k < 3; ++k )
						light.radiance[k] = emitter.radiance[k] * MAX( tint[k], 0.0f ) * scale;
					light.reach = area_light::Reach( light.rect, light.radiance );
					if ( light.reach <= 0.0f || area_light::DistanceTo( light.rect, view ) >
					                                area_light::kMaxReach + 512.0f )
						continue;
					candidate.m_Candidate.wasLit = wasLit( candidate.m_nKey );
					candidates.push_back( candidate );
				}
			}
		};
		append( 0, NULL );
		for ( C_BaseEntity *entity = ClientEntityList().FirstBaseEntity(); entity;
		    entity = ClientEntityList().NextBaseEntity( entity ) )
		{
			const model_t *model = entity->GetModel();
			if ( model && modelinfo->GetModelType( model ) == mod_brush )
				append( entity->GetModelIndex(), entity );
		}
	}

	for ( int s = 0; s < s_Sources.Count(); ++s )
	{
		area_light::AreaLight lights[area_light::kMaxAreaLights];
		int keys[area_light::kMaxAreaLights];
		const int nCount = s_Sources[s]->GetAreaLights( lights, keys, area_light::kMaxAreaLights );
		for ( int i = 0; i < nCount && (int)candidates.size() < kMaxCandidates; ++i )
		{
			Candidate_t candidate;
			candidate.m_Candidate.light = lights[i];
			candidate.m_nKey = keys[i];
			candidate.m_bCoreOnly = s_Sources[s]->CoreOnly();
			candidate.m_pAnim = NULL;
			candidate.m_pEmitter = NULL;
			candidate.m_Candidate.wasLit = wasLit( keys[i] );
			candidates.push_back( candidate );
		}
	}

	const float flScale = MAX( r_area_lights_scale.GetFloat(), 0.0f );
	for ( Candidate_t &candidate : candidates )
	{
		area_light::AreaLight &light = candidate.m_Candidate.light;
		if ( flScale == 1.0f )
			continue;
		for ( float &channel : light.radiance )
			channel *= flScale;
		light.reach = area_light::Reach( light.rect, light.radiance );
	}

	std::vector<emissive::Candidate> ranked( candidates.size() );
	for ( size_t i = 0; i < candidates.size(); ++i )
		ranked[i] = candidates[i].m_Candidate;
	std::unique_ptr<bool[]> lit( new bool[candidates.size() + 1] );
	// -1 not asked, 0 hidden, 1 in view.
	std::vector<signed char> seen( candidates.size(), -1 );
	std::vector<int> order;
	emissive::SelectLit(
	    ranked.data(), int( ranked.size() ), nBudget, view, lit.get(),
	    [&]( int index )
	    {
		    seen[size_t( index )] = InView( ranked[size_t( index )].light.rect, vecView ) ? 1 : 0;
		    return seen[size_t( index )] == 1;
	    },
	    &order );
	if ( r_area_lights_debug.GetInt() >= 3 )
	{
		r_area_lights_debug.SetValue( 1 );
		for ( size_t i = 0; i < ranked.size(); ++i )
		{
			const area_light::AreaLight &light = ranked[i].light;
			Msg( "arealight candidate key %d at %.0f %.0f %.0f reach %.0f distance %.0f importance "
			     "%.4g %s %s\n",
			    candidates[i].m_nKey, light.rect.center[0], light.rect.center[1],
			    light.rect.center[2], light.reach, area_light::DistanceTo( light.rect, view ),
			    emissive::Importance( light, view ),
			    seen[i] < 0 ? "not-asked"
			    : seen[i]   ? "in-view"
			                : "hidden",
			    lit[i] ? "lit" : "unlit" );
		}
	}

	// The lit ones, most important first (the engine gives the first
	// kMaxAreaLights its dlight slots).
	area_light::AreaLight lights[area_light::kMaxFrameAreaLights];
	int keys[area_light::kMaxFrameAreaLights];
	bool coreOnly[area_light::kMaxFrameAreaLights];
	int nLit = 0;
	m_Lit.clear();
	for ( int index : order )
	{
		if ( nLit == area_light::kMaxFrameAreaLights )
			break;
		Candidate_t &candidate = candidates[size_t( index )];
		area_light::AreaLight &light = candidate.m_Candidate.light;
		// A lit emitter follows its entity's bones for this frame.
		if ( candidate.m_pAnim && !candidate.m_pAnim->IsBoneCacheValid() )
		{
			C_BaseAnimating::PushAllowBoneAccess( true, false, "EmissiveAreaLights" );
			candidate.m_pAnim->SetupBones( NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime );
			C_BaseAnimating::PopBoneAccess( "EmissiveAreaLights" );
		}
		if ( candidate.m_pAnim )
		{
			matrix3x4_t poseToWorld;
			const studiohdr_t *pHdr = modelinfo->GetStudiomodel( candidate.m_pAnim->GetModel() );
			PoseToWorld( candidate.m_pAnim, pHdr, candidate.m_pEmitter->m_nBone,
			    candidate.m_pAnim->IsBoneCacheValid(), poseToWorld );
			PlaceRect( candidate.m_pEmitter->m_Rect, poseToWorld, light.rect );
			light.reach = area_light::Reach( light.rect, light.radiance );
		}
		lights[nLit] = light;
		keys[nLit] = candidate.m_nKey;
		coreOnly[nLit] = candidate.m_bCoreOnly;
		++nLit;
		m_Lit.push_back( candidate.m_nKey );
	}
	if ( r_area_lights_debug.GetBool() )
	{
		for ( size_t i = 0; i < candidates.size(); ++i )
		{
			const area_light::Rect &rect = candidates[i].m_Candidate.light.rect;
			float corners[4][3];
			area_light::Corners( rect, corners );
			const int r = lit[i] ? 0 : 255, g = lit[i] ? 255 : 0;
			for ( int c = 0; c < 4; ++c )
			{
				const float *a = corners[c], *b = corners[( c + 1 ) % 4];
				NDebugOverlay::Line(
				    Vector( a[0], a[1], a[2] ), Vector( b[0], b[1], b[2] ), r, g, 0, true, 0.0f );
			}
		}
	}
	Publish( lights, keys, nLit, coreOnly );
}

} // namespace

void EmissiveAreaLights_SetGeometry( area_light::IAreaLights4 *provider )
{
	s_EmissiveAreaLights.SetGeometry( provider );
}

CON_COMMAND(
    r_area_lights_list, "List every model's emissive area-light emitters built this level." )
{
	int nModels = 0, nEmitters = 0;
	for ( const auto &entry : s_Models )
	{
		if ( entry.second.m_Emitters.empty() )
			continue;
		++nModels;
		for ( const ModelEmitter_t &e : entry.second.m_Emitters )
		{
			++nEmitters;
			Msg( "arealight emitter %s skin %d bone %d body %d/%d area %.2f %s radiance %.3f %.3f "
			     "%.3f "
			     "material %s\n",
			    modelinfo->GetModelName( entry.first.first ), entry.first.second, e.m_nBone,
			    e.m_nBodyPart, e.m_nSubModel, area_light::Area( e.m_Rect ),
			    e.m_Rect.twoSided ? "two-sided" : "one-sided", e.m_Radiance[0], e.m_Radiance[1],
			    e.m_Radiance[2], e.m_pMaterial->GetName() );
		}
	}
	for ( const WorldEmitter_t &e : s_World )
		Msg( "arealight world emitter %s at %.0f %.0f %.0f area %.2f radiance %.3f %.3f %.3f\n",
		    e.m_pMaterial->GetName(), e.m_Light.rect.center[0], e.m_Light.rect.center[1],
		    e.m_Light.rect.center[2], area_light::Area( e.m_Light.rect ), e.m_Light.radiance[0],
		    e.m_Light.radiance[1], e.m_Light.radiance[2] );
	Msg( "arealight: %d world emitter(s)\n", int( s_World.size() ) );
	Msg( "arealight: %d emissive model(s), %d emitter(s), %d model/skin pair(s) built, %d "
	     "source(s)\n",
	    nModels, nEmitters, int( s_Models.size() ), s_Sources.Count() );
}

void EmissiveAreaLights_AddSource( IEmissiveAreaLightSource *pSource )
{
	if ( s_Sources.Find( pSource ) == s_Sources.InvalidIndex() )
		s_Sources.AddToTail( pSource );
}

void EmissiveAreaLights_RemoveSource( IEmissiveAreaLightSource *pSource )
{
	s_Sources.FindAndRemove( pSource );
}

uint64 EmissiveAreaLights_SampleRevision( ITexture *pTexture )
{
	// These paths always sample as unavailable, regardless of GPU contents.
	// Coverage lives in the quad, so that fallback is itself immutable.
	if ( !pTexture || pTexture->IsError() || pTexture->IsRenderTarget() ||
	     ( pTexture->IsProcedural() && pTexture->IsTranslucent() ) )
		return 1;
	if ( pTexture->IsProcedural() )
		return 0;
	return LoadSampleTexture( pTexture->GetName() ).m_Revision;
}

bool EmissiveAreaLights_SampleTexture( ITexture *pTexture, float s, float t, float rgba[4] )
{
	if ( !pTexture || pTexture->IsError() || pTexture->IsRenderTarget() || !std::isfinite( s ) ||
	     !std::isfinite( t ) )
		return false;
	if ( pTexture->IsProcedural() )
	{
		// Only opaque CPU-generated RGB/BGR textures promise these samples.
		// Missing sampling data leaves the sentinel intact; never invent white
		// emission for an unsupported procedural image.
		if ( pTexture->IsTranslucent() )
			return false;
		float color[3] = { -1.0f, -1.0f, -1.0f };
		pTexture->GetLowResColorSample( s, t, color );
		for ( int k = 0; k < 3; ++k )
		{
			if ( !( color[k] >= 0.0f && color[k] <= 1.0f ) )
				return false;
			rgba[k] = color[k];
		}
		rgba[3] = 1.0f;
		return true;
	}
	const vtf_sample::Texture &texture = LoadSampleTexture( pTexture->GetName() );
	if ( !texture.valid )
		return false;
	float rgb[3], alpha;
	texture.Fetch( s, t, rgb, &alpha );
	for ( int k = 0; k < 3; ++k )
		rgba[k] = SrgbLinearToGamma( rgb[k] );
	rgba[3] = alpha;
	return true;
}

bool EmissiveAreaLights_SampleFieldTexture(
    ITexture *pTexture, float s, float t, bool linearRgb, float rgba[4] )
{
	if ( !pTexture || pTexture->IsError() || pTexture->IsRenderTarget() ||
	     pTexture->IsProcedural() || !std::isfinite( s ) || !std::isfinite( t ) )
		return false;
	const vtf_sample::Texture &texture = LoadSampleTexture( pTexture->GetName() );
	if ( !texture.valid )
		return false;
	auto coordinate = []( float uv, int size, bool clamp )
	{
		return ( clamp ? std::clamp( uv, 0.0f, 1.0f ) : uv - std::floor( uv ) ) * size - 0.5f;
	};
	const unsigned flags = pTexture->GetFlags();
	const float x = coordinate( s, texture.width, ( flags & TEXTUREFLAGS_CLAMPS ) != 0 );
	const float y = coordinate( t, texture.height, ( flags & TEXTUREFLAGS_CLAMPT ) != 0 );
	const int ix = int( std::floor( x ) ), iy = int( std::floor( y ) );
	const float fx = x - ix, fy = y - iy;
	auto index = []( int i, int size, bool clamp )
	{
		return clamp ? std::clamp( i, 0, size - 1 ) : ( i % size + size ) % size;
	};
	for ( int k = 0; k < 4; ++k )
		rgba[k] = 0.0f;
	for ( int cy = 0; cy < 2; ++cy )
	{
		for ( int cx = 0; cx < 2; ++cx )
		{
			const int at = index( iy + cy, texture.height, ( flags & TEXTUREFLAGS_CLAMPT ) != 0 ) *
			                   texture.width +
			               index( ix + cx, texture.width, ( flags & TEXTUREFLAGS_CLAMPS ) != 0 );
			const float weight = ( cx ? fx : 1.0f - fx ) * ( cy ? fy : 1.0f - fy );
			for ( int k = 0; k < 3; ++k )
			{
				const float value = texture.rgb[3 * at + k];
				rgba[k] += weight * ( linearRgb ? value : SrgbLinearToGamma( value ) );
			}
			rgba[3] += weight * texture.alpha[at];
		}
	}
	return true;
}
