//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's area lights (area_lights.h).
//
//===========================================================================//

#include "render_pch.h"
#include "area_lights.h"

#include "cl_main.h"
#include "client.h"
#include "cmodel_engine.h"
#include "convar.h"
#include "dlight.h"
#include "gl_model_private.h"
#include "host.h"
#include "r_efxextern.h"
#include "render_core_world_draw.h"
#include "world_emitters.h"
#include "render/energy_field.h"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_nMapLoadCount;

// Diagnostics: the light R_AddAreaLights added since the last report.
double g_AreaLightsAddedEnergy = 0.0;
int g_AreaLightsLitLuxels = 0;
double g_AreaLightsSeconds = 0.0; // in R_AddAreaLights
int g_AreaLightsSurfaces = 0;     // lightmap builds that ran it
static int s_nReportFrame = 0;

static ConVar r_area_lights_report( "r_area_lights_report", "0", FCVAR_CHEAT,
    "Print the next published area lights (RFC 0011 render.area-light.v1) once" );

namespace
{
using area_light::AreaLight;
using area_light::kMaxAreaLights;

// dlight keys the engine gives area lights: the base plus the client's key.
constexpr int kAreaKeyBase = 0x02000000;
constexpr int kAreaKeyMask = 0x00ffffff;
// A published light outlives its frame by this much, so its slot is kept
// until the client's next publish.
constexpr float kHold = 0.1f;
// A light stays the same version while within these of its published state.
constexpr float kPositionTolerance = 0.05f;
constexpr float kRadianceTolerance = 1.0f / 512.0f;
constexpr float kRadianceRelativeTolerance = 0.01f;

constexpr int kGenerations = 4;

struct Generation
{
	int count = 0;
	AreaLightSlot lights[kMaxAreaLights];
};

struct Applied
{
	int key;
	uint32_t version;
};

struct State
{
	int map = -1;
	int generation = 0;
	Generation ring[kGenerations];
	// The current lights by dlight slot (index into the current generation).
	int bySlot[MAX_DLIGHTS];
	uint32_t nextVersion = 1;
	// Surfaces' applied versions and the dirty set; guarded (lightmaps may be
	// built on the material system's thread).
	std::mutex mutex;
	std::unordered_map<msurface2_t *, std::vector<Applied>> applied;
	std::unordered_map<int, std::vector<msurface2_t *>> surfacesOfKey;
	std::unordered_set<msurface2_t *> dirty;
	// The frame's lights, most important first (at most
	// kMaxFrameAreaLights): those in the current generation with their
	// slots, the rest with slot -1. The light set publishes them all.
	std::vector<AreaLightSlot> frame;
};

State &S()
{
	static State state;
	return state;
}

const Generation &Current()
{
	return S().ring[S().generation % kGenerations];
}

bool Near( const float *a, const float *b, int n, float tolerance )
{
	for ( int i = 0; i < n; ++i )
		if ( std::fabs( a[i] - b[i] ) > tolerance )
			return false;
	return true;
}

// Whether `next` is within tolerance of the published `last`.
bool SameVersion( const AreaLight &last, const AreaLight &next )
{
	if ( last.rect.twoSided != next.rect.twoSided ||
	     !Near( last.rect.center, next.rect.center, 3, kPositionTolerance ) ||
	     !Near( last.rect.halfU, next.rect.halfU, 3, kPositionTolerance ) ||
	     !Near( last.rect.halfV, next.rect.halfV, 3, kPositionTolerance ) )
		return false;
	for ( int k = 0; k < 3; ++k )
	{
		const float tolerance = std::max(
		    kRadianceTolerance, kRadianceRelativeTolerance * std::fabs( last.radiance[k] ) );
		if ( std::fabs( last.radiance[k] - next.radiance[k] ) > tolerance )
			return false;
	}
	return true;
}

void ResetForMap()
{
	State &s = S();
	s.map = g_nMapLoadCount;
	s.generation = 0;
	for ( Generation &g : s.ring )
		g.count = 0;
	for ( int &slot : s.bySlot )
		slot = -1;
	s.frame.clear();
	std::lock_guard<std::mutex> lock( s.mutex );
	s.applied.clear();
	s.surfacesOfKey.clear();
	s.dirty.clear();
}

// Surfaces holding `key` (any version) must be rebuilt without it.
void DirtyKey( int key )
{
	State &s = S();
	std::lock_guard<std::mutex> lock( s.mutex );
	const auto found = s.surfacesOfKey.find( key );
	if ( found == s.surfacesOfKey.end() )
		return;
	for ( msurface2_t *surfID : found->second )
	{
		s.dirty.insert( surfID );
		// Into the frame's dlit surfaces, so the rebuild happens when visible.
		MSurf_Flags( surfID ) |= SURFDRAW_HASDLIGHT;
	}
	s.surfacesOfKey.erase( found );
}

void Darken( int slot )
{
	dlight_t &dl = cl_dlights[slot];
	if ( ( dl.flags & DLIGHT_AREA ) == 0 )
		return;
	dl.radius = 0.0f;
	dl.die = 0.0f;
	dl.flags &= ~DLIGHT_AREA;
}

// A slot for dlight key `dlightKey`: its own, or a free one; -1 when every
// slot is held (an area light never takes another light's slot).
int FindSlot( int dlightKey )
{
	const float now = cl.GetTime();
	for ( int i = 0; i < MAX_DLIGHTS; ++i )
		if ( cl_dlights[i].key == dlightKey )
			return i;
	// CL_AllocDlight's own rule for a free slot.
	for ( int i = 0; i < MAX_DLIGHTS; ++i )
		if ( cl_dlights[i].die < now )
			return i;
	return -1;
}

class CAreaLights final : public area_light::IAreaLights4
{
public:
	int GetEmissiveTriangles( int modelIndex, area_light::EmissiveTriangle *out, int max ) override
	{
		const auto triangles = WorldEmitters_CoreGeometry( modelIndex );
		for ( int i = 0; out && i < max && i < int( triangles.size() ); ++i )
			out[i] = triangles[size_t( i )];
		return int( triangles.size() );
	}
	int GetWorldEmitters( area_light::WorldEmitterInfo *out, int max ) override
	{
		const std::vector<WorldEmitter> &emitters = WorldEmitters_Get();
		for ( int i = 0; out && i < max && i < int( emitters.size() ); ++i )
		{
			out[i].light = emitters[size_t( i )].light;
			Q_strncpy( out[i].material, emitters[size_t( i )].material->GetName(),
			    sizeof( out[i].material ) );
		}
		return int( emitters.size() );
	}

	void SetAreaLights( const AreaLight *lights, const int *keys, int count ) override
	{
		SetFrameAreaLights( lights, keys, NULL, count );
	}

	bool GetEnergyFieldSurface( int modelIndex, energy_field::Surface &out ) override
	{
		return WorldEmitters_EnergyFieldSurface( modelIndex, out );
	}

	void SetFrameAreaLights(
	    const AreaLight *lights, const int *keys, const bool *coreOnly, int count ) override
	{
		State &s = S();
		if ( s.map != g_nMapLoadCount )
			ResetForMap();
		const Generation &previous = Current();
		Generation next;
		int dropped = 0;
		// Past the slots' budget (or with every slot held) a light reaches
		// the light set alone.
		std::vector<AreaLightSlot> slotless;
		for ( int i = 0; lights && keys && i < count; ++i )
		{
			if ( !( lights[i].reach > 0.0f ) ||
			     next.count + int( slotless.size() ) == area_light::kMaxFrameAreaLights )
			{
				++dropped;
				continue;
			}
			const int key = keys[i] & kAreaKeyMask;
			const int dlightKey = kAreaKeyBase | key;
			if ( ( coreOnly && coreOnly[i] ) || next.count == kMaxAreaLights ||
			     FindSlot( dlightKey ) < 0 )
			{
				AreaLightSlot entry;
				entry.key = key;
				entry.light = lights[i];
				slotless.push_back( entry );
				continue;
			}
			AreaLightSlot entry;
			entry.key = key;
			entry.light = lights[i];
			// Same light as last frame, within tolerance: keep its version and
			// the state it was published with.
			bool same = false;
			for ( int p = 0; p < previous.count; ++p )
			{
				const AreaLightSlot &last = previous.lights[p];
				if ( last.key == key && SameVersion( last.light, lights[i] ) )
				{
					entry.version = last.version;
					entry.light = last.light;
					same = true;
					break;
				}
			}
			if ( !same )
				entry.version = s.nextVersion++;

			dlight_t *dl = CL_AllocDlight( dlightKey );
			entry.slot = int( dl - cl_dlights );
			dl->flags = DLIGHT_AREA;
			dl->origin.Init( entry.light.rect.center[0], entry.light.rect.center[1],
			    entry.light.rect.center[2] );
			// The marking sphere: the reach beyond the rectangle's farthest point.
			const float halfDiagonal = std::sqrt(
			    area_light::detail::Dot( entry.light.rect.halfU, entry.light.rect.halfU ) +
			    area_light::detail::Dot( entry.light.rect.halfV, entry.light.rect.halfV ) );
			dl->radius = entry.light.reach + halfDiagonal;
			dl->die = cl.GetTime() + kHold;
			// Black: a path that does not know area lights adds nothing.
			dl->color.r = dl->color.g = dl->color.b = 0;
			dl->color.exponent = 0;
			next.lights[next.count++] = entry;
		}

		// Lights gone or changed: darken their old slots and dirty the
		// surfaces that hold them.
		for ( int p = 0; p < previous.count; ++p )
		{
			const AreaLightSlot &last = previous.lights[p];
			bool kept = false, keptSlot = false;
			for ( int n = 0; n < next.count; ++n )
			{
				if ( next.lights[n].key != last.key )
					continue;
				kept = next.lights[n].version == last.version;
				keptSlot = next.lights[n].slot == last.slot;
				break;
			}
			if ( !keptSlot && cl_dlights[last.slot].key == ( kAreaKeyBase | last.key ) )
				Darken( last.slot );
			if ( !kept )
				DirtyKey( last.key );
		}

		s.generation += 1;
		s.ring[s.generation % kGenerations] = next;
		for ( int &slot : s.bySlot )
			slot = -1;
		for ( int n = 0; n < next.count; ++n )
			s.bySlot[next.lights[n].slot] = n;
		s.frame.assign( next.lights, next.lights + next.count );
		s.frame.insert( s.frame.end(), slotless.begin(), slotless.end() );

		if ( r_area_lights_report.GetBool() )
		{
			r_area_lights_report.SetValue( 0 );
			size_t applied = 0, dirty = 0;
			{
				std::lock_guard<std::mutex> lock( s.mutex );
				applied = s.applied.size();
				dirty = s.dirty.size();
			}
			Msg( "area lights generation %d: %d lit (%zu without a slot), %d dropped; %zu "
			     "surface(s) hold area light, %zu "
			     "dirty; since the last report (%d frames): %d luxel(s) lit, %.3f light added, %d "
			     "lightmap build(s) took %.3f ms\n",
			    s.generation, next.count, slotless.size(), dropped, applied, dirty,
			    host_framecount - s_nReportFrame, g_AreaLightsLitLuxels, g_AreaLightsAddedEnergy,
			    g_AreaLightsSurfaces, g_AreaLightsSeconds * 1000.0 );
			g_AreaLightsLitLuxels = 0;
			g_AreaLightsAddedEnergy = 0.0;
			g_AreaLightsSurfaces = 0;
			g_AreaLightsSeconds = 0.0;
			s_nReportFrame = host_framecount;
			for ( int n = 0; n < next.count; ++n )
			{
				const AreaLightSlot &e = next.lights[n];
				const AreaLight &l = e.light;
				float normal[3];
				area_light::Normal( l.rect, normal );
				Msg( "  area %d key %d v%u slot %d at %.1f %.1f %.1f facing %.2f %.2f %.2f area "
				     "%.1f %s "
				     "radiance %.3f %.3f %.3f reach %.0f\n",
				    n, e.key, e.version, e.slot, l.rect.center[0], l.rect.center[1],
				    l.rect.center[2], normal[0], normal[1], normal[2], area_light::Area( l.rect ),
				    l.rect.twoSided ? "two-sided" : "one-sided", l.radiance[0], l.radiance[1],
				    l.radiance[2], l.reach );
			}
			// The lights the core evaluates without a CPU slot (the light set
			// carries them all).
			for ( size_t n = 0; n < slotless.size(); ++n )
			{
				const AreaLightSlot &e = slotless[n];
				const AreaLight &l = e.light;
				Msg( "  slotless area %zu key %d at %.1f %.1f %.1f radiance %.3f %.3f %.3f reach "
				     "%.0f\n",
				    n, e.key, l.rect.center[0], l.rect.center[1], l.rect.center[2], l.radiance[0],
				    l.radiance[1], l.radiance[2], l.reach );
			}
		}
	}
};

CAreaLights s_AreaLights;

// A slot's light, if the slot still carries it.
const AreaLightSlot *Live( int slot )
{
	if ( slot < 0 || slot >= MAX_DLIGHTS || S().map != g_nMapLoadCount )
		return nullptr;
	const int index = S().bySlot[slot];
	if ( index < 0 )
		return nullptr;
	const AreaLightSlot &entry = Current().lights[index];
	const dlight_t &dl = cl_dlights[slot];
	if ( ( dl.flags & DLIGHT_AREA ) == 0 || dl.key != ( kAreaKeyBase | entry.key ) ||
	     !dl.IsRadiusGreaterThanZero() )
		return nullptr;
	return &entry;
}

} // namespace

EXPOSE_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE(
    CAreaLights, area_light::, IAreaLights, area_light::kAreaLightsVersion, s_AreaLights );
EXPOSE_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE(
    CAreaLights, area_light::, IAreaLights3, area_light::kAreaLightsFrameVersion, s_AreaLights );
EXPOSE_SINGLE_INTERFACE_GLOBALVAR_WITH_NAMESPACE(
    CAreaLights, area_light::, IAreaLights4, area_light::kAreaLightsGeometryVersion, s_AreaLights );

int AreaLights_Generation()
{
	return S().generation;
}

int AreaLights_Get( int generation, AreaLightSlot *out )
{
	State &s = S();
	if ( s.map != g_nMapLoadCount )
		return 0;
	// An evicted (or future) generation reads as the current one.
	if ( generation > s.generation || generation <= s.generation - kGenerations )
		generation = s.generation;
	const Generation &g = s.ring[generation % kGenerations];
	for ( int i = 0; i < g.count; ++i )
		out[i] = g.lights[i];
	return g.count;
}

unsigned int AreaLights_SlotMask()
{
	unsigned int mask = 0;
	for ( int slot = 0; slot < MAX_DLIGHTS; ++slot )
		if ( Live( slot ) )
			mask |= 1u << slot;
	return mask;
}

const AreaLightSlot *AreaLights_ForSlot( int slot )
{
	return Live( slot );
}

int AreaLights_Frame( const AreaLightSlot **out )
{
	State &s = S();
	*out = s.frame.data();
	if ( s.map != g_nMapLoadCount )
		return 0;
	return int( s.frame.size() );
}

bool AreaLights_CoreOwns( msurface2_t *surfID )
{
	return RenderCoreWorldDraw_OwnsLighting( surfID );
}

bool AreaLights_IsWorldSurface( msurface2_t *surfID )
{
	const model_t *world = host_state.worldmodel;
	if ( !world || !host_state.worldbrush )
		return false;
	const int index = MSurf_Index( surfID );
	return index >= world->brush.firstmodelsurface &&
	       index < world->brush.firstmodelsurface + world->brush.nummodelsurfaces;
}

bool AreaLights_ShouldMark( int slot, msurface2_t *surfID, bool worldSurface )
{
	const AreaLightSlot *entry = Live( slot );
	if ( !entry )
		return false;
	State &s = S();
	// The render core lights the surfaces it draws (they take no area light
	// here): mark one only to clear the light its lightmap still holds.
	if ( AreaLights_CoreOwns( surfID ) )
	{
		std::lock_guard<std::mutex> lock( s.mutex );
		return worldSurface && s.applied.count( surfID ) != 0;
	}
	if ( !worldSurface )
		return true;
	std::lock_guard<std::mutex> lock( s.mutex );
	const auto found = s.applied.find( surfID );
	if ( found == s.applied.end() )
		return true;
	for ( const Applied &a : found->second )
		if ( a.key == entry->key && a.version == entry->version )
			return false;
	return true;
}

bool AreaLights_IsDirty( msurface2_t *surfID )
{
	State &s = S();
	std::lock_guard<std::mutex> lock( s.mutex );
	return s.dirty.count( surfID ) != 0;
}

void AreaLights_Applied(
    msurface2_t *surfID, bool worldSurface, const AreaLightSlot *lights, int count )
{
	State &s = S();
	std::lock_guard<std::mutex> lock( s.mutex );
	s.dirty.erase( surfID );
	if ( !worldSurface )
		return;
	std::vector<Applied> &applied = s.applied[surfID];
	applied.clear();
	for ( int i = 0; i < count; ++i )
	{
		applied.push_back( Applied{ lights[i].key, lights[i].version } );
		std::vector<msurface2_t *> &surfaces = s.surfacesOfKey[lights[i].key];
		if ( std::find( surfaces.begin(), surfaces.end(), surfID ) == surfaces.end() )
			surfaces.push_back( surfID );
	}
	if ( applied.empty() )
		s.applied.erase( surfID );
}

bool AreaLights_Representative( int slot, const Vector &receiver, dworldlight_t &out )
{
	const AreaLightSlot *entry = Live( slot );
	if ( !entry )
		return false;
	const float p[3] = { receiver.x, receiver.y, receiver.z };
	const area_light::Representative rep = area_light::RepresentativeAt( entry->light, p );
	if ( !rep.lit )
		return false;
	std::memset( &out, 0, sizeof( out ) );
	out.type = emit_point;
	out.origin.Init( rep.position[0], rep.position[1], rep.position[2] );
	out.intensity.Init( rep.intensityAt100[0], rep.intensityAt100[1], rep.intensityAt100[2] );
	out.quadratic_attn = 1.0f / ( area_light::kRepresentativeReferenceDistance *
	                                area_light::kRepresentativeReferenceDistance );
	// Far enough past the receiver that the cutoff never clips it.
	const Vector delta = out.origin - receiver;
	out.radius = 2.0f * delta.Length() + 1.0f;
	out.cluster = CM_LeafCluster( CM_PointLeafnum( out.origin ) );
	return true;
}

void AreaLights_AddAtPoint( int slot, const Vector &point, const Vector &normal, Vector &color )
{
	const AreaLightSlot *entry = Live( slot );
	if ( !entry )
		return;
	const float p[3] = { point.x, point.y, point.z };
	const float n[3] = { normal.x, normal.y, normal.z };
	float light[3];
	area_light::IrradianceAt( entry->light, p, n, light );
	color.x += light[0];
	color.y += light[1];
	color.z += light[2];
}
