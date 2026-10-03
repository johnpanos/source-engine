//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frame's debug controls (RFC 0014 "One owner of debug state"):
//			one validated value in the FrameDesc policy. The engine's render
//			core host parses the cl_render_debug_* ConVars once per frame
//			into it; the renderer validates it when the frame begins and
//			applies it to the whole frame, keeping the previous value when a
//			new one is invalid. No shader, pass or engine module reads a
//			debug ConVar directly: passes read FrameDesc::debug and ask
//			DebugSpecializationFor (debug_specialization.h) for their
//			programs' specialization.
//
//			Engine-facing: no dual-ABI library type (std::string, std::list)
//			appears here or in what it includes.
//
//=============================================================================//

#ifndef RENDER_FRAME_DEBUG_CONTROLS_H
#define RENDER_FRAME_DEBUG_CONTROLS_H

#include "foundation/expected.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace render::frame
{

inline constexpr std::size_t kDebugProgramNameBytes = 32;

// cl_render_debug_legacy.
enum class DebugLegacy : std::uint32_t
{
	kOff = 0,
	kTint = 1, // legacy stream passes' written pixels tinted magenta
	kSkip = 2  // legacy stream passes not recorded; their pixels show the hatch
};

struct DebugControls
{
	bool costOverlay = false; // cl_render_debug_cost: measured core recording/pass costs
	std::uint32_t view = 0; // cl_render_debug_view (shaderlib::DebugView)
	// cl_render_debug_view_program: the one program the view, the BRDF mode
	// and the overrides apply to (others draw flat 18% grey); empty for all.
	char program[kDebugProgramNameBytes] = {};
	float viewScale = 1.0f;       // cl_render_debug_view_scale
	float viewRange = 4096.0f;    // cl_render_debug_view_range
	float viewThreshold = 1.0f;   // cl_render_debug_view_threshold
	std::uint32_t brdf = 0;       // cl_render_debug_brdf (shaderlib::DebugBrdf)
	bool furnace = false;         // cl_render_debug_furnace
	std::uint32_t termsOff = 0;   // cl_render_debug_term (shaderlib::DebugTerm bits)
	float forceRoughness = -1.0f; // cl_render_debug_force_roughness; below 0 off
	float forceMetalness = -1.0f; // cl_render_debug_force_metalness; below 0 off
	DebugLegacy legacy = DebugLegacy::kOff;
};

enum class DebugControlsStatus : std::uint8_t
{
	kUnknownView = 1, // not a catalog number
	kReservedView,    // a catalog number whose terms the core does not draw yet
	kUnknownProgram,  // not one of the core's programs
	kUnterminatedProgram,
	kInvalidBrdf,
	kInvalidTerms,    // bits outside shaderlib::kDebugTermAll
	kInvalidNumber,   // a scale, range or threshold not finite and above zero
	kInvalidOverride, // a forced roughness or metalness not in [0, 1] or below 0
	kInvalidLegacy
};

struct DebugControlsError
{
	DebugControlsStatus status = DebugControlsStatus::kUnknownView;
	char message[160] = {}; // why, for the console
};

// Whether the controls are valid for a core whose programs are named
// `programs`.
foundation::Expected<void, DebugControlsError> ValidateDebugControls(
    const DebugControls &controls, std::span<const std::string_view> programs );

// Whether a pixel view is active: legacy stream passes are then not
// recorded (their targets show the hatch) and post-processing is bypassed.
bool PixelViewActive( const DebugControls &controls );

// Whether the controls change nothing (every control at its default).
bool DebugControlsNeutral( const DebugControls &controls );

// Comma-separated term names (cl_render_debug_term: "ao,ibl"; spaces are
// ignored, empty is no term) as shaderlib::DebugTerm bits. False, with the
// first unknown name in `unknown`, when a name is not a term.
bool ParseDebugTerms(
    const char *names, std::uint32_t *bits, char *unknown, std::size_t unknownBytes );

} // namespace render::frame

#endif // RENDER_FRAME_DEBUG_CONTROLS_H
