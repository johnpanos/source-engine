//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frame's debug controls (RFC 0014); see
//			render/frame/debug_controls.h.
//
//=============================================================================//

#include "render/frame/debug_controls.h"

#include "render/frame/debug_specialization.h"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace render::frame
{

namespace
{

[[gnu::format( printf, 2, 3 )]] foundation::Unexpected<DebugControlsError> Reject(
    DebugControlsStatus status, const char *format, ... )
{
	DebugControlsError error;
	error.status = status;
	std::va_list values;
	va_start( values, format );
	std::vsnprintf( error.message, sizeof( error.message ), format, values );
	va_end( values );
	return foundation::MakeUnexpected( error );
}

std::string_view ProgramName( const DebugControls &controls )
{
	return std::string_view(
	    controls.program, strnlen( controls.program, sizeof( controls.program ) ) );
}

bool PositiveFinite( float value )
{
	return std::isfinite( value ) && value > 0.0f;
}

bool OverrideValid( float value )
{
	return std::isfinite( value ) && ( value < 0.0f ? value == -1.0f : value <= 1.0f );
}

} // namespace

foundation::Expected<void, DebugControlsError> ValidateDebugControls(
    const DebugControls &controls, std::span<const std::string_view> programs )
{
	const shaderlib::DebugViewInfo *view = shaderlib::FindDebugView( controls.view );
	if ( !view )
		return Reject( DebugControlsStatus::kUnknownView,
		    "cl_render_debug_view %u: not a view (0-23, 32-36; 24-31 and 37 on are reserved)",
		    controls.view );
	if ( !view->installed )
		return Reject( DebugControlsStatus::kReservedView,
		    "cl_render_debug_view %u (%.*s): its terms are not on the core yet", controls.view,
		    int( view->name.size() ), view->name.data() );
	if ( strnlen( controls.program, sizeof( controls.program ) ) == sizeof( controls.program ) )
		return Reject( DebugControlsStatus::kUnterminatedProgram,
		    "cl_render_debug_view_program: the name is longer than %zu characters",
		    sizeof( controls.program ) - 1 );
	const std::string_view program = ProgramName( controls );
	if ( !program.empty() )
	{
		bool known = false;
		for ( std::string_view name : programs )
			known = known || name == program;
		if ( !known )
			return Reject( DebugControlsStatus::kUnknownProgram,
			    "cl_render_debug_view_program %.*s: not a core program (? lists them)",
			    int( program.size() ), program.data() );
	}
	if ( controls.brdf >= shaderlib::kDebugBrdfModes )
		return Reject( DebugControlsStatus::kInvalidBrdf, "cl_render_debug_brdf %u: modes are 0-%u",
		    controls.brdf, shaderlib::kDebugBrdfModes - 1 );
	if ( controls.termsOff & ~shaderlib::kDebugTermAll )
		return Reject( DebugControlsStatus::kInvalidTerms,
		    "cl_render_debug_term: bits 0x%x name no term",
		    controls.termsOff & ~shaderlib::kDebugTermAll );
	if ( !PositiveFinite( controls.viewScale ) || !PositiveFinite( controls.viewRange ) ||
	     !PositiveFinite( controls.viewThreshold ) )
		return Reject( DebugControlsStatus::kInvalidNumber,
		    "cl_render_debug_view_scale, _range and _threshold must be finite and above 0" );
	if ( !OverrideValid( controls.forceRoughness ) || !OverrideValid( controls.forceMetalness ) )
		return Reject( DebugControlsStatus::kInvalidOverride,
		    "cl_render_debug_force_roughness and _metalness take -1 (off) or 0 to 1" );
	if ( static_cast<std::uint32_t>( controls.legacy ) > 2 )
		return Reject( DebugControlsStatus::kInvalidLegacy,
		    "cl_render_debug_legacy %u: modes are 0-2",
		    static_cast<std::uint32_t>( controls.legacy ) );
	return {};
}

bool ParseDebugTerms(
    const char *names, std::uint32_t *bits, char *unknown, std::size_t unknownBytes )
{
	*bits = 0;
	if ( unknown && unknownBytes )
		unknown[0] = '\0';
	std::string_view rest = names ? std::string_view( names ) : std::string_view();
	while ( !rest.empty() )
	{
		const std::size_t comma = rest.find( ',' );
		std::string_view name = rest.substr( 0, comma );
		rest = comma == std::string_view::npos ? std::string_view() : rest.substr( comma + 1 );
		while ( !name.empty() && name.front() == ' ' )
			name.remove_prefix( 1 );
		while ( !name.empty() && name.back() == ' ' )
			name.remove_suffix( 1 );
		if ( name.empty() )
			continue;
		const std::uint32_t bit = shaderlib::DebugTermBit( name );
		if ( bit == 0 )
		{
			if ( unknown && unknownBytes )
				std::snprintf( unknown, unknownBytes, "%.*s", int( name.size() ), name.data() );
			return false;
		}
		*bits |= bit;
	}
	return true;
}

bool PixelViewActive( const DebugControls &controls )
{
	return controls.view != 0;
}

bool DebugControlsNeutral( const DebugControls &controls )
{
	const DebugControls neutral;
	return controls.costOverlay == neutral.costOverlay && controls.view == neutral.view && ProgramName( controls ).empty() &&
	       controls.viewScale == neutral.viewScale && controls.viewRange == neutral.viewRange &&
	       controls.viewThreshold == neutral.viewThreshold && controls.brdf == neutral.brdf &&
	       controls.furnace == neutral.furnace && controls.termsOff == neutral.termsOff &&
	       controls.forceRoughness == neutral.forceRoughness &&
	       controls.forceMetalness == neutral.forceMetalness && controls.legacy == neutral.legacy;
}

shaderlib::DebugSpecialization DebugSpecializationFor(
    const DebugControls &controls, std::string_view program )
{
	shaderlib::DebugSpecialization debug;
	debug.termsOff = controls.termsOff;
	debug.flags = controls.furnace ? shaderlib::kDebugFlagFurnace : 0u;
	const std::string_view filter = ProgramName( controls );
	const bool selected = filter.empty() || filter == program;
	if ( !selected )
	{
		// Outside the filter: flat grey under a pixel view; otherwise the
		// program keeps its shading (the BRDF mode and overrides are the
		// filtered program's alone).
		if ( PixelViewActive( controls ) )
			debug.flags |= shaderlib::kDebugFlagFilteredOut;
		return debug;
	}
	debug.view = controls.view;
	debug.brdf = controls.brdf;
	debug.forceRoughness = controls.forceRoughness;
	debug.forceMetalness = controls.forceMetalness;
	// The view's parameters matter only to the views that read them; keeping
	// the others at their defaults keeps one pipeline per view.
	const shaderlib::DebugViewInfo *info = shaderlib::FindDebugView( controls.view );
	if ( info && info->radiometric )
		debug.scale = controls.viewScale;
	if ( controls.view == static_cast<std::uint32_t>( shaderlib::DebugView::kLinearDepth ) )
		debug.range = controls.viewRange;
	if ( controls.view == static_cast<std::uint32_t>( shaderlib::DebugView::kOverRange ) )
		debug.threshold = controls.viewThreshold;
	return debug;
}

} // namespace render::frame
