//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One resolver from a material to the program that draws it (RFC
//			0016 K5, "One model; legacy cases are degenerate").
//
//			Passes (world, props, models) never name a family. They map a
//			material's variables (MapVariables), hand the result here, and
//			get back a ProgramRequest: the pipeline for the pass's target
//			formats, the material group's constants and textures (by the
//			importer's normalized names, which the caller resolves to
//			textures), the vertex stride and draw constants the program
//			reads, and the per-draw inputs its draw group takes by name
//			("lightmap": the draw's lightmap page).
//
//			This is the one place that knows the families. Today it
//			dispatches to the narrow ones (lightmapped, unlit); as they fold
//			into the general surface model the dispatch shrinks, and callers
//			do not change. A material it cannot draw is refused with the
//			reason, which names the gap in the model.
//
//			Render sequence: pipelines are made on first use.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_PROGRAM_RESOLVER_H
#define RENDER_MATERIAL_PROGRAM_RESOLVER_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/material_programs.h"
#include "render/material/vmt_import.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace render::material
{

// The frame's terms every program reads a subset of (one owner): how the
// lightmap pages encode light and the output's linear (tone-mapping) scale.
struct FrameTerms
{
	float lightmapScale = 4.5947938f; // LDR gamma pages at half overbright (2^2.2)
	float outputScale = 1.0f;
	// The target has no sRGB view: the shader encodes its output (the
	// resolver's color format is then the target's unorm format).
	bool encodeOutput = false;
	// The view's fog (legacy::CorePassFog's terms): type -1 none, 0 range,
	// 1 height; color linear and tone-scaled; parameters; the eye's world z.
	float fogType = -1.0f;
	float fogColor[3] = { 0.0f, 0.0f, 0.0f };
	float fogParams[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float fogEyeZ = 0.0f;
};

struct ResolvedProgram
{
	ProgramRequest request;
	device::BlendMode blend = device::BlendMode::kOpaque;
	// The per-draw inputs the draw group takes, in binding order.
	std::vector<std::string> drawInputs;
};

class ProgramResolver
{
public:
	// Programs for targets of these formats (color as the pass writes it) and
	// sample count.
	static foundation::Expected<std::unique_ptr<ProgramResolver>, std::string> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
	    std::uint32_t sampleCount = 1 );
	~ProgramResolver();
	ProgramResolver( const ProgramResolver & ) = delete;
	ProgramResolver &operator=( const ProgramResolver & ) = delete;

	// The program for a mapped material, or why it has none.
	foundation::Expected<ResolvedProgram, std::string> Resolve( const MaterialDesc &material );
	// The draw group a resolved program reads, with its inputs' textures by
	// name (in drawInputs order); nullopt when the program reads none.
	std::optional<GroupRequest> DrawGroup(
	    const ResolvedProgram &program, const std::vector<std::string> &inputTextures ) const;
	// The frame group a resolved program reads, for these terms; nullopt when
	// it reads none.
	std::optional<GroupRequest> FrameGroup(
	    const ResolvedProgram &program, const FrameTerms &terms ) const;

	// The editor's preview of any material (Hammer's textured view). By
	// contract an approximation, never a claim, and never used by the game's
	// passes: the base texture times $color and $alpha, with the material's
	// blend (translucent, additive), alpha test and reference, the vertex
	// color on and the lighting fixed at one. Every variable it does not
	// read is named in `ignored`, so nothing is silent.
	struct Preview
	{
		ResolvedProgram program;
		std::vector<std::string> ignored; // lower case, as the VMT names them
	};
	foundation::Expected<Preview, std::string> ResolvePreview( const MaterialDesc &material );

private:
	struct State;
	explicit ProgramResolver( std::unique_ptr<State> state );
	std::unique_ptr<State> m_State;
};

// Whether a material's variables claim no more than the model draws, without
// a device (the pass's main-thread decision to take its surfaces). The same
// rules as Resolve, minus pipelines.
foundation::Expected<device::BlendMode, std::string> ClaimForDrawing(
    const MaterialDesc &material );

} // namespace render::material

#endif // RENDER_MATERIAL_PROGRAM_RESOLVER_H
