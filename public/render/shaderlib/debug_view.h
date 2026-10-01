//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The debug view catalog and its specialization constants (RFC 0014
//			render.debug-views.v1, on the RFC 0016 core). One owner of the
//			view numbers, the lighting-model term bits and the constant ids
//			every core program reads through
//			render/shaders/common/debug_view.glsl.
//
//			Every debug parameter is a specialization constant (clause D20)
//			of the program's fragment stage, so a debug pipeline has the
//			shipped pipeline's module and layouts. A program whose
//			specialization is neutral gets no debug constant at all: its
//			pipeline is the shipped one. A debug pipeline is made on first use
//			for its exact values (a debug tool; the program's owner caches it).
//
//			The constant ids (100-108) are reserved in every core program; a
//			program declares them only by including debug_view.glsl.
//
//=============================================================================//

#ifndef RENDER_SHADERLIB_DEBUG_VIEW_H
#define RENDER_SHADERLIB_DEBUG_VIEW_H

#include "render/device/pipeline.h"

#include <compare>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace render::shaderlib
{

// The views of cl_render_debug_view (RFC 0014 "Catalog"); 0 is off.
enum class DebugView : std::uint32_t
{
	kOff = 0,
	kAlbedo = 1,
	kWorldNormal = 2,
	kNormalMap = 3,
	kRoughness = 4,
	kFilteredRoughness = 5,
	kMetalness = 6,
	kAmbientOcclusion = 7,
	kBakedLight = 8,
	kDirectLight = 9,
	kImageSpecular = 10,
	kScreenSpaceReflections = 11,
	kEmissive = 12,
	kUvChecker = 13,
	kVertexColor = 14,
	kLinearDepth = 15,
	kNanInfNegative = 16,
	kOverRange = 17,
	kIndirectIrradiance = 18,
	kIndirectRadiance = 19,
	kAllDiffuse = 20,
	kShadowVisibility = 21,
	kClusterLoad = 22,
	kVolumetricTransmittance = 23,
	kReflectionProbeSelection = 24,
	kReflectionProbeRadiance = 25,
	kReflectionProbeWeight = 26,
	kReflectionProbeHeader = 27,
	kWorldBatch = 32,
	kWorldMaterial = 33,
	kLightmapChart = 34,
	kLightmapTexelDensity = 35,
	kLightmapChartBorders = 36
};

// What a program can report (debug_view.glsl's kDebugHas* bits, which must
// agree). A view whose inputs a program lacks draws the not-applicable hatch.
enum DebugInput : std::uint32_t
{
	kDebugInputAlbedo = 1u << 0,
	kDebugInputNormal = 1u << 1,
	kDebugInputNormalMap = 1u << 2,
	kDebugInputRoughness = 1u << 3,
	kDebugInputFilteredRoughness = 1u << 4,
	kDebugInputMetalness = 1u << 5,
	kDebugInputAo = 1u << 6,
	kDebugInputBaked = 1u << 7,
	kDebugInputDirect = 1u << 8,
	kDebugInputImageSpecular = 1u << 9,
	kDebugInputSsr = 1u << 10,
	kDebugInputEmission = 1u << 11,
	kDebugInputUv0 = 1u << 12,
	kDebugInputVertexColor = 1u << 13,
	kDebugInputFinal = 1u << 14, // the color before tone mapping; every program has it
	kDebugInputDepth = 1u << 15, // the fragment's view depth; every program has it
	kDebugInputReflectionProbe = 1u << 16
};

struct DebugViewInfo
{
	DebugView view;
	std::string_view name;    // the catalog's name, lower case
	std::uint32_t inputs = 0; // the DebugInput bits the view reads
	bool radiometric = false; // scaled by cl_render_debug_view_scale
	bool installed = false;   // the core draws it today (others are reserved for their terms)
};

// Every catalog entry, in view order.
std::span<const DebugViewInfo> DebugViewCatalog();
// The entry for a view number; nullptr when the number is reserved or unknown.
const DebugViewInfo *FindDebugView( std::uint32_t view );

// The lighting-model terms cl_render_debug_term names (RFC 0014 "Lighting-
// model controls"), as bits of kDebugTermsOff.
enum DebugTerm : std::uint32_t
{
	kDebugTermClustered = 1u << 0,
	kDebugTermSun = 1u << 1,
	kDebugTermArea = 1u << 2,
	kDebugTermProjected = 1u << 3,
	kDebugTermBaked = 1u << 4,
	kDebugTermProbes = 1u << 5,
	kDebugTermIbl = 1u << 6,
	kDebugTermSsr = 1u << 7,
	kDebugTermAo = 1u << 8,
	kDebugTermSpecularOcclusion = 1u << 9,
	kDebugTermEmission = 1u << 10,
	kDebugTermVolumetric = 1u << 11
};
inline constexpr std::uint32_t kDebugTermAll = ( 1u << 12 ) - 1;

// The term bit for a name ("clustered", ...); 0 when unknown.
std::uint32_t DebugTermBit( std::string_view name );
// The name of a single term bit; empty when not one.
std::string_view DebugTermName( std::uint32_t bit );

// cl_render_debug_brdf.
enum class DebugBrdf : std::uint32_t
{
	kFull = 0,
	kDiffuseOnly = 1,
	kSpecularOnly = 2,
	kNoEnergyCompensation = 3,
	kSplitSumSample = 4
};
inline constexpr std::uint32_t kDebugBrdfModes = 5;

// kDebugFlags bits.
inline constexpr std::uint32_t kDebugFlagFurnace = 1u << 0;
// The program is outside cl_render_debug_view_program's filter: flat 18% grey.
inline constexpr std::uint32_t kDebugFlagFilteredOut = 1u << 1;

// The specialization constant ids (debug_view.glsl declares the same).
inline constexpr std::uint32_t kDebugConstantView = 100;
inline constexpr std::uint32_t kDebugConstantBrdf = 101;
inline constexpr std::uint32_t kDebugConstantTermsOff = 102;
inline constexpr std::uint32_t kDebugConstantFlags = 103;
inline constexpr std::uint32_t kDebugConstantScale = 104;
inline constexpr std::uint32_t kDebugConstantRange = 105;
inline constexpr std::uint32_t kDebugConstantThreshold = 106;
inline constexpr std::uint32_t kDebugConstantForceRoughness = 107;
inline constexpr std::uint32_t kDebugConstantForceMetalness = 108;

// One program's debug specialization. The defaults are neutral: the shipped
// program. Floats compare by value; a debug pipeline is keyed by all fields.
struct DebugSpecialization
{
	std::uint32_t view = 0;
	std::uint32_t brdf = 0;
	std::uint32_t termsOff = 0;
	std::uint32_t flags = 0;
	float scale = 1.0f;
	float range = 4096.0f;
	float threshold = 1.0f;
	float forceRoughness = -1.0f; // below 0: off
	float forceMetalness = -1.0f;

	bool IsNeutral() const { return *this == DebugSpecialization(); }
	auto operator<=>( const DebugSpecialization & ) const = default;
	bool operator==( const DebugSpecialization & ) const = default;
};

// The constants of a debug specialization for a stage, appended to a
// program's own; none when the specialization is neutral.
void AppendDebugConstants( const DebugSpecialization &debug, device::ShaderStage stage,
    std::vector<device::SpecializationConstant> &constants );

} // namespace render::shaderlib

#endif // RENDER_SHADERLIB_DEBUG_VIEW_H
