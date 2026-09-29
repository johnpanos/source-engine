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

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The RFC 0011 budget: 8 area lights on desktop, 4 on Android.
#if defined( ANDROID )
#define AREA_LIGHTS_DEFAULT "4"
#else
#define AREA_LIGHTS_DEFAULT "8"
#endif
static ConVar r_area_lights( "r_area_lights", AREA_LIGHTS_DEFAULT, 0,
    "Most emissive surfaces that light their surroundings at once, the most important at the "
    "view (0: none; at most 8)." );
static ConVar r_area_lights_scale( "r_area_lights_scale", "1", FCVAR_ARCHIVE,
    "Strength of emissive area lights: 1 is physical (a surface lights with exactly the "
    "radiance it draws); more exaggerates it." );
static ConVar r_area_lights_debug( "r_area_lights_debug", "0", FCVAR_CHEAT,
    "1: draw each emissive area light (green lit, red not); 2: also log each model's emitters "
    "as they are built." );

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
};

std::unordered_map<std::string, SampleTexture_t> s_Textures;

const SampleTexture_t &LoadSampleTexture( const char *pTextureName )
{
	SampleTexture_t &texture = s_Textures[pTextureName];
	if ( texture.m_bTried )
		return texture;
	texture.m_bTried = true;
	char path[MAX_PATH];
	Q_snprintf( path, sizeof( path ), "materials/%s.vtf", pTextureName );
	CUtlBuffer buf;
	if ( g_pFullFileSystem->ReadFile( path, "GAME", buf ) )
		vtf_sample::Decode( buf, kMaxSampleDimension, texture );
	return texture;
}

// The loader the shared selfillum rule samples through (materialsystem/
// selfillum_emission.h).
struct SampleLoader_t
{
	const vtf_sample::Texture *operator()( const char *pName ) const
	{
		return &LoadSampleTexture( pName );
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
	C_BaseAnimating *m_pAnim; // null for a source's light
	const ModelEmitter_t *m_pEmitter;
	float m_Tint[3];
};

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
		s_bWorldFetched = false;
		s_Textures.clear();
		m_Lit.clear();
		m_nPublished = 0;
	}

	virtual void PreRender();

private:
	void Publish( const area_light::AreaLight *pLights, const int *pKeys, int nCount )
	{
		if ( nCount == 0 && m_nPublished == 0 )
			return;
		arealights->SetAreaLights( pLights, pKeys, nCount );
		m_nPublished = nCount;
	}

	int m_nFrame;
	int m_nPublished;
	std::vector<int> m_Lit; // the keys lit last frame
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
	const int nBudget = MIN( r_area_lights.GetInt(), area_light::kMaxAreaLights );
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
				light.radiance[k] = emitter.m_Radiance[k] * candidate.m_Tint[k];
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
	if ( !s_bWorldFetched )
		FetchWorldEmitters();
	for ( size_t w = 0; w < s_World.size() && (int)candidates.size() < kMaxCandidates; ++w )
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
	emissive::SelectLit( ranked.data(), int( ranked.size() ), nBudget, view, lit.get() );

	area_light::AreaLight lights[area_light::kMaxAreaLights];
	int keys[area_light::kMaxAreaLights];
	int nLit = 0;
	m_Lit.clear();
	for ( size_t i = 0; i < candidates.size(); ++i )
	{
		Candidate_t &candidate = candidates[i];
		if ( lit[i] && nLit < area_light::kMaxAreaLights )
		{
			area_light::AreaLight &light = candidate.m_Candidate.light;
			// A lit emitter follows its entity's bones for this frame.
			if ( candidate.m_pAnim && !candidate.m_pAnim->IsBoneCacheValid() )
			{
				C_BaseAnimating::PushAllowBoneAccess( true, false, "EmissiveAreaLights" );
				candidate.m_pAnim->SetupBones(
				    NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime );
				C_BaseAnimating::PopBoneAccess( "EmissiveAreaLights" );
			}
			if ( candidate.m_pAnim )
			{
				matrix3x4_t poseToWorld;
				const studiohdr_t *pHdr =
				    modelinfo->GetStudiomodel( candidate.m_pAnim->GetModel() );
				PoseToWorld( candidate.m_pAnim, pHdr, candidate.m_pEmitter->m_nBone,
				    candidate.m_pAnim->IsBoneCacheValid(), poseToWorld );
				PlaceRect( candidate.m_pEmitter->m_Rect, poseToWorld, light.rect );
				light.reach = area_light::Reach( light.rect, light.radiance );
			}
			lights[nLit] = light;
			keys[nLit] = candidate.m_nKey;
			++nLit;
			m_Lit.push_back( candidate.m_nKey );
		}
		if ( r_area_lights_debug.GetBool() )
		{
			const area_light::Rect &rect = candidate.m_Candidate.light.rect;
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
	Publish( lights, keys, nLit );
}

} // namespace

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

bool EmissiveAreaLights_MaterialRadiance( const char *pMaterialName, float out[3] )
{
	IMaterial *pMaterial = materials->FindMaterial( pMaterialName, TEXTURE_GROUP_VGUI, false );
	if ( !pMaterial || pMaterial->IsErrorMaterial() )
		return false;
	SampleLoader_t load;
	const vtf_sample::Texture *pBase =
	    selfillum_emission::TextureOfVar( pMaterial, "$basetexture", load );
	if ( !pBase )
		return false;
	double sum[3] = { 0, 0, 0 };
	const int nTexels = pBase->width * pBase->height;
	for ( int i = 0; i < nTexels; ++i )
		for ( int k = 0; k < 3; ++k )
			sum[k] += pBase->rgb[3 * i + k];
	for ( int k = 0; k < 3; ++k )
		out[k] = nTexels ? float( sum[k] / nTexels ) : 0.0f;
	return nTexels > 0;
}
