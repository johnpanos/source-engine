//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The render core's lighting settings as the video menus present
//			them: five ConVar choices and the Low/High presets over them
//			(RFC 0016 K12). Menu-independent, so a test drives it without VGUI.
//
//			The presets are the render_quality "low" and "high" settings of
//			quality/product_profiles/portal2-linux-native-vulkan-high.json for
//			these five ConVars; a change to either changes both.
//=============================================================================//

#ifndef GAMEUI_RENDER_CORE_LIGHTING_PRESET_H
#define GAMEUI_RENDER_CORE_LIGHTING_PRESET_H

#include <algorithm>

namespace gameui
{

// One value per ConVar, each the ConVar's integer value.
struct RenderCoreLighting
{
	int ambientOcclusion = 2; // r_core_ao_quality: 0 off .. 4 ultra
	int shadows = 2;          // r_core_shadow_quality: 0 off .. 3 high
	int depthPrepass = 1;     // r_core_depth_prepass: 0/1
	int shadowMovers = 1;     // r_core_shadow_movers: 0/1
	int runtimeDirect = 1;    // r_core_runtime_direct: 0 baked direct light, 1 runtime
	bool operator==( const RenderCoreLighting & ) const = default;
};

inline constexpr int kRenderCoreAOChoices = 5;
inline constexpr int kRenderCoreShadowChoices = 4;
inline constexpr int kRenderCoreToggleChoices = 2;

// The menu's rows, in display order after the preset row.
enum class RenderCoreLightingRow
{
	kAmbientOcclusion,
	kShadows,
	kDepthPrepass,
	kShadowMovers,
	kRuntimeDirect,
};

inline constexpr int RenderCoreLightingChoices( RenderCoreLightingRow row )
{
	switch ( row )
	{
	case RenderCoreLightingRow::kAmbientOcclusion:
		return kRenderCoreAOChoices;
	case RenderCoreLightingRow::kShadows:
		return kRenderCoreShadowChoices;
	default:
		return kRenderCoreToggleChoices;
	}
}

// Every value within its ConVar's range.
inline constexpr RenderCoreLighting Clamped( RenderCoreLighting lighting )
{
	lighting.ambientOcclusion =
	    std::clamp( lighting.ambientOcclusion, 0, kRenderCoreAOChoices - 1 );
	lighting.shadows = std::clamp( lighting.shadows, 0, kRenderCoreShadowChoices - 1 );
	lighting.depthPrepass = std::clamp( lighting.depthPrepass, 0, 1 );
	lighting.shadowMovers = std::clamp( lighting.shadowMovers, 0, 1 );
	lighting.runtimeDirect = std::clamp( lighting.runtimeDirect, 0, 1 );
	return lighting;
}

// One row's choice; the value is clamped to the row's range.
inline constexpr RenderCoreLighting WithChoice(
    RenderCoreLighting lighting, RenderCoreLightingRow row, int choice )
{
	switch ( row )
	{
	case RenderCoreLightingRow::kAmbientOcclusion:
		lighting.ambientOcclusion = choice;
		break;
	case RenderCoreLightingRow::kShadows:
		lighting.shadows = choice;
		break;
	case RenderCoreLightingRow::kDepthPrepass:
		lighting.depthPrepass = choice;
		break;
	case RenderCoreLightingRow::kShadowMovers:
		lighting.shadowMovers = choice;
		break;
	case RenderCoreLightingRow::kRuntimeDirect:
		lighting.runtimeDirect = choice;
		break;
	}
	return Clamped( lighting );
}

inline constexpr int Choice( const RenderCoreLighting &lighting, RenderCoreLightingRow row )
{
	switch ( row )
	{
	case RenderCoreLightingRow::kAmbientOcclusion:
		return lighting.ambientOcclusion;
	case RenderCoreLightingRow::kShadows:
		return lighting.shadows;
	case RenderCoreLightingRow::kDepthPrepass:
		return lighting.depthPrepass;
	case RenderCoreLightingRow::kShadowMovers:
		return lighting.shadowMovers;
	case RenderCoreLightingRow::kRuntimeDirect:
		return lighting.runtimeDirect;
	}
	return 0;
}

// The preset row's choices; its index is the menu's choice index. Custom is
// shown when the rows match no preset and is never applied.
enum class RenderCoreLightingPreset
{
	kLow,
	kHigh,
	kCustom,
};
inline constexpr int kRenderCoreLightingPresetChoices = 3;

// Low: no runtime direct light, shadow atlas or mover shadows; the world
// draws the bake's total lightmap layer, its direct light and shadows baked.
// High: every light's direct light at runtime, shadowed.
inline constexpr RenderCoreLighting PresetLighting( RenderCoreLightingPreset preset )
{
	if ( preset == RenderCoreLightingPreset::kLow )
		return RenderCoreLighting{ 1, 0, 1, 0, 0 };
	return RenderCoreLighting{ 3, 3, 1, 1, 1 };
}

inline constexpr RenderCoreLightingPreset ClassifyPreset( const RenderCoreLighting &lighting )
{
	for ( RenderCoreLightingPreset preset :
	    { RenderCoreLightingPreset::kLow, RenderCoreLightingPreset::kHigh } )
	{
		if ( Clamped( lighting ) == PresetLighting( preset ) )
			return preset;
	}
	return RenderCoreLightingPreset::kCustom;
}

// The preset row's choice: Low and High replace every row; Custom changes
// nothing (it only names a hand-edited combination).
inline constexpr RenderCoreLighting WithPreset(
    const RenderCoreLighting &lighting, int presetChoice )
{
	if ( presetChoice == int( RenderCoreLightingPreset::kLow ) )
		return PresetLighting( RenderCoreLightingPreset::kLow );
	if ( presetChoice == int( RenderCoreLightingPreset::kHigh ) )
		return PresetLighting( RenderCoreLightingPreset::kHigh );
	return Clamped( lighting );
}

} // namespace gameui

#endif // GAMEUI_RENDER_CORE_LIGHTING_PRESET_H
