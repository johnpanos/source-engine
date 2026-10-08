//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The Nintendo 3DS's platform settings, applied after config.cfg (so a saved
// desktop config cannot turn them back on) and directly on the variables, so
// the console's cheat check does not refuse the cheat-flagged ones. The 3DS
// draws through the fullbright PICA backend (materialsystem/shaderapipica):
// no render targets, no shadow depth, no lighting terms, one texture stage;
// it has ~178 MB of RAM and two to four slow cores. Every value here turns
// off work whose result that backend cannot show, or that the device cannot
// afford; the reason is next to it.
//
//=============================================================================//

#if defined( PLATFORM_3DS )

#include "tier1/convar.h"
#include "icvar.h"
#include "tier1/strtools.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
struct PlatformDefault
{
	const char *name;
	const char *value;
};

const PlatformDefault kDefaults[] =
{
	// Portal 2's system levels pick its own lowest presets (shadows, effects,
	// detail, model and texture budgets); the entries below then win.
	{ "cpu_level", "0" },
	{ "gpu_level", "0" },
	{ "mem_level", "0" },
	{ "gpu_mem_level", "0" },
	// The data cache's target (models' vertex data, animation blocks): the
	// desktop's 64 MB is never reached on the 3DS, so nothing is evicted and
	// model vertex data (15 MB on sp_a1_intro4) outlives the meshes built from
	// it; evicted data reloads from disk when used again.
	{ "datacachesize", "16" },
	// RFC 0026 P3: the render core draws the world (lightmapped) and the
	// models (Source's model lighting per vertex) with the reduced 3DS
	// material model on the PICA200 device; the dynamic hand-off carries the
	// models, since 3DS maps have no world stage (desktop gates it on queued,
	// capture and resize acceptance, none of which the 3DS has).
	{ "r_core_world", "1" },
	{ "r_core_dynamic_draws", "1" },
	// Post-processing samples render targets the backend never draws.
	{ "mat_motion_blur_enabled", "0" },
	{ "mat_disable_bloom", "1" },
	{ "mat_dynamic_tonemapping", "0" },
	{ "mat_hdr_level", "0" },
	// One texture stage, no per-pixel lighting: bump, specular, phong and
	// parallax inputs would be loaded and never sampled.
	{ "mat_bumpmap", "0" },
	{ "mat_specular", "0" },
	{ "mat_phong", "0" },
	{ "mat_pbr_parallaxmap", "0" },
	{ "mat_forceaniso", "0" },
	// Shadows: render-to-texture blob shadows and flashlight depth need
	// render targets.
	{ "r_shadows", "0" },
	{ "r_shadowrendertotexture", "0" },
	{ "r_flashlightdepthtexture", "0" },
	{ "r_flashlightupdatedepth", "0" },
	{ "r_flashlightrender", "0" },
	// Runtime lighting (RFC 0011 and the light set) feeds lighting the
	// fullbright backend does not shade: probe volume, indirect light,
	// projected lights, dynamic occlusion, area lights, portal dlights.
	{ "r_probevolume", "0" },
	{ "r_probevolume_visibility", "0" },
	{ "r_indirect_occlusion", "0" },
	{ "r_indirect_shadows", "0" },
	{ "r_indirect_portals", "0" },
	{ "r_indirect_focus", "0" },
	{ "r_indirect_probe_budget", "0" },
	{ "r_projected_lights", "0" },
	{ "r_dynamic_occlusion", "0" },
	{ "r_area_lights", "0" },
	{ "r_area_lights_world", "0" },
	{ "r_portal_dlights", "0" },
	{ "r_portal_use_dlights", "0" },
	{ "r_dynamiclighting", "0" },
	{ "r_maxdlights", "0" },
	{ "r_rimlight", "0" },
	{ "cl_surface_core_emission", "0" },
	{ "cl_fizzler_core_emission", "0" },
	// Water reflection and refraction and refract textures are render targets.
	{ "r_WaterDrawReflection", "0" },
	{ "r_WaterDrawRefraction", "0" },
	{ "r_updaterefracttexture", "0" },
	// Monitors render a second view into a render target.
	{ "cl_drawmonitors", "0" },
	// Detail sprites and model/static-prop decals: CPU and draw count the
	// device cannot spare.
	{ "r_DrawDetailProps", "0" },
	{ "r_decals", "64" },
	{ "r_decalstaticprops", "0" },
	{ "r_drawmodeldecals", "0" },
	{ "r_maxmodeldecal", "8" },
	// Paint blobs: the isosurface mesher runs on the CPU every frame.
	{ "r_paintblob_draw_isosurface", "0" },
	{ "r_threaded_blobulator", "0" },
	// Rope collision and smoothing cost CPU per segment.
	{ "rope_collide", "0" },
	{ "rope_smooth", "0" },
	{ "rope_subdiv", "0" },
	// Sound: the DSP chain runs per mixed sample on the CPU.
	{ "dsp_off", "1" },
	{ "snd_mixer_master_dsp", "0" },
};
} // namespace

// The platform value of a variable, or NULL (the video config consults this,
// so a saved desktop video config does not override the platform).
const char *N3ds_PlatformDefault( const char *name )
{
	for ( const PlatformDefault &entry : kDefaults )
		if ( !V_stricmp( entry.name, name ) )
			return entry.value;
	return NULL;
}

void N3ds_ApplyPlatformDefaults()
{
	// Every registered variable of a listed name: modules can register
	// same-named variables the cvar system does not link (the material
	// system's and the client's mat_motion_blur_enabled), so FindVar alone
	// would set only the first.
	int applied = 0;
	bool found[ARRAYSIZE( kDefaults )] = {};
	ICvar::Iterator iter( g_pCVar );
	for ( iter.SetFirst(); iter.IsValid(); iter.Next() )
	{
		ConCommandBase *base = iter.Get();
		if ( base->IsCommand() )
			continue;
		for ( int i = 0; i < ARRAYSIZE( kDefaults ); ++i )
		{
			if ( V_stricmp( base->GetName(), kDefaults[i].name ) )
				continue;
			static_cast<ConVar *>( base )->SetValue( kDefaults[i].value );
			found[i] = true;
			++applied;
		}
	}
	int missing = 0;
	for ( int i = 0; i < ARRAYSIZE( kDefaults ); ++i )
	{
		if ( !found[i] )
		{
			++missing;
			DevMsg( "n3ds defaults: no variable %s\n", kDefaults[i].name );
		}
	}
	Msg( "n3ds defaults: %d variables set, %d names not registered\n", applied, missing );
}

#endif // PLATFORM_3DS
