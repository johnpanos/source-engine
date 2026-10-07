//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The debug view catalog (RFC 0014 render.debug-views.v1); see
//			render/shaderlib/debug_view.h.
//
//=============================================================================//

#include "render/shaderlib/debug_view.h"

#include <bit>

namespace render::shaderlib
{

namespace
{

// Views 18-23 and 32-36 arrive with their terms (the indirect terms, shadows,
// clusters and volumetrics on the core; the world batches and charts with the
// lab's BSP2 views) and are reserved until then.
constexpr DebugViewInfo kCatalog[] = { { DebugView::kOff, "off", 0, false, true },
    { DebugView::kAlbedo, "albedo", kDebugInputAlbedo, false, true },
    { DebugView::kWorldNormal, "world_normal", kDebugInputNormal, false, true },
    { DebugView::kNormalMap, "normal_map", kDebugInputNormalMap, false, true },
    { DebugView::kRoughness, "roughness", kDebugInputRoughness, false, true },
    { DebugView::kFilteredRoughness, "filtered_roughness", kDebugInputFilteredRoughness, false,
        true },
    { DebugView::kMetalness, "metalness", kDebugInputMetalness, false, true },
    { DebugView::kAmbientOcclusion, "ao", kDebugInputAo, false, true },
    { DebugView::kBakedLight, "baked_light", kDebugInputBaked, true, true },
    { DebugView::kDirectLight, "direct_light", kDebugInputDirect, true, true },
    { DebugView::kImageSpecular, "image_specular", kDebugInputImageSpecular, true, true },
    { DebugView::kScreenSpaceReflections, "ssr", kDebugInputSsr, true, true },
    { DebugView::kEmissive, "emissive", kDebugInputEmission, true, true },
    { DebugView::kUvChecker, "uv_checker", kDebugInputUv0, false, true },
    { DebugView::kVertexColor, "vertex_color", kDebugInputVertexColor, false, true },
    { DebugView::kLinearDepth, "linear_depth", kDebugInputDepth, false, true },
    { DebugView::kNanInfNegative, "nan_inf_negative", kDebugInputFinal, false, true },
    { DebugView::kOverRange, "over_range", kDebugInputFinal, false, true },
    { DebugView::kIndirectIrradiance, "indirect_irradiance", 0, true, false },
    { DebugView::kIndirectRadiance, "indirect_radiance", 0, true, false },
    { DebugView::kAllDiffuse, "all_diffuse", 0, true, false },
    { DebugView::kShadowVisibility, "shadow_visibility", 0, false, false },
    { DebugView::kClusterLoad, "cluster_load", 0, false, false },
    { DebugView::kVolumetricTransmittance, "volumetric_transmittance", 0, false, false },
    { DebugView::kReflectionProbeSelection, "reflection_probe_selection",
        kDebugInputReflectionProbe, false, true },
    { DebugView::kReflectionProbeRadiance, "reflection_probe_radiance", kDebugInputReflectionProbe,
        true, true },
    { DebugView::kReflectionProbeWeight, "reflection_probe_weight", kDebugInputReflectionProbe,
        false, true },
    { DebugView::kReflectionProbeHeader, "reflection_probe_header", kDebugInputReflectionProbe,
        false, true },
    { DebugView::kWorldBatch, "world_batch", 0, false, false },
    { DebugView::kWorldMaterial, "world_material", 0, false, false },
    { DebugView::kLightmapChart, "lightmap_chart", 0, false, false },
    { DebugView::kLightmapTexelDensity, "lightmap_texel_density", 0, false, false },
    { DebugView::kLightmapChartBorders, "lightmap_chart_borders", 0, false, false } };

constexpr std::string_view kTermNames[] = { "clustered", "sun", "area", "projected", "baked",
    "probes", "ibl", "ssr", "ao", "specular_occlusion", "emission", "volumetric",
    "shadow_visibility", "directional", "normal_map", "bounce", "soft_shadows" };
static_assert( std::size( kTermNames ) == std::size_t( std::bit_width( kDebugTermAll ) ) );

std::uint32_t Bits( float value )
{
	return std::bit_cast<std::uint32_t>( value );
}

} // namespace

std::span<const DebugViewInfo> DebugViewCatalog()
{
	return kCatalog;
}

const DebugViewInfo *FindDebugView( std::uint32_t view )
{
	for ( const DebugViewInfo &info : kCatalog )
	{
		if ( static_cast<std::uint32_t>( info.view ) == view )
			return &info;
	}
	return nullptr;
}

std::uint32_t DebugTermBit( std::string_view name )
{
	for ( std::size_t i = 0; i < std::size( kTermNames ); ++i )
	{
		if ( kTermNames[i] == name )
			return 1u << i;
	}
	return 0;
}

std::string_view DebugTermName( std::uint32_t bit )
{
	if ( !std::has_single_bit( bit ) || ( bit & kDebugTermAll ) == 0 )
		return {};
	return kTermNames[std::countr_zero( bit )];
}

void AppendDebugConstants( const DebugSpecialization &debug, device::ShaderStage stage,
    std::vector<device::SpecializationConstant> &constants )
{
	if ( debug.IsNeutral() )
		return;
	constants.push_back( { stage, kDebugConstantView, debug.view } );
	constants.push_back( { stage, kDebugConstantBrdf, debug.brdf } );
	constants.push_back( { stage, kDebugConstantTermsOff, debug.termsOff } );
	constants.push_back( { stage, kDebugConstantFlags, debug.flags } );
	constants.push_back( { stage, kDebugConstantScale, Bits( debug.scale ) } );
	constants.push_back( { stage, kDebugConstantRange, Bits( debug.range ) } );
	constants.push_back( { stage, kDebugConstantThreshold, Bits( debug.threshold ) } );
	constants.push_back( { stage, kDebugConstantForceRoughness, Bits( debug.forceRoughness ) } );
	constants.push_back( { stage, kDebugConstantForceMetalness, Bits( debug.forceMetalness ) } );
}

} // namespace render::shaderlib
