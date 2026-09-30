//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One resolver from a material to its program (RFC 0016 K5); see
//			program_resolver.h. The only code outside the families that names
//			them.
//
//=============================================================================//

#include "render/material/program_resolver.h"

#include "family_program.h"

#include "render/material/lightmapped_family.h"
#include "render/material/pbr_family.h"
#include "render/material/registry.h"
#include "render/material/unlit_family.h"
#include "render/material/vmt_mapping.h"

#include <cctype>
#include <cmath>
#include <cstdlib>

namespace render::material
{

namespace
{

const FamilyRegistry &Registry()
{
	static const FamilyRegistry registry = []
	{
		FamilyRegistry built;
		for ( const FamilyDesc &family : FamiliesFromMapping( BuiltinVmtMapping() ) )
			(void)built.Register( family );
		return built;
	}();
	return registry;
}

bool SameKey( std::string_view a, std::string_view b )
{
	if ( a.size() != b.size() )
		return false;
	for ( std::size_t i = 0; i < a.size(); ++i )
	{
		if ( std::tolower( static_cast<unsigned char>( a[i] ) ) !=
		     std::tolower( static_cast<unsigned char>( b[i] ) ) )
			return false;
	}
	return true;
}

// The numbers of a value ("0.5", "[1 1 1]", "{255 255 255}"), when it is
// only numbers.
std::optional<std::vector<double>> Numbers( const std::string &value )
{
	std::vector<double> numbers;
	const char *at = value.c_str();
	while ( *at )
	{
		if ( std::isspace( static_cast<unsigned char>( *at ) ) || *at == '[' || *at == ']' ||
		     *at == '{' || *at == '}' )
		{
			++at;
			continue;
		}
		char *end = nullptr;
		const double number = std::strtod( at, &end );
		if ( end == at )
			return std::nullopt;
		numbers.push_back( number );
		at = end;
	}
	return numbers;
}

// Whether a variable's value is its declared default: equal numbers, or equal
// text ignoring case. An empty declared default is 0 for a number.
bool AtDefault( const std::string &value, const std::string &declared )
{
	const std::optional<std::vector<double>> a = Numbers( value );
	std::optional<std::vector<double>> b = Numbers( declared );
	if ( a && b && b->empty() && a->size() == 1 )
		b = std::vector<double>{ 0.0 };
	if ( a && b && !a->empty() )
	{
		if ( a->size() != b->size() )
			return false;
		for ( std::size_t i = 0; i < a->size(); ++i )
		{
			if ( std::fabs( ( *a )[i] - ( *b )[i] ) > 1e-4 )
				return false;
		}
		return true;
	}
	return SameKey( value, declared );
}

// No escape hatch: a variable the model does not read keeps the material
// out unless it holds its legacy shader's declared default.
std::optional<std::string> UnreadVariable( const MaterialDesc &material )
{
	for ( const std::string &key : material.unmapped )
	{
		const VmtPair *set = nullptr;
		for ( const VmtPair &variable : material.variables )
		{
			if ( SameKey( variable.key, key ) )
				set = &variable;
		}
		const VmtPair *declared = nullptr;
		for ( const VmtPair &item : material.declaredDefaults )
		{
			if ( SameKey( item.key, key ) )
				declared = &item;
		}
		if ( !set )
			continue;
		if ( !declared )
			return "the model does not read " + key;
		if ( !AtDefault( set->value, declared->value ) )
			return "the model does not read " + key + " " + set->value;
	}
	return std::nullopt;
}

// The material's block, with a stand-in for every texture it binds (a claim
// asks only whether one is bound).
std::optional<ParameterBlock> BlockFor( const MaterialDesc &material, std::string *why )
{
	const FamilySchema *family = Registry().Find( material.family );
	if ( !family )
	{
		*why = "shader " + material.shader + " maps to no family the model draws (" +
		       material.family + ")";
		return std::nullopt;
	}
	if ( std::optional<std::string> unread = UnreadVariable( material ) )
	{
		*why = *unread;
		return std::nullopt;
	}
	ParameterBlock block( *family );
	if ( !ApplyValues( material, block ) )
	{
		*why = "its values do not fit the " + material.family + " schema";
		return std::nullopt;
	}
	for ( const MaterialValue &value : material.values )
	{
		if ( value.kind != ValueKind::kTexture )
			continue;
		// A per-view texture (the view's local env_cubemap, a render target)
		// has no one image for the material.
		if ( value.text == "env_cubemap" || value.text.starts_with( "_rt_" ) ||
		     value.text.starts_with( "[" ) )
		{
			*why = "the model does not bind the per-view texture " + value.text + " (" + value.key +
			       ")";
			return std::nullopt;
		}
		(void)block.SetTexture( value.parameter, device::TextureId( 1 ) );
	}
	return block;
}

std::string TextureOf( const MaterialDesc &material, std::string_view parameter )
{
	for ( const MaterialValue &value : material.values )
	{
		if ( value.kind == ValueKind::kTexture && value.parameter == parameter )
			return value.text;
	}
	return {};
}

} // namespace

struct ProgramResolver::State
{
	explicit State( device::IRenderDevice2 &device ) : device( device ) {}
	device::IRenderDevice2 &device;
	SurfaceVertexLayout layout = SurfaceVertexLayout::kFlat;
	// Owns the surface program; every point the resolver makes is drawn
	// through it.
	std::unique_ptr<LightmappedFamily> lightmapped;
	// World pbr (SetWorldPbr) and the scene terms its points take.
	bool worldPbr = false;
	std::uint32_t sceneTerms = 0;
};

ProgramResolver::ProgramResolver( std::unique_ptr<State> state ) : m_State( std::move( state ) )
{
}
ProgramResolver::~ProgramResolver() = default;

namespace
{
constexpr std::string_view kProgramNames[] = { "lightmapped", "unlit", "preview", "pbr" };
}

std::span<const std::string_view> ProgramResolver::ProgramNames()
{
	return kProgramNames;
}

foundation::Expected<std::unique_ptr<ProgramResolver>, std::string> ProgramResolver::Create(
    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
    std::uint32_t sampleCount, VertexLayout layout, const ProgramModules &modules )
{
	auto state = std::make_unique<State>( device );
	state->layout =
	    layout == VertexLayout::kSurface ? SurfaceVertexLayout::kWorld : SurfaceVertexLayout::kFlat;
	auto lightmapped = LightmappedFamily::Create(
	    device, colorFormat, depthFormat, sampleCount, modules.lightmappedFragment );
	if ( !lightmapped )
		return foundation::MakeUnexpected( std::string( "the surface program was refused" ) );
	state->lightmapped = std::move( lightmapped ).Value();
	return std::unique_ptr<ProgramResolver>( new ProgramResolver( std::move( state ) ) );
}

foundation::Expected<device::BlendMode, std::string> ClaimForDrawing( const MaterialDesc &material )
{
	std::string why;
	const std::optional<ParameterBlock> block = BlockFor( material, &why );
	if ( !block )
		return foundation::MakeUnexpected( why );
	if ( material.family == "lightmapped" )
	{
		const LightmappedClaim claim = ClaimLightmapped( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.blend;
	}
	if ( material.family == "unlit" )
	{
		const UnlitClaim claim = ClaimUnlit( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.blend;
	}
	return foundation::MakeUnexpected( "family " + material.family + " has no program yet" );
}

foundation::Expected<ResolvedProgram, std::string> ProgramResolver::Resolve(
    const MaterialDesc &material )
{
	State &s = *m_State;
	std::string why;
	const std::optional<ParameterBlock> block = BlockFor( material, &why );
	if ( !block )
		return foundation::MakeUnexpected( why );
	ResolvedProgram out;
	if ( material.family == "lightmapped" )
	{
		const LightmappedClaim claim = ClaimLightmapped( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		SurfaceTextures textures;
		textures.base = TextureOf( material, "basetexture" );
		textures.envmap = TextureOf( material, "envmap" );
		textures.envmapMask = TextureOf( material, "envmapmask" );
		textures.bump = TextureOf( material, "bumpmap" );
		textures.detail = TextureOf( material, "detail" );
		auto request = s.lightmapped->Request( claim, textures, s.layout );
		if ( !request )
			return foundation::MakeUnexpected(
			    std::string( request.Error() == LightmappedStatus::kInvalidRequest
			                     ? "its bump or env map term reads the surface vertex, and the "
			                       "resolver's is flat"
			                     : "a lightmapped pipeline was refused" ) );
		out.name = "lightmapped";
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		out.drawInputs = { "lightmap" };
		return out;
	}
	if ( material.family == "unlit" )
	{
		// Unlit is the surface program's unlit point, on the resolver's vertex.
		const UnlitClaim claim = ClaimUnlit( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		SurfaceTextures textures;
		textures.base = TextureOf( material, "basetexture" );
		auto request = s.lightmapped->Program().Request(
		    claim.Variant( s.layout ), claim.constants, textures );
		if ( !request )
			return foundation::MakeUnexpected( std::string( "an unlit pipeline was refused" ) );
		out.name = "unlit";
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		out.drawInputs = { "lightmap" }; // bound, not read: a neutral page serves
		return out;
	}
	if ( material.family == "pbr" )
	{
		// The pbr point on the world vertex (SetWorldPbr): the lightmap basis
		// for its indirect diffuse and the scene's terms.
		if ( !s.worldPbr )
			return foundation::MakeUnexpected( std::string(
			    "world pbr is not enabled: the pbr point draws world surfaces only in "
			    "render_lab (RFC 0016 K11) until K12" ) );
		const PbrClaim claim = ClaimPbr( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( s.layout != SurfaceVertexLayout::kWorld )
			return foundation::MakeUnexpected( std::string(
			    "the pbr point reads the surface vertex, and the resolver's is flat" ) );
		SurfaceVariant variant = claim.Variant();
		variant.layout = SurfaceVertexLayout::kWorld;
		variant.terms |= kSurfaceBakedLightmap | s.sceneTerms;
		SurfaceTextures textures;
		textures.base = TextureOf( material, "basetexture" );
		textures.mrao = TextureOf( material, "mraotexture" );
		if ( claim.normalMap )
			textures.bump = TextureOf( material, "bumpmap" );
		if ( claim.emission )
			textures.emission = TextureOf( material, "emissiontexture" );
		auto request = s.lightmapped->Program().Request( variant, claim.constants, textures );
		if ( !request )
			return foundation::MakeUnexpected( std::string( "a pbr pipeline was refused" ) );
		out.name = "pbr";
		out.request = std::move( request ).Value();
		out.blend = device::BlendMode::kOpaque;
		out.drawInputs = { "lightmap", "lightmap-gradient" };
		return out;
	}
	return foundation::MakeUnexpected( "family " + material.family + " has no program yet" );
}

void ProgramResolver::SetWorldPbr( bool enabled, std::uint32_t sceneTerms )
{
	m_State->worldPbr = enabled;
	m_State->sceneTerms = sceneTerms;
}

SurfaceProgram &ProgramResolver::Program() const
{
	return m_State->lightmapped->Program();
}

foundation::Expected<device::PipelineId, std::string> ProgramResolver::DebugPipeline(
    const ResolvedProgram &program, const shaderlib::DebugSpecialization &debug )
{
	if ( debug.IsNeutral() )
		return program.request.pipeline;
	auto pipeline = m_State->lightmapped->DebugPipeline( program.request.pipeline, debug );
	if ( !pipeline )
		return foundation::MakeUnexpected(
		    std::string( pipeline.Error() == LightmappedStatus::kInvalidRequest
		                     ? "the program " + program.name + " was not made by this resolver"
		                     : "the debug pipeline of " + program.name + " was refused" ) );
	return pipeline.Value();
}

foundation::Expected<ProgramResolver::Preview, std::string> ProgramResolver::ResolvePreview(
    const MaterialDesc &material )
{
	State &s = *m_State;
	// What the preview reads, by VMT key.
	constexpr std::string_view kRead[] = { "$basetexture", "$color", "$alpha", "$translucent",
	    "$additive", "$vertexalpha", "$alphatest", "$alphatestreference", "$vertexcolor" };
	auto find = [&]( std::string_view key ) -> const VmtPair *
	{
		const VmtPair *found = nullptr;
		for ( const VmtPair &variable : material.variables )
		{
			if ( SameKey( variable.key, key ) )
				found = &variable;
		}
		return found;
	};
	auto flag = [&]( std::string_view key )
	{
		const VmtPair *variable = find( key );
		float numbers[4] = {};
		return variable && VmtNumbers( variable->value, numbers ) > 0 && numbers[0] != 0.0f;
	};
	auto scalar = [&]( std::string_view key, float fallback )
	{
		const VmtPair *variable = find( key );
		float numbers[4] = {};
		return variable && VmtNumbers( variable->value, numbers ) > 0 ? numbers[0] : fallback;
	};

	Preview out;
	for ( const VmtPair &variable : material.variables )
	{
		bool read = false;
		for ( std::string_view key : kRead )
			read = read || SameKey( variable.key, key );
		bool metadata = false;
		for ( const VmtPair &item : material.metadata )
			metadata = metadata || SameKey( item.key, variable.key );
		if ( !read && !metadata )
		{
			std::string key = variable.key;
			for ( char &c : key )
				c = char( std::tolower( static_cast<unsigned char>( c ) ) );
			out.ignored.push_back( std::move( key ) );
		}
	}

	// The preview is the unlit point with the editor's (gamma) vertex colors
	// and alpha.
	UnlitClaim claim;
	claim.claimed = true;
	const bool alphaTest = flag( "$alphatest" );
	claim.blend = flag( "$additive" )                                ? device::BlendMode::kAdditive
	              : flag( "$translucent" ) || flag( "$vertexalpha" ) ? device::BlendMode::kAlpha
	                                                                 : device::BlendMode::kOpaque;
	claim.alphaWrite = claim.blend == device::BlendMode::kOpaque && !alphaTest;
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if ( const VmtPair *variable = find( "$color" ) )
	{
		if ( VmtNumbers( variable->value, color ) == 1 )
			color[1] = color[2] = color[0];
	}
	std::copy( color, color + 3, claim.constants.tint );
	claim.constants.tint[3] = scalar( "$alpha", 1.0f );
	claim.constants.flags[0] = 1.0f; // the vertex color
	claim.constants.flags[1] = alphaTest ? 1.0f : 0.0f;
	claim.constants.flags[2] = detail::AlphaTestReference( scalar( "$alphatestreference", 0.0f ) );
	claim.constants.state[1] = 1.0f; // display (gamma) vertex colors, as the editor's are
	claim.constants.state[3] = 1.0f; // and the vertex alpha
	const VmtPair *base = find( "$basetexture" );
	SurfaceTextures textures;
	textures.base = base ? VmtTextureReference( base->value ) : std::string();
	auto request = s.lightmapped->Program().Request(
	    claim.Variant( SurfaceVertexLayout::kFlat ), claim.constants, textures );
	if ( !request )
		return foundation::MakeUnexpected( std::string( "a preview pipeline was refused" ) );
	out.program.name = "preview";
	out.program.request = std::move( request ).Value();
	out.program.blend = claim.blend;
	out.program.drawInputs = { "lightmap" }; // bound, not read: a neutral page serves
	return out;
}

std::optional<GroupRequest> ProgramResolver::DrawGroup(
    const ResolvedProgram &program, const std::vector<std::string> &inputTextures ) const
{
	const State &s = *m_State;
	if ( !program.request.drawLayout.IsValid() ||
	     inputTextures.size() != program.drawInputs.size() )
		return std::nullopt;
	if ( program.name == "pbr" && inputTextures.size() == 2 )
		return s.lightmapped->Program().DrawGroup( inputTextures[0], {}, {}, inputTextures[1] );
	if ( program.request.drawLayout == s.lightmapped->DrawLayout() )
		return s.lightmapped->LightmapGroup( inputTextures[0] );
	return std::nullopt;
}

std::optional<GroupRequest> ProgramResolver::FrameGroup(
    const ResolvedProgram &program, const FrameTerms &terms ) const
{
	const State &s = *m_State;
	if ( !program.request.frameLayout.IsValid() )
		return std::nullopt;
	if ( program.request.frameLayout == s.lightmapped->FrameLayout() )
	{
		SurfaceFrame frame;
		frame.light[0] = terms.lightmapScale;
		frame.light[1] = terms.outputScale;
		frame.light[2] = terms.encodeOutput ? 1.0f : 0.0f;
		std::copy( terms.fogColor, terms.fogColor + 3, frame.fogColor );
		frame.fogColor[3] = terms.fogType;
		std::copy( terms.fogParams, terms.fogParams + 4, frame.fogParams );
		frame.fogMisc[0] = terms.fogEyeZ;
		frame.light[3] = terms.specular ? 1.0f : 0.0f;
		std::copy( terms.eye, terms.eye + 3, frame.eye );
		frame.eye[3] = terms.envmapScale;
		frame.fogMisc[1] = terms.ssbumpNormalized ? 1.0f : 0.0f;
		const std::size_t areas =
		    std::min<std::size_t>( terms.areas.size(), std::size_t( kSurfaceMaxAreaLights ) );
		frame.areaCount[0] = float( areas );
		std::copy( terms.areas.begin(), terms.areas.begin() + std::ptrdiff_t( areas ), frame.areas );
		return s.lightmapped->Program().FrameGroup(
		    frame, terms.splitSumTable, terms.ltcTable, terms.map );
	}
	return std::nullopt;
}

} // namespace render::material
