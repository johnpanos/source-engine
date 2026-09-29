//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One resolver from a material to its program (RFC 0016 K5); see
//			program_resolver.h. The only code outside the families that names
//			them.
//
//=============================================================================//

#include "render/material/program_resolver.h"

#include "render/material/lightmapped_family.h"
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
		if ( value.kind == ValueKind::kTexture )
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
	std::unique_ptr<LightmappedFamily> lightmapped;
	std::unique_ptr<UnlitFamily> unlit;
};

ProgramResolver::ProgramResolver( std::unique_ptr<State> state ) : m_State( std::move( state ) )
{
}
ProgramResolver::~ProgramResolver() = default;

foundation::Expected<std::unique_ptr<ProgramResolver>, std::string> ProgramResolver::Create(
    device::IRenderDevice2 &device, device::Format colorFormat, device::Format depthFormat,
    std::uint32_t sampleCount )
{
	auto state = std::make_unique<State>( device );
	auto lightmapped = LightmappedFamily::Create( device, colorFormat, depthFormat, sampleCount );
	auto unlit = UnlitFamily::Create( device, colorFormat, depthFormat, sampleCount );
	if ( !lightmapped || !unlit )
		return foundation::MakeUnexpected( std::string( "a family's layouts were refused" ) );
	state->lightmapped = std::move( lightmapped ).Value();
	state->unlit = std::move( unlit ).Value();
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
		auto request = s.lightmapped->Request( claim, TextureOf( material, "basetexture" ) );
		if ( !request )
			return foundation::MakeUnexpected(
			    std::string( "a lightmapped pipeline was refused" ) );
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		out.drawInputs = { "lightmap" };
		return out;
	}
	if ( material.family == "unlit" )
	{
		// Unlit is the lightmapped term with the lighting fixed at one: the
		// same program and vertex, the unlit claim's constants.
		const UnlitClaim unlit = ClaimUnlit( *block );
		if ( !unlit.claimed )
			return foundation::MakeUnexpected( unlit.reason );
		LightmappedClaim claim;
		claim.claimed = true;
		claim.blend = unlit.blend;
		claim.alphaWrite = unlit.alphaWrite;
		std::copy( unlit.constants.color, unlit.constants.color + 4, claim.constants.tint );
		claim.constants.flags[0] = unlit.constants.flags[0]; // $vertexcolor
		claim.constants.flags[1] = unlit.constants.flags[2]; // $alphatest
		claim.constants.flags[2] = unlit.constants.flags[3]; // its reference
		claim.constants.flags[3] = 1.0f;                     // lighting is one
		auto request = s.lightmapped->Request( claim, TextureOf( material, "basetexture" ) );
		if ( !request )
			return foundation::MakeUnexpected(
			    std::string( "a lightmapped pipeline was refused" ) );
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		out.drawInputs = { "lightmap" }; // bound, not read: a neutral page serves
		return out;
	}
	return foundation::MakeUnexpected( "family " + material.family + " has no program yet" );
}

std::optional<GroupRequest> ProgramResolver::DrawGroup(
    const ResolvedProgram &program, const std::vector<std::string> &inputTextures ) const
{
	const State &s = *m_State;
	if ( !program.request.drawLayout.IsValid() ||
	     inputTextures.size() != program.drawInputs.size() )
		return std::nullopt;
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
		LightmappedFrame frame;
		frame.light[0] = terms.lightmapScale;
		frame.light[1] = terms.outputScale;
		frame.light[2] = terms.encodeOutput ? 1.0f : 0.0f;
		return s.lightmapped->FrameGroup( frame );
	}
	return std::nullopt;
}

} // namespace render::material
