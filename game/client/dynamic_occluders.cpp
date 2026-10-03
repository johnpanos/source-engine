//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frame's moving objects as occluders (render.dynamic-
//          occlusion.v1, RFC 0011): one rule for every prop, physics object,
//          NPC and door, published to the engine, which blocks each light they
//          stand in front of in world lightmaps and model lighting.
//
//          Which: every drawn entity that casts a dynamic shadow in the game's
//          own shadow model (ShadowCastType is not none: the render-to-texture
//          shadows these occluders replace), within kMaxDistance of the view,
//          nearest first. So the map's authoring holds: disableshadows
//          (EF_NOSHADOW) turns an object's shadow off, and brush entities
//          (func_brush, func_door, iris panels, pit housings) cast none, as in
//          retail, where the bake owns their light. Physics brushes
//          (func_physbox) and every studio model that casts keep casting.
//          (Brush entities were occluders until 2026-09-28: their solid model
//          boxes, sunk in floors and pits, shadowed the static floor around
//          sp_a2_core's receptacle and stalemate button; RFC/0011-progress.md.)
//
//          Boxes larger than kMaxBoxRadius are left out: they are hollow
//          environment pieces (elevator cars), which a solid box would black
//          out inside.
//
//          Boxes: an animated model drawn last frame gives its hitboxes on its
//          bones (a turret's legs, GLaDOS's segments); anything else its
//          collision box (a cube, a door), or its render box without one. Each
//          is keyed by the entity's handle and the box's index.
//
//          Published after the frame's views are drawn (PostRender), when the
//          frame's bones are set up, for the next frame's lighting.
//
//          Core physical bone followers publish current triangle poses at the
//          end of render-start animation work through VEngineOccluders002.
//          The core owns their GPU shadow visibility; legacy boxes stay separate.
//
//=============================================================================//
#include "cbase.h"
#include "dynamic_occluders.h"

#include "c_baseanimating.h"
#include "bone_setup.h"
#include "cdll_client_int.h"
#include "collisionproperty.h"
#include "igamesystem.h"
#include "model_types.h"
#include "render/dynamic_occlusion.h"
#include "studio.h"
#include "view.h"
#include "vcollide_parse.h"
#include "tier1/KeyValues.h"

#include <algorithm>
#include <vector>
#include <map>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
using dynamic_occlusion::Box;

const float kMaxDistance = 3072.0f;
// A box thinner than this is not worth a shadow.
const float kMinHalfExtent = 0.5f;
// A box larger than this (radius) is a hollow environment piece (an elevator
// car, a moving room), not a solid object: a solid box would black out what is
// inside it. Props, physics objects, NPC parts and doors are smaller.
const float kMaxBoxRadius = 160.0f;

bool s_bActive = false;

// The engine's switch (engine/dynamic_occlusion.cpp r_dynamic_occlusion).
bool EngineOcclusionEnabled()
{
	static ConVarRef r_dynamic_occlusion( "r_dynamic_occlusion" );
	return occluders != NULL && r_dynamic_occlusion.IsValid() && r_dynamic_occlusion.GetBool();
}

// A box from entity-space bounds placed by a transform.
bool BoxFromBounds( const Vector &mins, const Vector &maxs, const matrix3x4_t &toWorld, Box &out )
{
	const Vector half = 0.5f * ( maxs - mins );
	if ( half.x < kMinHalfExtent && half.y < kMinHalfExtent && half.z < kMinHalfExtent )
		return false;
	if ( half.Length() > kMaxBoxRadius )
		return false;
	Vector center;
	VectorTransform( 0.5f * ( mins + maxs ), toWorld, center );
	for ( int k = 0; k < 3; ++k )
		out.center[k] = center[k];
	for ( int a = 0; a < 3; ++a )
	{
		Vector axis;
		MatrixGetColumn( toWorld, a, axis );
		axis *= MAX( half[a], kMinHalfExtent );
		for ( int k = 0; k < 3; ++k )
			out.axes[a][k] = axis[k];
	}
	return true;
}

// Whether the entity blocks light as a moving object: the same set that casts
// render-to-texture shadows without occlusion.
bool IsOccluder( C_BaseEntity *pEnt )
{
	if ( pEnt->IsDormant() || pEnt->IsEffectActive( EF_NODRAW ) ||
	     pEnt->GetRenderMode() == kRenderNone || pEnt->entindex() <= 0 )
		return false;
	const model_t *pModel = pEnt->GetModel();
	if ( !pModel )
		return false;
	const int nType = modelinfo->GetModelType( pModel );
	if ( nType != mod_brush && nType != mod_studio )
		return false;
	// The retail shadow caster set: a brush entity casts only where its class
	// says so (C_PhysBox), a studio model unless its shadow is disabled.
	return pEnt->ShadowCastType() != SHADOWS_NONE && pEnt->ShouldDraw();
}

// Appends the entity's boxes.
void AddBoxes( C_BaseEntity *pEnt, std::vector<Box> &out )
{
	const int nKey = pEnt->GetRefEHandle().ToInt();
	const model_t *pModel = pEnt->GetModel();
	int nPart = 0;
	C_BaseAnimating *pAnim = pEnt->GetBaseAnimating();
	if ( pAnim && pAnim->IsBoneCacheValid() )
	{
		CStudioHdr *pHdr = pAnim->GetModelPtr();
		const mstudiohitboxset_t *pSet =
		    pHdr && pHdr->IsValid() ? pHdr->pHitboxSet( pAnim->GetHitboxSet() ) : NULL;
		if ( pSet && pSet->numhitboxes > 1 )
		{
			for ( int i = 0; i < pSet->numhitboxes; ++i )
			{
				const mstudiobbox_t *pBox = pSet->pHitbox( i );
				if ( pBox->bone < 0 || pBox->bone >= pHdr->numbones() )
					continue;
				Box box;
				if ( !BoxFromBounds( pBox->bbmin, pBox->bbmax, pAnim->GetBone( pBox->bone ), box ) )
					continue;
				box.entity = nKey;
				box.part = nPart++;
				out.push_back( box );
			}
			if ( nPart > 0 )
				return;
		}
	}
	Box box;
	CCollisionProperty *pCollision = pEnt->CollisionProp();
	Vector mins, maxs;
	if ( pCollision && ( pCollision->OBBMaxs() - pCollision->OBBMins() ).LengthSqr() > 0.0f )
	{
		mins = pCollision->OBBMins();
		maxs = pCollision->OBBMaxs();
		if ( !BoxFromBounds( mins, maxs, pCollision->CollisionToWorldTransform(), box ) )
			return;
	}
	else
	{
		modelinfo->GetModelBounds( pModel, mins, maxs );
		if ( !BoxFromBounds( mins, maxs, pEnt->EntityToWorldTransform(), box ) )
			return;
	}
	box.entity = nKey;
	box.part = 0;
	out.push_back( box );
}

// Models using custom bone-follower collision supply their actual rigid
// leaves to core shadows, even when their legacy blob shadow is disabled.
// The model selects named followers or the server's multi-solid fallback.
// Local geometry is immutable per model; only the current client pose changes.
struct PhysicalPart
{
	int bone;
	int part;
	std::vector<Vector> vertices;
};
std::map<int, std::vector<PhysicalPart>> s_PhysicalParts;

const std::vector<PhysicalPart> &PhysicalParts( C_BaseAnimating &entity )
{
	const int modelIndex = entity.GetModelIndex();
	auto [held, inserted] = s_PhysicalParts.try_emplace( modelIndex );
	if ( !inserted )
		return held->second;
	auto &parts = held->second;
	KeyValues *keys = new KeyValues( "model" );
	KeyValues::AutoDelete deleteKeys( keys );
	const char *text = modelinfo->GetModelKeyValueText( entity.GetModel() );
	if ( text )
		(void)keys->LoadFromBuffer( "model", text );
	KeyValues *followers = keys->FindKey( "bone_followers" );

	CStudioHdr *hdr = entity.GetModelPtr();
	vcollide_t *collide = modelinfo->GetVCollide( modelIndex );
	if ( !hdr || !hdr->IsValid() || !collide || !collide->pKeyValues )
	{
		Warning(
		    "Core physical shadows: model %d has unavailable bone-follower data\n", modelIndex );
		return parts;
	}
	std::map<int, int> requested;
	for ( KeyValues *key = followers ? followers->GetFirstSubKey() : NULL; key;
	    key = key->GetNextKey() )
	{
		const int bone = Studio_BoneIndexByName( hdr, key->GetString() );
		if ( bone >= 0 && bone < MAXSTUDIOBONES )
			requested[hdr->pBone( bone )->physicsbone] = bone;
	}
	IVPhysicsKeyParser *parser = physcollision->VPhysicsKeyParserCreate( collide->pKeyValues );
	const bool ragdollFallback = requested.empty() && collide->solidCount > 1;
	bool valid = parser != NULL && ( ragdollFallback || !requested.empty() );
	while ( valid && !parser->Finished() )
	{
		if ( Q_stricmp( parser->GetCurrentBlockName(), "solid" ) )
		{
			parser->SkipBlock();
			continue;
		}
		solid_t solid;
		parser->ParseSolid( &solid, NULL );
		const auto follower = requested.find( solid.index );
		if ( !ragdollFallback && follower == requested.end() )
			continue;
		int bone = Studio_BoneIndexByName( hdr, solid.name );
		if ( bone < 0 && follower != requested.end() )
			bone = follower->second; // the server bone-follower fallback
		Vector *mesh = NULL;
		const int count =
		    solid.index >= 0 && solid.index < collide->solidCount
		        ? physcollision->CreateDebugMesh( collide->solids[solid.index], &mesh )
		        : 0;
		valid = bone >= 0 && bone < MAXSTUDIOBONES && mesh && count > 0 && count % 3 == 0 &&
		        count <= 65536;
		if ( valid )
			parts.push_back( { bone, solid.index, { mesh, mesh + count } } );
		if ( mesh )
			physcollision->DestroyDebugMesh( count, mesh );
	}
	if ( parser )
		physcollision->VPhysicsKeyParserDestroy( parser );
	if ( !valid || parts.size() != ( ragdollFallback ? static_cast<size_t>( collide->solidCount )
	                                                 : requested.size() ) )
	{
		parts.clear();
		Warning(
		    "Core physical shadows: model %d has invalid bone-follower geometry\n", modelIndex );
	}
	return parts;
}

class CDynamicOccluders : public CAutoGameSystemPerFrame
{
public:
	CDynamicOccluders()
	    : CAutoGameSystemPerFrame( "CDynamicOccluders" ), m_nFrame( -1 ), m_bPublished( false )
	{
	}

	virtual void LevelShutdownPostEntity()
	{
		m_bPublished = false;
		s_PhysicalParts.clear();
		if ( coreOccluders )
			(void)coreOccluders->SetCoreOccluders( nullptr, 0 );
	}

	void PublishCore()
	{
		if ( !coreOccluders )
			return;
		static ConVarRef coreWorld( "r_core_world" );
		std::vector<std::vector<float>> vertices;
		std::vector<dynamic_occlusion::TriangleInput> inputs;
		if ( coreWorld.IsValid() && coreWorld.GetInt() == 1 && engine->IsInGame() )
		{
			for ( C_BaseEntity *ent = ClientEntityList().FirstBaseEntity(); ent;
			    ent = ClientEntityList().NextBaseEntity( ent ) )
			{
				C_BaseAnimating *anim = ent->GetBaseAnimating();
				const model_t *model = ent->GetModel();
				if ( !anim || !model || ent->IsDormant() || ent->IsEffectActive( EF_NODRAW ) ||
				     !ent->ShouldDraw() || ent->GetRenderMode() != kRenderNormal ||
				     !ent->IsSolidFlagSet( FSOLID_CUSTOMBOXTEST ) ||
				     !ent->IsSolidFlagSet( FSOLID_CUSTOMRAYTEST ) ||
				     modelinfo->IsTranslucent( model ) )
					continue;
				const auto &parts = PhysicalParts( *anim );
				if ( parts.empty() )
					continue;
				matrix3x4_t bones[MAXSTUDIOBONES];
				if ( !anim->SetupBones(
				         bones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime ) )
				{
					Warning(
					    "Core physical shadows: entity %d bone setup failed\n", ent->entindex() );
					return; // keep the preceding complete publication
				}
				for ( const PhysicalPart &part : parts )
				{
					auto &positions = vertices.emplace_back();
					positions.reserve( part.vertices.size() * 3 );
					for ( const Vector &local : part.vertices )
					{
						Vector world;
						VectorTransform( local, bones[part.bone], world );
						positions.insert( positions.end(), { world.x, world.y, world.z } );
					}
					inputs.push_back( { ent->GetRefEHandle().ToInt(), part.part, positions.data(),
					    static_cast<unsigned int>( part.vertices.size() ) } );
				}
			}
		}
		if ( !coreOccluders->SetCoreOccluders( inputs.data(), inputs.size() ) )
			Warning( "Core physical shadows: frame publication refused\n" );
	}

	virtual void PostRender()
	{
		if ( m_nFrame == gpGlobals->framecount )
			return;
		m_nFrame = gpGlobals->framecount;
		s_bActive = EngineOcclusionEnabled() && engine->IsInGame();
		if ( !s_bActive )
		{
			if ( m_bPublished && occluders )
				occluders->SetOccluders( NULL, 0 );
			m_bPublished = false;
			return;
		}

		const Vector &vecView = MainViewOrigin();
		std::vector<std::pair<float, C_BaseEntity *>> candidates;
		for ( C_BaseEntity *pEnt = ClientEntityList().FirstBaseEntity(); pEnt;
		    pEnt = ClientEntityList().NextBaseEntity( pEnt ) )
		{
			if ( !IsOccluder( pEnt ) )
				continue;
			const float flDist2 = vecView.DistToSqr( pEnt->WorldSpaceCenter() );
			if ( flDist2 > kMaxDistance * kMaxDistance )
				continue;
			candidates.emplace_back( flDist2, pEnt );
		}
		std::sort( candidates.begin(), candidates.end(),
		    []( const std::pair<float, C_BaseEntity *> &a,
		        const std::pair<float, C_BaseEntity *> &b )
		    {
			    return a.first < b.first;
		    } );
		std::vector<Box> boxes;
		for ( const auto &candidate : candidates )
		{
			if ( (int)boxes.size() >= dynamic_occlusion::kMaxOccluders )
				break;
			AddBoxes( candidate.second, boxes );
		}
		if ( (int)boxes.size() > dynamic_occlusion::kMaxOccluders )
			boxes.resize( dynamic_occlusion::kMaxOccluders );
		occluders->SetOccluders( boxes.data(), (int)boxes.size() );
		m_bPublished = true;
	}

private:
	int m_nFrame;
	bool m_bPublished;
};

CDynamicOccluders s_DynamicOccluders;

} // namespace

void DynamicOccluders_PublishCore()
{
	s_DynamicOccluders.PublishCore();
}

bool DynamicOccluders_Active()
{
	return s_bActive;
}
