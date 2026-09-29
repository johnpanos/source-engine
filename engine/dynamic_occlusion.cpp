//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's moving occluders (dynamic_occlusion.h).
//
//===========================================================================//

#include "render_pch.h"
#include "dynamic_occlusion.h"

#include "cmodel_engine.h"
#include "convar.h"
#include "gl_model_private.h"
#include "host.h"

#include <cmath>
#include <map>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_nMapLoadCount;

static ConVar r_dynamic_occlusion( "r_dynamic_occlusion", "1", 0,
    "Moving objects (props, physics objects, doors) block light from every light that reaches "
    "them, in world lightmaps and model lighting (RFC 0011 render.dynamic-occlusion.v1); their "
    "blob shadows are not drawn" );
static ConVar r_dynamic_occlusion_report( "r_dynamic_occlusion_report", "0", FCVAR_CHEAT,
    "Print the next published occluders and the surfaces they dirtied, once" );
static ConVar r_dynamic_occlusion_verify( "r_dynamic_occlusion_verify", "0", FCVAR_CHEAT,
    "Check every lightmap occlusion build against one from nothing with every box near the "
    "surface (the texel regions each box can shadow are a prefilter, and builds redo only the "
    "regions of boxes that changed); mismatches are counted in the report" );

// Diagnostics since the last report (gl_lightmap.cpp R_ApplyDynamicOcclusion).
double g_OcclusionSeconds = 0.0;
double g_OcclusionRemoved = 0.0;
int g_OcclusionLuxels = 0;
int g_OcclusionSurfaces = 0;
int g_OcclusionVerified = 0;        // texels checked (r_dynamic_occlusion_verify)
int g_OcclusionMismatches = 0;      // of them, a different result
int g_OcclusionTexelsEvaluated = 0; // texels evaluated again by lightmap builds
static int s_nReportFrame = 0;
static int s_nModelLights = 0;        // model lights judged
static int s_nModelLightsBlocked = 0; // of them, dimmed by a box

namespace
{
using dynamic_occlusion::Box;

constexpr float kCenterTolerance = 0.05f;
constexpr float kAxisTolerance = 0.02f;
constexpr int kGenerations = 4;

struct State
{
	int map = -1;
	int generation = 0;
	std::vector<OccluderEntry> ring[kGenerations];
	uint32_t nextVersion = 1;
	std::mutex mutex; // the dirty set and the static-visibility cache
	std::unordered_set<msurface2_t *> dirty;
	std::unordered_map<uint64_t, bool> staticVisible;
};

State &S()
{
	static State state;
	return state;
}

void ResetForMap()
{
	State &s = S();
	s.map = g_nMapLoadCount;
	s.generation = 0;
	for ( std::vector<OccluderEntry> &g : s.ring )
		g.clear();
	std::lock_guard<std::mutex> lock( s.mutex );
	s.dirty.clear();
	s.staticVisible.clear();
}

bool SamePose( const Box &a, const Box &b )
{
	for ( int k = 0; k < 3; ++k )
	{
		if ( std::fabs( a.center[k] - b.center[k] ) > kCenterTolerance )
			return false;
		for ( int ax = 0; ax < 3; ++ax )
			if ( std::fabs( a.axes[ax][k] - b.axes[ax][k] ) > kAxisTolerance )
				return false;
	}
	return true;
}

// Dirties the world surfaces within `radius` of `center` (a node walk, as
// dlights mark surfaces).
int DirtyNode(
    mnode_t *node, const Vector &center, float radius, std::unordered_set<msurface2_t *> &dirty )
{
	if ( node->contents >= 0 )
		return 0;
	const float dist = DotProduct( center, node->plane->normal ) - node->plane->dist;
	if ( dist > radius )
		return DirtyNode( node->children[0], center, radius, dirty );
	if ( dist < -radius )
		return DirtyNode( node->children[1], center, radius, dirty );
	int count = 0;
	SurfaceHandle_t surfID = SurfaceHandleFromIndex( node->firstsurface );
	for ( int i = 0; i < node->numsurfaces; ++i, ++surfID )
	{
		if ( MSurf_Flags( surfID ) & ( SURFDRAW_NOLIGHT | SURFDRAW_NODRAW ) )
			continue;
		if ( dirty.insert( surfID ).second )
		{
			MSurf_Flags( surfID ) |= SURFDRAW_HASDLIGHT;
			++count;
		}
	}
	count += DirtyNode( node->children[0], center, radius, dirty );
	return count + DirtyNode( node->children[1], center, radius, dirty );
}

int DirtyAround( const Box &box )
{
	if ( !host_state.worldbrush || !host_state.worldbrush->nodes )
		return 0;
	const Vector center( box.center[0], box.center[1], box.center[2] );
	const float radius = DynamicOcclusion_Reach( box ) + dynamic_occlusion::Radius( box );
	State &s = S();
	std::lock_guard<std::mutex> lock( s.mutex );
	return DirtyNode( host_state.worldbrush->nodes, center, radius, s.dirty );
}

class COccluders final : public dynamic_occlusion::IOccluders
{
public:
	void SetOccluders( const Box *boxes, int count ) override
	{
		State &s = S();
		if ( s.map != g_nMapLoadCount )
			ResetForMap();
		const std::vector<OccluderEntry> &previous = s.ring[s.generation % kGenerations];
		std::map<std::pair<int, int>, const OccluderEntry *> last;
		for ( const OccluderEntry &entry : previous )
			last[std::make_pair( entry.box.entity, entry.box.part )] = &entry;

		std::vector<OccluderEntry> next;
		next.reserve( size_t( count ) );
		int changed = 0, dirtied = 0;
		const bool enabled = r_dynamic_occlusion.GetBool();
		for ( int i = 0; enabled && boxes && i < count && i < dynamic_occlusion::kMaxOccluders;
		    ++i )
		{
			OccluderEntry entry;
			entry.box = boxes[i];
			const auto found = last.find( std::make_pair( boxes[i].entity, boxes[i].part ) );
			if ( found != last.end() && SamePose( found->second->box, boxes[i] ) )
			{
				entry = *found->second; // keep the pose it was published with
				last.erase( found );
			}
			else
			{
				entry.version = s.nextVersion++;
				++changed;
				dirtied += DirtyAround( entry.box );
				if ( found != last.end() )
				{
					dirtied += DirtyAround( found->second->box );
					last.erase( found );
				}
			}
			next.push_back( entry );
		}
		// Boxes gone: their shadows go too.
		for ( const auto &gone : last )
		{
			dirtied += DirtyAround( gone.second->box );
			++changed;
		}

		s.generation += 1;
		s.ring[s.generation % kGenerations] = std::move( next );
		if ( r_dynamic_occlusion_report.GetBool() )
		{
			r_dynamic_occlusion_report.SetValue( 0 );
			const std::vector<OccluderEntry> &now = s.ring[s.generation % kGenerations];
			size_t dirty = 0, cached = 0;
			{
				std::lock_guard<std::mutex> lock( s.mutex );
				dirty = s.dirty.size();
				cached = s.staticVisible.size();
			}
			Msg( "dynamic occlusion generation %d: %zu box(es), %d changed, %d surface(s) dirtied, "
			     "%zu dirty, %zu static visibility answer(s) cached; since the last report (%d "
			     "frames): %d lightmap build(s) took %.3f ms, %d texel(s) lost %.3f light\n",
			    s.generation, now.size(), changed, dirtied, dirty, cached,
			    host_framecount - s_nReportFrame, g_OcclusionSurfaces, g_OcclusionSeconds * 1000.0,
			    g_OcclusionLuxels, g_OcclusionRemoved );
			Msg( "  model lights: %d judged, %d dimmed by a box\n", s_nModelLights,
			    s_nModelLightsBlocked );
			Msg( "  lightmap builds evaluated %d texel(s) again\n", g_OcclusionTexelsEvaluated );
			if ( r_dynamic_occlusion_verify.GetBool() )
				Msg( "  verify: %d texel(s) rebuilt from nothing against every near box, %d "
				     "mismatch(es)\n",
				    g_OcclusionVerified, g_OcclusionMismatches );
			g_OcclusionVerified = g_OcclusionMismatches = g_OcclusionTexelsEvaluated = 0;
			s_nModelLights = s_nModelLightsBlocked = 0;
			s_nReportFrame = host_framecount;
			g_OcclusionSeconds = g_OcclusionRemoved = 0.0;
			g_OcclusionLuxels = g_OcclusionSurfaces = 0;
			for ( const OccluderEntry &e : now )
				Msg( "  box entity %d part %d v%u at %.1f %.1f %.1f radius %.1f reach %.0f\n",
				    e.box.entity, e.box.part, e.version, e.box.center[0], e.box.center[1],
				    e.box.center[2], dynamic_occlusion::Radius( e.box ),
				    DynamicOcclusion_Reach( e.box ) );
		}
	}
};

COccluders s_Occluders;

} // namespace

EXPOSE_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE( COccluders, dynamic_occlusion::, IOccluders,
    dynamic_occlusion::kOccludersVersion, s_Occluders );

bool DynamicOcclusion_Enabled()
{
	return r_dynamic_occlusion.GetBool();
}

int DynamicOcclusion_Generation()
{
	return S().generation;
}

const std::vector<OccluderEntry> &DynamicOcclusion_Get( int generation )
{
	static const std::vector<OccluderEntry> none;
	State &s = S();
	if ( s.map != g_nMapLoadCount || !r_dynamic_occlusion.GetBool() )
		return none;
	if ( generation > s.generation || generation <= s.generation - kGenerations )
		generation = s.generation;
	return s.ring[generation % kGenerations];
}

float DynamicOcclusion_Reach( const dynamic_occlusion::Box &box )
{
	const float reach = kShadowReachScale * dynamic_occlusion::Radius( box );
	return reach < kMaxShadowReach ? reach : kMaxShadowReach;
}

bool DynamicOcclusion_IsDirty( msurface2_t *surfID )
{
	State &s = S();
	std::lock_guard<std::mutex> lock( s.mutex );
	return s.dirty.count( surfID ) != 0;
}

void DynamicOcclusion_Rebuilt( msurface2_t *surfID )
{
	State &s = S();
	std::lock_guard<std::mutex> lock( s.mutex );
	s.dirty.erase( surfID );
}

bool DynamicOcclusion_StaticVisibleTo( msurface2_t *surfID, int luxel, int lightKey,
    const Vector &point, const Vector &normal, const Vector &target, bool sky )
{
	State &s = S();
	const uint64_t key = ( uint64_t( uint32_t( MSurf_Index( surfID ) ) ) << 32 ) |
	                     ( uint64_t( uint32_t( luxel ) & 0xfffff ) << 12 ) |
	                     uint64_t( uint32_t( lightKey ) & 0xfff );
	{
		std::lock_guard<std::mutex> lock( s.mutex );
		const auto found = s.staticVisible.find( key );
		if ( found != s.staticVisible.end() )
			return found->second;
	}
	const Vector start = point + normal * 1.0f;
	Vector toTarget = target - start;
	const float length = toTarget.NormalizeInPlace();
	const Vector end = sky ? target : start + toTarget * ( length > 2.0f ? length - 2.0f : 0.0f );
	Ray_t ray;
	ray.Init( start, end );
	trace_t tr;
	CM_BoxTrace( ray, 0, MASK_OPAQUE, false, tr );
	const bool visible = tr.fraction >= 0.999f || ( sky && ( tr.surface.flags & SURF_SKY ) );
	std::lock_guard<std::mutex> lock( s.mutex );
	s.staticVisible[key] = visible;
	return visible;
}

bool DynamicOcclusion_StaticVisible(
    msurface2_t *surfID, int luxel, int light, const Vector &point, const Vector &normal )
{
	const dworldlight_t &wl = host_state.worldbrush->worldlights[light];
	if ( wl.type == emit_skylight )
		return DynamicOcclusion_StaticVisibleTo( surfID, luxel, light, point, normal,
		    point + normal * 1.0f - wl.normal * dynamic_occlusion::kDistantLength, true );
	return DynamicOcclusion_StaticVisibleTo(
	    surfID, luxel, light, point, normal, wl.origin, false );
}

void DynamicLightmaps_DirtySphere( const Vector &center, float radius )
{
	if ( !host_state.worldbrush || !host_state.worldbrush->nodes )
		return;
	State &s = S();
	if ( s.map != g_nMapLoadCount )
		ResetForMap();
	std::lock_guard<std::mutex> lock( s.mutex );
	DirtyNode( host_state.worldbrush->nodes, center, radius, s.dirty );
}

float DynamicOcclusion_VisibilityFrom(
    const Vector &lightPosition, float lightRadius, const Vector &receiver, int self )
{
	const std::vector<OccluderEntry> &entries =
	    DynamicOcclusion_Get( DynamicOcclusion_Generation() );
	if ( entries.empty() )
		return 1.0f;
	const float r[3] = { receiver.x, receiver.y, receiver.z };
	const float l[3] = { lightPosition.x, lightPosition.y, lightPosition.z };
	static thread_local std::vector<dynamic_occlusion::Box> near;
	near.clear();
	for ( const OccluderEntry &entry : entries )
	{
		if ( self != 0 && entry.box.entity == self )
			continue;
		if ( dynamic_occlusion::SegmentNearSphere(
		         r, l, entry.box.center, dynamic_occlusion::Radius( entry.box ) + lightRadius ) )
			near.push_back( entry.box );
	}
	if ( near.empty() )
		return 1.0f;
	return dynamic_occlusion::Visibility(
	    r, dynamic_occlusion::DiskSamples( l, lightRadius, r ), near, self );
}

dynamic_occlusion::Samples DynamicOcclusion_WorldLightSamples(
    const dworldlight_t &light, const Vector &receiver )
{
	if ( light.type == emit_skylight )
	{
		const float r[3] = { receiver.x, receiver.y, receiver.z };
		const float d[3] = { light.normal.x, light.normal.y, light.normal.z };
		return dynamic_occlusion::DistantSamples( r, d );
	}
	const float p[3] = { light.origin.x, light.origin.y, light.origin.z };
	const float r[3] = { receiver.x, receiver.y, receiver.z };
	return dynamic_occlusion::DiskSamples( p, DynamicOcclusion_WorldLightRadius( light ), r );
}

float DynamicOcclusion_WorldLightRadius( const dworldlight_t &light )
{
	if ( light.type == emit_skylight )
		return 0.0f;
	return light.type == emit_surface ? dynamic_occlusion::kSurfaceLightRadius
	                                  : dynamic_occlusion::kPointLightRadius;
}

bool DynamicOcclusion_Verify()
{
	return r_dynamic_occlusion_verify.GetBool();
}

float DynamicOcclusion_ModelVisibility(
    const dworldlight_t &light, const Vector &receiver, int self )
{
	const std::vector<OccluderEntry> &entries =
	    DynamicOcclusion_Get( DynamicOcclusion_Generation() );
	if ( entries.empty() || light.type == emit_skyambient )
		return 1.0f;
	const dynamic_occlusion::Samples samples =
	    DynamicOcclusion_WorldLightSamples( light, receiver );
	// Only the boxes near the path to the light (its disk included) can block it.
	const float r[3] = { receiver.x, receiver.y, receiver.z };
	const float *target = samples.point[0];
	static thread_local std::vector<dynamic_occlusion::Box> near;
	near.clear();
	for ( const OccluderEntry &entry : entries )
	{
		if ( self != 0 && entry.box.entity == self )
			continue;
		if ( dynamic_occlusion::SegmentNearSphere( r, target, entry.box.center,
		         dynamic_occlusion::Radius( entry.box ) + dynamic_occlusion::kSurfaceLightRadius ) )
			near.push_back( entry.box );
	}
	++s_nModelLights;
	if ( near.empty() )
		return 1.0f;
	const float visible = dynamic_occlusion::Visibility( r, samples, near, self );
	s_nModelLightsBlocked += visible < 1.0f;
	return visible;
}
