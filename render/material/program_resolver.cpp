//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: One resolver from a material to its program (RFC 0016 K5); see
//			program_resolver.h. The only code outside the families that names
//			them.
//
//=============================================================================//

#include "render/material/program_resolver.h"

#include "family_program.h"

#include "render/material/cable_family.h"
#include "render/material/lightmapped_family.h"
#include "render/material/pbr_family.h"
#include "render/material/refract_family.h"
#include "render/material/registry.h"
#include "render/material/unlit_family.h"
#include "render/material/vertexlit_family.h"
#include "render/material/vmt_mapping.h"
#include "render/material/water_family.h"

#include <algorithm>
#include <cstring>
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

bool IsSprite( const MaterialDesc &material )
{
	return material.legacyShader == "sprite_dx9";
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

// A legacy colour default is written in the shader's own 0-255 form
// ("{255 255 255}") while the material system reports the same colour in unit
// scale ("[ 1.000000 1.000000 1.000000 ]"), so both sides are brought to unit
// scale before they are compared. A three- or four-number value with a
// component above 1 is the 0-255 form; a scalar or a pair is never a colour.
std::vector<double> UnitScale( std::vector<double> numbers )
{
	if ( numbers.size() < 3 )
		return numbers;
	for ( const double number : numbers )
	{
		if ( number <= 1.0 )
			continue;
		for ( double &component : numbers )
			component /= 255.0;
		break;
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
		const std::vector<double> left = UnitScale( *a );
		const std::vector<double> right = UnitScale( *b );
		if ( left.size() != right.size() )
			return false;
		for ( std::size_t i = 0; i < left.size(); ++i )
		{
			if ( std::fabs( left[i] - right[i] ) > 1e-4 )
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
	const auto enabled = [&]( std::string_view key )
	{
		for ( const VmtPair &variable : material.variables )
		{
			if ( SameKey( variable.key, key ) )
			{
				const auto value = Numbers( variable.value );
				return !value || value->size() != 1 || value->front() != 0.0;
			}
		}
		return false;
	};
	bool cloakEnabled = false;
	for ( const VmtPair &variable : material.variables )
	{
		if ( SameKey( variable.key, "$cloakpassenabled" ) )
		{
			const auto value = Numbers( variable.value );
			cloakEnabled = !value || value->size() != 1 || value->front() != 0.0;
		}
	}
	for ( const std::string &key : material.unmapped )
	{
		// LightmappedGeneric reads these values only inside the corresponding
		// distance-field feature. InitParams and declared defaults can differ.
		if ( material.family == "lightmapped" &&
		     ( ( !enabled( "$softedges" ) && ( SameKey( key, "$edgesoftnessstart" ) ||
		                                         SameKey( key, "$edgesoftnessend" ) ) ) ||
		         ( !enabled( "$outline" ) &&
		             ( SameKey( key, "$outlinecolor" ) || SameKey( key, "$outlinealpha" ) ||
		                 SameKey( key, "$outlinestart0" ) || SameKey( key, "$outlinestart1" ) ||
		                 SameKey( key, "$outlineend0" ) || SameKey( key, "$outlineend1" ) ) ) ) )
			continue;
		// VertexLitGeneric only reads these controls inside its enabled cloak
		// pass. A dormant factor of 1 on the cube does not request transmission.
		if ( !cloakEnabled && material.family == "vertexlit" &&
		     ( SameKey( key, "$cloakpassenabled" ) || SameKey( key, "$cloakfactor" ) ||
		         SameKey( key, "$cloakcolortint" ) || SameKey( key, "$cloaktint" ) ||
		         SameKey( key, "$refractamount" ) ) )
			continue;
		// VertexLitGeneric uploads $seamless_scale only when seamless mapping is
		// on for the base or the detail texture (vertexlitgeneric_dx9_helper.cpp's
		// "if ( bSeamlessDetail || bSeamlessBase )"), and its declared default is
		// the 1.0 an unenabled pass never uses. A scale with neither enabled is
		// inert, so its value cannot change a pixel.
		if ( material.family == "vertexlit" && SameKey( key, "$seamless_scale" ) &&
		     !enabled( "$seamless_base" ) && !enabled( "$seamless_detail" ) )
			continue;
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

// A render target a family's parameter may name: the image a view drew
// earlier in the frame, which the pass imports per frame (the world pass
// re-imports it when the backend replaces it). Every other per-view texture
// keeps the material out.
bool ViewRenderTarget( const MaterialDesc &material, const MaterialValue &value )
{
	return material.family == "water" && value.parameter == "reflecttexture" &&
	       SameKey( value.text, "_rt_WaterReflection" );
}

// The material's block, with a stand-in for every texture it binds (a claim
// asks only whether one is bound).
std::optional<ParameterBlock> BlockFor(
    const MaterialDesc &material, std::string *why, bool nativeReflectionProbes = false )
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
	// VertexLitGeneric::SHADER_INIT_PARAMS clears an authored envmap mask
	// when a bump map is defined. Without the normal-alpha mask flag it also
	// clears the envmap (likewise for base-alpha masking with a bump map).
	// Apply the same precedence before probe availability and mesh claims.
	bool bumpedVertexLit = false;
	bool authoredEnvmapMask = false;
	if ( material.family == "vertexlit" )
	{
		for ( const MaterialValue &value : material.values )
		{
			bumpedVertexLit |= value.kind == ValueKind::kTexture && value.parameter == "bumpmap" &&
			                   !value.text.empty();
			authoredEnvmapMask |= value.kind == ValueKind::kTexture &&
			                      value.parameter == "envmapmask" && !value.text.empty();
		}
	}
	const bool suppressEnvmap =
	    bumpedVertexLit && !detail::ReadFlag( block, "normalmapalphaenvmapmask" ) &&
	    ( authoredEnvmapMask || detail::ReadFlag( block, "basealphaenvmapmask" ) );
	for ( const MaterialValue &value : material.values )
	{
		if ( value.kind != ValueKind::kTexture )
			continue;
		if ( bumpedVertexLit && value.parameter == "envmapmask" )
			continue;
		if ( suppressEnvmap && value.parameter == "envmap" )
			continue;
		// Water reads its env map only as a forced reflection; the water
		// claim's callers require the probes then (WaterEnvNeedsProbes).
		if ( material.family == "water" && value.parameter == "envmap" &&
		     value.text == "env_cubemap" )
		{
			(void)block.SetTexture( value.parameter, device::TextureId( 1 ) );
			continue;
		}
		if ( ( material.family == "vertexlit" || material.family == "unlit" ||
		         ( ( material.family == "refract" || material.family == "lightmapped" ) &&
		             value.text == "env_cubemap" ) ) &&
		     value.parameter == "envmap" )
		{
			if ( !nativeReflectionProbes )
			{
				*why = "$envmap needs the stage's native reflection probes";
				return std::nullopt;
			}
			(void)block.SetTexture( value.parameter, device::TextureId( 1 ) );
			continue; // the program binds RPRB, never the legacy cubemap handle
		}
		// A per-view texture (the view's local env_cubemap, a render target)
		// has no one image for the material.
		if ( ( value.text == "env_cubemap" || value.text.starts_with( "_rt_" ) ||
		         value.text.starts_with( "[" ) ) &&
		     !ViewRenderTarget( material, value ) )
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
	bool mesh = false; // ResolveMesh's call of Resolve
	bool sceneColorAvailable = false;
};

ProgramResolver::ProgramResolver( std::unique_ptr<State> state ) : m_State( std::move( state ) )
{
}
ProgramResolver::~ProgramResolver() = default;

namespace
{
std::optional<std::string> DepthClaim( const ParameterBlock &block, bool portal )
{
	constexpr std::string_view depthKeys[] = { "model", "nocull", "nofog" };
	// PortalRefract marks all stages translucent for material-system ordering.
	// Stage 1 only writes the aperture's depth/stencil; it never blends color.
	constexpr std::string_view portalKeys[] = { "model", "nocull", "nofog", "translucent", "stage",
	    "portalopenamount", "portalstatic", "portalmasktexture", "portalcolortexture",
	    "portalcolorscale", "texturetransform", "time", "alphatest", "alphatestreference" };
	if ( portal && detail::ReadParameter( block, "stage" ) != 1.0f )
		return "portal aperture only: refractive and color stages are separate cohorts";
	return detail::UnclaimedParameter( block, portal ? std::span<const std::string_view>( portalKeys )
	                                               : std::span<const std::string_view>( depthKeys ) );
}

constexpr std::string_view kProgramNames[] = {
    "lightmapped", "unlit", "preview", "pbr", "refract", "water", "depth" };
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
	switch ( layout )
	{
	case VertexLayout::kFlat:
		state->layout = SurfaceVertexLayout::kFlat;
		break;
	case VertexLayout::kSurface:
		state->layout = SurfaceVertexLayout::kWorld;
		break;
	case VertexLayout::kModel:
		state->layout = SurfaceVertexLayout::kModel;
		break;
	}
	auto lightmapped = LightmappedFamily::Create( device, colorFormat, depthFormat, sampleCount,
	    modules.lightmappedFragment, modules.shadowFragment );
	if ( !lightmapped )
		return foundation::MakeUnexpected( std::string( "the surface program was refused" ) );
	state->lightmapped = std::move( lightmapped ).Value();
	return std::unique_ptr<ProgramResolver>( new ProgramResolver( std::move( state ) ) );
}

foundation::Expected<device::BlendMode, std::string> ClaimForDrawing( const MaterialDesc &material,
    bool worldPbr, bool *requiresDepthAlpha, bool nativeReflectionProbes )
{
	if ( requiresDepthAlpha )
		*requiresDepthAlpha = false;
	std::string why;
	const std::optional<ParameterBlock> block =
	    BlockFor( material, &why, material.family == "lightmapped" && nativeReflectionProbes );
	if ( !block )
		return foundation::MakeUnexpected( why );
	if ( material.family == "depth" || material.family == "portal-mask" )
		return ClaimForMesh( material, false, false );
	if ( material.family == "lightmapped" )
	{
		const LightmappedClaim claim = ClaimLightmapped( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.blend;
	}
	if ( material.family == "unlit" || material.family == "cable" ||
	     material.family == "decal-modulate" )
	{
		const UnlitClaim claim = material.family == "cable" ? ClaimCable( *block )
		                         : material.family == "decal-modulate"
		                             ? ClaimDecalModulate( *block )
		                         : IsSprite( material ) ? ClaimSprite( *block )
		                                                : ClaimUnlit( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( requiresDepthAlpha )
			*requiresDepthAlpha = claim.depthBlend;
		return claim.blend;
	}
	if ( material.family == "pbr" )
	{
		if ( !worldPbr )
			return foundation::MakeUnexpected( std::string(
			    "world pbr is not enabled: the pbr point draws a world stage's surfaces only" ) );
		const PbrClaim claim = ClaimPbr( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.Variant().blend;
	}
	if ( material.family == "water" )
	{
		const WaterClaim claim = ClaimWater( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( !claim.reflectTarget && TextureOf( material, "envmap" ) == "env_cubemap" &&
		     !nativeReflectionProbes )
			return foundation::MakeUnexpected(
			    std::string( "$envmap needs the stage's native reflection probes" ) );
		return claim.blend;
	}
	return foundation::MakeUnexpected( "family " + material.family + " has no program yet" );
}

bool SupportsOpaqueBatch( const MaterialDesc &material, bool mesh )
{
	if ( material.family == "pbr" )
	{
		std::string why;
		const auto block = BlockFor( material, &why, true );
		if ( !block )
			return false;
		const PbrClaim claim = ClaimPbr( *block, true );
		return claim.claimed && !claim.transmission && !claim.translucent;
	}
	return mesh ? material.family == "vertexlit"
	            : material.family == "lightmapped" || material.family == "unlit";
}

foundation::Expected<device::BlendMode, std::string> ClaimForMesh(
    const MaterialDesc &material, bool nativeReflectionProbes, bool sceneColorAvailable )
{
	std::string why;
	const std::optional<ParameterBlock> block = BlockFor( material, &why, nativeReflectionProbes );
	if ( !block )
		return foundation::MakeUnexpected( why );
	if ( material.family == "depth" || material.family == "portal-mask" )
	{
		if ( auto unread = DepthClaim( *block, material.family == "portal-mask" ) )
			return foundation::MakeUnexpected( "the depth point does not draw " + *unread );
		return device::BlendMode::kOpaque;
	}

	if ( material.family == "vertexlit" )
	{
		const VertexLitMeshClaim claim = ClaimVertexLitMesh( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.blend;
	}
	if ( material.family == "pbr" )
	{
		const PbrClaim claim = ClaimPbr( *block, sceneColorAvailable );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.Variant().blend;
	}
	if ( material.family == "refract" )
	{
		const bool nativeEnvMap = TextureOf( material, "envmap" ) == "env_cubemap";
		const RefractClaim claim =
		    ClaimRefract( *block, sceneColorAvailable, nativeEnvMap, nativeReflectionProbes );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.envmap ? device::BlendMode::kOpaque : device::BlendMode::kAlpha;
	}
	if ( material.family == "unlit" )
	{
		const UnlitClaim claim = ClaimUnlitMesh( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( detail::ReadFlag( *block, "vertexcolor" ) ||
		     detail::ReadFlag( *block, "vertexalpha" ) )
			return foundation::MakeUnexpected( "model vertices have no color or alpha channel" );
		return claim.blend;
	}
	return foundation::MakeUnexpected( "family " + material.family + " has no mesh point yet" );
}

foundation::Expected<ResolvedProgram, std::string> ProgramResolver::Resolve(
    const MaterialDesc &material )
{
	State &s = *m_State;
	std::string why;
	const std::optional<ParameterBlock> block = BlockFor( material, &why,
	    ( s.mesh || material.family == "lightmapped" ) &&
	        ( s.sceneTerms & kSurfaceReflectionProbes ) != 0 );
	if ( !block )
		return foundation::MakeUnexpected( why );
	ResolvedProgram out;
	out.twoSided = IsSprite( material ) || detail::ReadFlag( *block, "nocull" );
	if ( material.family == "depth" || material.family == "portal-mask" )
	{
		if ( auto unread = DepthClaim( *block, material.family == "portal-mask" ) )
			return foundation::MakeUnexpected( "the depth point does not draw " + *unread );
		SurfaceVariant variant;
		variant.layout = s.layout;
		variant.terms = kSurfaceUnlit | kSurfaceDepthOnly;
		variant.portalMask = material.family == "portal-mask";
		SurfaceConstants constants;
		if ( variant.portalMask )
		{
			const float open = std::clamp( detail::ReadParameter( *block, "portalopenamount" ), 0.0f, 1.0f );
			const float smooth = open * open * ( 3.0f - 2.0f * open );
			constants.surfaceControls[2] = smooth * smooth;
			for ( int i = 0; i < 8; ++i )
				constants.texture2Transform[i] = detail::ReadParameter( *block, "texturetransform", i );
		}
		SurfaceTextures textures;
		auto request = s.lightmapped->Program().Request( variant, constants, textures );
		if ( !request )
			return foundation::MakeUnexpected( std::string( "a depth pipeline was refused" ) );
		out.name = "depth";
		out.request = std::move( request ).Value();
		return out;
	}

	if ( material.family == "lightmapped" )
	{
		LightmappedClaim claim = ClaimLightmapped( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		SurfaceTextures textures;
		textures.base = TextureOf( material, "basetexture" );
		textures.envmap = TextureOf( material, "envmap" );
		// The view's env_cubemap is the stage's reflection probes (RPRB),
		// which BlockFor admitted only when the scene carries them.
		if ( textures.envmap == "env_cubemap" )
		{
			claim.terms |= kSurfaceReflectionProbes;
			textures.envmap.clear();
		}
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
	if ( material.family == "unlit" || material.family == "cable" ||
	     material.family == "decal-modulate" )
	{
		const UnlitClaim claim = material.family == "cable" ? ClaimCable( *block )
		                         : material.family == "decal-modulate"
		                             ? ClaimDecalModulate( *block )
		                         : IsSprite( material ) ? ClaimSprite( *block )
		                         : s.mesh               ? ClaimUnlitMesh( *block )
		                                                : ClaimUnlit( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		SurfaceTextures textures;
		textures.baseSrgb = claim.baseSrgb;
		textures.base = TextureOf( material, "hdrbasetexture" );
		if ( textures.base.empty() )
			textures.base = TextureOf( material, "basetexture" );
		if ( claim.cable )
			textures.bump = TextureOf( material, "bumpmap" );
		if ( claim.twoTexture )
			textures.emission = TextureOf( material, "texture2" );
		if ( s.mesh && s.worldPbr && !claim.twoTexture && !claim.cable && !claim.decalModulate )
		{
			if ( detail::ReadFlag( *block, "vertexcolor" ) ||
			     detail::ReadFlag( *block, "vertexalpha" ) ||
			     s.layout == SurfaceVertexLayout::kFlat )
				return foundation::MakeUnexpected(
				    std::string( "the emissive mesh point needs a model vertex without color" ) );
			SurfaceVariant variant;
			variant.blend = claim.blend;
			variant.alphaWrite = claim.alphaWrite;
			variant.terms =
			    kSurfacePbr | ( s.sceneTerms & ~kSurfaceLightmapTerms ) | kSurfaceMeshDirect;
			variant.layout = s.layout;
			SurfaceConstants constants = claim.constants;
			constants.meshModes[3] = 1.0f;
			constants.meshModes[1] = claim.nativeProbe ? 1.0f : 0.0f;
			auto request = s.lightmapped->Program().Request( variant, constants, textures );
			if ( !request )
				return foundation::MakeUnexpected(
				    std::string( "the emissive mesh pipeline was refused" ) );
			out.name = "pbr";
			out.request = std::move( request ).Value();
			out.blend = claim.blend;
			return out;
		}
		// Unlit world surfaces keep the surface program's unlit point.
		auto request = s.lightmapped->Program().Request(
		    claim.Variant( s.layout ), claim.constants, textures );
		if ( !request )
			return foundation::MakeUnexpected( std::string( "an unlit pipeline was refused" ) );
		out.name = "unlit";
		out.depthBlend = claim.depthBlend;
		out.fogToBlack = claim.fogToBlack;
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		// Unlit points bind the neutral page; no captured lightmap is read.
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
		const PbrClaim claim = ClaimPbr( *block, s.sceneColorAvailable );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( s.layout == SurfaceVertexLayout::kFlat )
			return foundation::MakeUnexpected( std::string(
			    "the pbr point reads the surface vertex, and the resolver's is flat" ) );
		SurfaceVariant variant = claim.Variant();
		variant.layout = s.layout;
		if ( !s.mesh && s.layout == SurfaceVertexLayout::kModel )
			return foundation::MakeUnexpected(
			    std::string( "the pbr world point needs the world vertex" ) );
		variant.terms |= s.mesh ? ( ( s.sceneTerms & ~kSurfaceLightmapTerms ) | kSurfaceMeshDirect )
		                        : ( kSurfaceBakedLightmap | s.sceneTerms );
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
		out.blend = variant.blend;
		out.sceneColor = claim.transmission;
		// Runtime direct light: the gradient page is the indirect layer's own.
		if ( !s.mesh )
			out.drawInputs = { "lightmap",
			    ( variant.terms & kSurfaceRuntimeDirect ) ? "lightmap-indirect-gradient"
			                                              : "lightmap-gradient",
			    "lightmap-indirect" };
		return out;
	}
	if ( material.family == "vertexlit" )
	{
		if ( !s.mesh || !s.worldPbr )
			return foundation::MakeUnexpected(
			    std::string( "the modern VertexLitGeneric point needs a mesh scene" ) );
		const VertexLitMeshClaim claim = ClaimVertexLitMesh( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( s.layout == SurfaceVertexLayout::kFlat )
			return foundation::MakeUnexpected(
			    std::string( "the modern mesh point needs the surface vertex" ) );
		SurfaceVariant variant = claim.Variant();
		variant.layout = s.layout;
		variant.terms |= ( s.sceneTerms & ~kSurfaceLightmapTerms ) | kSurfaceMeshDirect;
		out.foliage = variant.treeSwayMode != 0;
		SurfaceTextures textures;
		textures.base = TextureOf( material, "basetexture" );
		if ( claim.normalMap )
			textures.bump = TextureOf( material, "bumpmap" );
		if ( claim.selfIllumMask )
			textures.emission = TextureOf( material, "selfillummask" );
		if ( claim.phongExponentTexture )
			textures.envmapMask = TextureOf( material, "phongexponenttexture" );
		else if ( claim.envmapMask )
			textures.envmapMask = TextureOf( material, "envmapmask" );
		if ( claim.detail )
			textures.detail = TextureOf( material, "detail" );
		if ( claim.lightwarp )
			textures.mrao = TextureOf( material, "lightwarptexture" );
		else if ( claim.phongWarp )
			textures.mrao = TextureOf( material, "phongwarptexture" );
		auto request = s.lightmapped->Program().Request( variant, claim.constants, textures );
		if ( !request )
			return foundation::MakeUnexpected(
			    std::string( "the modern mesh pipeline was refused" ) );
		out.name = "pbr";
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		// Model draws bind the neutral lighting block; probes and view lights
		// are the modern mesh point's sources, not a world lightmap page.
		return out;
	}
	if ( material.family == "refract" )
	{
		if ( !s.mesh || !s.worldPbr || s.layout == SurfaceVertexLayout::kFlat )
			return foundation::MakeUnexpected(
			    std::string( "the Refract point needs a model vertex in a scene" ) );
		const bool nativeProbes = ( s.sceneTerms & kSurfaceReflectionProbes ) != 0;
		const bool nativeEnvMap = TextureOf( material, "envmap" ) == "env_cubemap";
		const RefractClaim claim =
		    ClaimRefract( *block, s.sceneColorAvailable, nativeEnvMap, nativeProbes );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		SurfaceVariant variant = claim.Variant();
		variant.layout = s.layout;
		if ( claim.nativeProbe )
			variant.terms |= kSurfaceReflectionProbes;
		SurfaceTextures textures;
		if ( claim.baseTexture )
			textures.base = TextureOf( material, "basetexture" );
		textures.bump = TextureOf( material, "normalmap" );
		if ( claim.envmap && !claim.nativeProbe )
			textures.envmap = TextureOf( material, "envmap" );
		if ( claim.tintTexture )
			textures.emission = TextureOf( material, "refracttinttexture" );
		auto request = s.lightmapped->Program().Request( variant, claim.constants, textures );
		if ( !request )
			return foundation::MakeUnexpected( std::string( "a Refract pipeline was refused" ) );
		out.name = "refract";
		out.request = std::move( request ).Value();
		out.blend = claim.envmap ? device::BlendMode::kOpaque : device::BlendMode::kAlpha;
		out.sceneColor = claim.sceneColor;
		return out;
	}
	if ( material.family == "water" )
	{
		// The water point on the world vertex (its lightmap page is the
		// draw's, for $lightmapwaterfog).
		const WaterClaim claim = ClaimWater( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( s.layout != SurfaceVertexLayout::kWorld )
			return foundation::MakeUnexpected( std::string(
			    "the water point reads the surface vertex, and the resolver's is flat" ) );
		SurfaceTextures textures;
		textures.bump = TextureOf( material, "normalmap" );
		if ( claim.sludge )
			textures.base = TextureOf( material, "basetexture" );
		if ( claim.flow )
		{
			textures.flowmap = TextureOf( material, "flowmap" );
			textures.flowNoise = TextureOf( material, "flow_noise_texture" );
		}
		SurfaceVariant variant = claim.Variant();
		if ( claim.reflectTarget )
			out.viewInputs = { TextureOf( material, "reflecttexture" ) };
		else if ( TextureOf( material, "envmap" ) == "env_cubemap" )
		{
			// The view's env_cubemap is the stage's reflection probes (RPRB).
			if ( ( s.sceneTerms & kSurfaceReflectionProbes ) == 0 )
				return foundation::MakeUnexpected(
				    std::string( "$envmap needs the stage's native reflection probes" ) );
			variant.terms |= kSurfaceReflectionProbes;
		}
		else
			textures.envmap = TextureOf( material, "envmap" );
		auto request = s.lightmapped->Program().Request( variant, claim.constants, textures );
		if ( !request )
			return foundation::MakeUnexpected( std::string( "a water pipeline was refused" ) );
		out.name = "water";
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		out.drawInputs = { "lightmap" };
		return out;
	}
	return foundation::MakeUnexpected( "family " + material.family + " has no program yet" );
}

void ProgramResolver::SetWorldPbr( bool enabled, std::uint32_t sceneTerms )
{
	m_State->worldPbr = enabled;
	m_State->sceneTerms = sceneTerms;
}

void ProgramResolver::SetSceneColorAvailable( bool available )
{
	m_State->sceneColorAvailable = available;
}

foundation::Expected<ResolvedProgram, std::string> ProgramResolver::ResolveMesh(
    const MaterialDesc &material )
{
	m_State->mesh = true;
	auto resolved = Resolve( material );
	m_State->mesh = false;
	return resolved;
}

foundation::Expected<device::PipelineId, std::string> ProgramResolver::VariantPipeline(
    const ResolvedProgram &program, std::uint32_t add, std::uint32_t remove )
{
	auto pipeline =
	    m_State->lightmapped->Program().VariantPipeline( program.request.pipeline, add, remove );
	if ( !pipeline )
		return foundation::MakeUnexpected(
		    "the variant of " + program.name + " was refused or not made by this resolver" );
	return pipeline.Value();
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
	if ( program.name == "pbr" && inputTextures.size() == 3 )
		return s.lightmapped->Program().DrawGroup(
		    inputTextures[0], {}, {}, inputTextures[1], inputTextures[2] );
	if ( ( program.name == "pbr" || program.name == "refract" || program.name == "depth" ||
	         program.name == "unlit" ) &&
	     inputTextures.empty() )
		return s.lightmapped->Program().DrawGroup( "", {}, {}, {}, {} );
	if ( program.request.drawLayout == s.lightmapped->DrawLayout() && inputTextures.size() == 1 )
		return s.lightmapped->LightmapGroup( inputTextures[0] );
	return std::nullopt;
}

std::optional<std::string> FrameInputError( const ResolvedProgram &program, const FrameTerms &terms )
{
	if ( program.foliage )
	{
		if ( !terms.foliageAvailable )
			return "$treesway needs captured wind and animation time";
		for ( const auto &sample : terms.foliage )
			for ( float value : sample )
				if ( !std::isfinite( value ) )
					return "$treesway wind and animation time must be finite";
	}
	return std::nullopt;
}

std::optional<GroupRequest> ProgramResolver::FrameGroup(
    const ResolvedProgram &program, const FrameTerms &terms ) const
{
	const State &s = *m_State;
	if ( !program.request.frameLayout.IsValid() )
		return std::nullopt;
	if ( FrameInputError( program, terms ) )
		return std::nullopt;
	if ( program.request.frameLayout == s.lightmapped->FrameLayout() )
	{
		SurfaceFrame frame;
		std::copy_n( terms.motionCurrentToClip, 16, frame.motionCurrentToClip );
		std::copy_n( terms.motionPreviousToClip, 16, frame.motionPreviousToClip );
		std::copy_n( terms.motionExtent, 4, frame.motionExtent );
		std::memcpy( frame.clipPlanes, terms.clipPlanes, sizeof( frame.clipPlanes ) );
		frame.light[0] = terms.lightmapScale;
		frame.light[1] = terms.outputScale;
		frame.light[2] = terms.encodeOutput ? 1.0f : 0.0f;
		std::copy( terms.fogColor, terms.fogColor + 3, frame.fogColor );
		if ( program.fogToBlack )
			std::fill_n( frame.fogColor, 3, 0.0f );
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
		std::copy( terms.sunDirection, terms.sunDirection + 4, frame.sunDirection );
		std::copy( terms.sunColor, terms.sunColor + 4, frame.sunColor );
		std::copy( terms.sunShadow, terms.sunShadow + 4, frame.sunShadow );
		frame.water[0] = terms.time;
		std::memcpy( frame.foliage, terms.foliage, sizeof( frame.foliage ) );
		frame.water[1] = terms.waterReflectTintScale;
		frame.water[2] = terms.viewRight[0];
		frame.water[3] = terms.viewRight[1];
		std::copy( terms.viewport, terms.viewport + 4, frame.viewport );
		std::copy( terms.areas.begin(), terms.areas.begin() + std::ptrdiff_t( areas ), frame.areas );
		return s.lightmapped->Program().FrameGroup(
		    frame, terms.splitSumTable, terms.ltcTable, terms.map );
	}
	return std::nullopt;
}

} // namespace render::material
