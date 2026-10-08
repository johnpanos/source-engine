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
#include "render/material/energy_family.h"
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
#include <cstdio>
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

// SpriteCard's particle cards (render.sprite-card.v1).
bool IsSpriteCard( const MaterialDesc &material )
{
	return material.legacyShader == "spritecard_dx8";
}

// The Black shader's surfaces: the unlit point with a zero tint.
bool IsBlack( const MaterialDesc &material )
{
	return material.legacyShader == "black";
}

// Wireframe, and Eyeball, whose SHADER_FALLBACK is Wireframe (eyeball.cpp).
bool IsWireframe( const MaterialDesc &material )
{
	return material.legacyShader == "wireframe" || material.legacyShader == "wireframe_dx9" ||
	       material.legacyShader == "eyeball";
}

// The Sky shader's faces (render.pass.sky); HDR selects its encodings.
bool IsSky( const MaterialDesc &material )
{
	return material.legacyShader == "sky_hdr_dx9" || material.legacyShader == "sky_dx9";
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

// A matrix parameter's declared default in the shader's text form ("center .5
// .5 scale 1 1 rotate 0 translate 0 0", as Refract declares $bumptransform),
// while the material system reports the live value as its 4x4 matrix. Only
// the identity is converted: scale 1 1, rotate 0 and translate 0 0 make the
// centre irrelevant. Any other text stays text.
std::optional<std::vector<double>> IdentityTransformText( const std::string &declared )
{
	std::string text;
	for ( const char c : declared )
		text += char( std::tolower( static_cast<unsigned char>( c ) ) );
	double cx, cy, sx, sy, rotate, tx, ty;
	char trailing;
	if ( std::sscanf( text.c_str(), " center %lf %lf scale %lf %lf rotate %lf translate %lf %lf %c",
	         &cx, &cy, &sx, &sy, &rotate, &tx, &ty, &trailing ) != 7 )
		return std::nullopt;
	if ( sx != 1.0 || sy != 1.0 || rotate != 0.0 || tx != 0.0 || ty != 0.0 )
		return std::nullopt;
	return std::vector<double>{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
}

// Whether a variable's value is its declared default: equal numbers, or equal
// text ignoring case. An empty declared default is 0 for a number.
bool AtDefault( const std::string &value, const std::string &declared )
{
	const std::optional<std::vector<double>> a = Numbers( value );
	std::optional<std::vector<double>> b = Numbers( declared );
	if ( !b )
		b = IdentityTransformText( declared );
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
	// VertexLitGeneric's optional passes (vertexlitgeneric_dx9.cpp): each reads
	// its controls only when its enable flag is set (SHADER_INIT_PARAMS,
	// SHADER_FALLBACK and DRAW all test it), so with the pass off they are inert
	// whatever the live material holds (the material system's zeros, or a
	// dormant cloak factor of 1 on the cube).
	struct DormantPass
	{
		const char *enable;
		const char *controls[24]; // ends at the first null
	};
	static const DormantPass kVertexLitPasses[] = {
	    { "$cloakpassenabled", { "$cloakpassenabled", "$cloakfactor", "$cloakcolortint",
	                               "$cloaktint", "$refractamount" } },
	    { "$emissiveblendenabled",
	        { "$emissiveblendenabled", "$emissiveblendbasetexture", "$emissiveblendscrollvector",
	            "$emissiveblendstrength", "$emissiveblendtexture", "$emissiveblendtint",
	            "$emissiveblendflowtexture" } },
	    { "$sheenpassenabled",
	        { "$sheenpassenabled", "$sheenmap", "$sheenmapmask", "$sheenmapmaskframe",
	            "$sheenmaptint", "$sheenmapmaskscalex", "$sheenmapmaskscaley",
	            "$sheenmapmaskoffsetx", "$sheenmapmaskoffsety", "$sheenmapmaskdirection",
	            "$sheenindex" } },
	    { "$fleshinteriorenabled",
	        { "$fleshinteriorenabled", "$fleshinteriortexture", "$fleshinteriornoisetexture",
	            "$fleshbordertexture1d", "$fleshnormaltexture", "$fleshsubsurfacetexture",
	            "$fleshcubetexture", "$fleshbordernoisescale", "$fleshdebugforcefleshon",
	            "$fleshEffectCenterRadius1", "$fleshEffectCenterRadius2",
	            "$fleshEffectCenterRadius3", "$fleshEffectCenterRadius4", "$fleshsubsurfacetint",
	            "$fleshborderwidth", "$fleshbordersoftness", "$fleshbordertint",
	            "$fleshglobalopacity", "$fleshglossbrightness", "$fleshscrollspeed" } } };
	// UnlitTwoTexture declares the same cloak pass (unlittwotexture_dx9.cpp,
	// SetupVarsCloakBlendedPass), read only with $cloakpassenabled set: the
	// sprite copies Portal 2's lasers draw hold $refractamount 0 inertly.
	const auto dormant = [&]( std::string_view key )
	{
		const bool unlit = material.family == "unlit";
		if ( material.family != "vertexlit" && !unlit )
			return false;
		for ( const DormantPass &pass : kVertexLitPasses )
		{
			if ( unlit && &pass != &kVertexLitPasses[0] )
				break;
			if ( enabled( pass.enable ) )
				continue;
			for ( const char *control : pass.controls )
			{
				if ( !control )
					break;
				if ( SameKey( key, control ) )
					return true;
			}
		}
		return false;
	};
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
		if ( dormant( key ) )
			continue;
		// Refract never reads $alpha: refract_ps2x writes the normal map's
		// alpha (times the vertex alpha with $vertexcolormodulate), and
		// refract_dx9_helper.cpp enables no blend that $alpha modulates (only
		// $masked's). A prop fading its $alpha (the breaking glass panes'
		// glass_fracture_* proxies) draws the same pixels.
		if ( material.family == "refract" && SameKey( key, "$alpha" ) )
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
	// The client's render-to-texture shadow page (clientshadowmgr.cpp).
	if ( material.family == "blob-shadow" && value.parameter == "basetexture" &&
	     SameKey( value.text, "_rt_Shadows" ) )
		return true;
	return material.family == "water" &&
	       ( ( value.parameter == "reflecttexture" &&
	             SameKey( value.text, "_rt_WaterReflection" ) ) ||
	           ( value.parameter == "refracttexture" &&
	               SameKey( value.text, "_rt_WaterRefraction" ) ) );
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
		// The pbr point reflects the view's probes: a named cube would be
		// ignored, so only env_cubemap (P2:CE's "nearest cubemap") is read.
		if ( material.family == "pbr" && value.parameter == "envmap" &&
		     value.text != "env_cubemap" )
		{
			*why = "the pbr point reflects the view's probes, not the named cube " + value.text +
			       " ($envmap)";
			return std::nullopt;
		}
		if ( ( material.family == "vertexlit" || material.family == "unlit" ||
		         ( ( material.family == "refract" || material.family == "lightmapped" ||
		               material.family == "pbr" ) &&
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

// Whether scene terms are a world stage's. A plain map's cubemaps supply
// reflection probes alone; its models keep Source's model lighting at the
// draw (its ambient cube and lights) rather than the stage's direct light.
static bool StageScene( std::uint32_t sceneTerms )
{
	return ( sceneTerms & ~kSurfaceReflectionProbes ) != 0;
}

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
	// Teeth's factor, or Eyes' projections and textures, while Resolve draws
	// them as VertexLitGeneric.
	std::optional<TeethClaim> teeth;
	std::optional<EyesClaim> eyes;
	std::string eyesIris;
	std::string eyesGlint;
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

// Teeth or Eyes as the VertexLitGeneric description its point draws: the
// base and the parameters both shaders read the same way.
MaterialDesc AsVertexLit( const MaterialDesc &material )
{
	MaterialDesc out = material;
	out.family = "vertexlit";
	out.shader = "VertexLitGeneric";
	out.legacyShader = "vertexlitgeneric";
	out.values.clear();
	for ( const MaterialValue &value : material.values )
		if ( value.parameter == "basetexture" || value.parameter == "frame" ||
		     value.parameter == "basetexturetransform" || value.parameter == "translucent" ||
		     value.parameter == "nocull" || value.parameter == "model" || value.parameter == "nofog" ||
		     value.parameter == "halflambert" )
			out.values.push_back( value );
	return out;
}

foundation::Expected<device::BlendMode, std::string> ClaimForDrawing( const MaterialDesc &material,
    bool worldPbr, bool *requiresDepthAlpha, bool nativeReflectionProbes )
{
	if ( requiresDepthAlpha )
		*requiresDepthAlpha = false;
	std::string why;
	const std::optional<ParameterBlock> block =
	    BlockFor( material, &why,
	        ( material.family == "lightmapped" || material.family == "pbr" ) &&
	            nativeReflectionProbes );
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
	     material.family == "decal-modulate" || material.family == "energy" ||
	     material.family == "modulate" || material.family == "blob-shadow" ||
	     material.family == "shadow-build" )
	{
		const UnlitClaim claim =
		    material.family == "cable"            ? ClaimCable( *block )
		    : material.family == "energy"         ? ClaimEnergy( *block )
		    : material.family == "modulate"       ? ClaimModulate( *block )
		    : material.family == "blob-shadow"    ? ClaimBlobShadow( *block )
		    : material.family == "shadow-build"   ? ClaimShadowBuild( *block )
		    : material.family == "decal-modulate" ? ClaimDecalModulate( *block )
		    : IsSprite( material )                ? ClaimSprite( *block )
		    : IsSpriteCard( material )            ? ClaimSpriteCard( *block )
		    : IsBlack( material )                 ? ClaimBlack( *block )
		    : IsWireframe( material )             ? ClaimWireframe( *block )
		    : IsSky( material ) ? ClaimSky( *block, material.legacyShader == "sky_hdr_dx9" )
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

std::string MeshSpecularExponentSource( const MaterialDesc &material )
{
	if ( material.family != "vertexlit" )
		return {};
	std::string why;
	const std::optional<ParameterBlock> block = BlockFor( material, &why, false );
	if ( !block )
		return {};
	const VertexLitMeshClaim claim = ClaimVertexLitMesh( *block );
	if ( !claim.claimed || claim.constants.meshModes[2] <= 0.5f )
		return {};
	if ( claim.phongExponentFromMap )
		return "map";
	char text[32];
	std::snprintf( text, sizeof( text ), "%g", claim.phongExponent );
	return text;
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
		return claim.Blend();
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
	if ( material.family == "eyes" )
	{
		const EyesClaim eyes = ClaimEyes( *block );
		if ( !eyes.claimed )
			return foundation::MakeUnexpected( eyes.reason );
		return ClaimForMesh( AsVertexLit( material ), nativeReflectionProbes, sceneColorAvailable );
	}
	if ( material.family == "teeth" )
	{
		const TeethClaim teeth = ClaimTeeth( *block );
		if ( !teeth.claimed )
			return foundation::MakeUnexpected( teeth.reason );
		return ClaimForMesh( AsVertexLit( material ), nativeReflectionProbes,
		    sceneColorAvailable );
	}
	if ( material.family == "shadow-build" )
	{
		// A caster's coverage into the shadow page: the model vertex's
		// position and base coordinate only.
		const UnlitClaim claim = ClaimShadowBuild( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		return claim.blend;
	}
	if ( material.family == "modulate" )
	{
		const UnlitClaim claim = ClaimModulate( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		if ( detail::ReadFlag( *block, "vertexcolor" ) ||
		     detail::ReadFlag( *block, "vertexalpha" ) )
			return foundation::MakeUnexpected( "model vertices have no color or alpha channel" );
		return claim.blend;
	}
	if ( material.family == "energy" )
	{
		const UnlitClaim claim = ClaimEnergy( *block );
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
	    ( s.mesh || material.family == "lightmapped" || material.family == "pbr" ) &&
	        ( s.sceneTerms & kSurfaceReflectionProbes ) != 0 );
	if ( !block )
		return foundation::MakeUnexpected( why );
	ResolvedProgram out;
	out.twoSided =
	    IsSprite( material ) || IsSpriteCard( material ) || detail::ReadFlag( *block, "nocull" );
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
			return foundation::MakeUnexpected( "a depth pipeline was refused (" +
			    s.lightmapped->Program().PipelineFailure() + ")" );
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
		if ( claim.blendTexture2 )
		{
			// WorldVertexTransition's second layer (SurfaceVariant::blendTexture2).
			textures.emission = TextureOf( material, "basetexture2" );
			textures.mrao = TextureOf( material, "bumpmap2" );
			textures.envmapMask = TextureOf( material, "blendmodulatetexture" );
		}
		auto request = s.lightmapped->Request( claim, textures, s.layout );
		if ( !request )
			return foundation::MakeUnexpected(
			    std::string( request.Error() == LightmappedStatus::kInvalidRequest
			                     ? "its bump or env map term reads the surface vertex, and the "
			                       "resolver's is flat"
			                     : "a lightmapped pipeline was refused (" +
			                           s.lightmapped->Program().PipelineFailure() + ")" ) );
		out.name = "lightmapped";
		out.request = std::move( request ).Value();
		out.blend = claim.blend;
		out.drawInputs = { "lightmap" };
		return out;
	}
	if ( material.family == "unlit" || material.family == "cable" ||
	     material.family == "decal-modulate" || material.family == "energy" ||
	     material.family == "modulate" || material.family == "blob-shadow" ||
	     material.family == "shadow-build" )
	{
		const UnlitClaim claim =
		    material.family == "cable"            ? ClaimCable( *block )
		    : material.family == "energy"         ? ClaimEnergy( *block )
		    : material.family == "modulate"       ? ClaimModulate( *block )
		    : material.family == "blob-shadow"    ? ClaimBlobShadow( *block )
		    : material.family == "shadow-build"   ? ClaimShadowBuild( *block )
		    : material.family == "decal-modulate" ? ClaimDecalModulate( *block )
		    : IsSprite( material )                ? ClaimSprite( *block )
		    : IsSpriteCard( material )            ? ClaimSpriteCard( *block )
		    : IsBlack( material )                 ? ClaimBlack( *block )
		    : IsWireframe( material )             ? ClaimWireframe( *block )
		    : IsSky( material ) ? ClaimSky( *block, material.legacyShader == "sky_hdr_dx9" )
		    : s.mesh            ? ClaimUnlitMesh( *block )
		                        : ClaimUnlit( *block );
		if ( !claim.claimed )
			return foundation::MakeUnexpected( claim.reason );
		SurfaceTextures textures;
		textures.baseSrgb = claim.baseSrgb;
		textures.base = claim.baseParameter.empty() ? TextureOf( material, "hdrbasetexture" )
		                                            : TextureOf( material, claim.baseParameter );
		if ( textures.base.empty() && claim.baseParameter.empty() )
			textures.base = TextureOf( material, "basetexture" );
		if ( claim.cable )
			textures.bump = TextureOf( material, "bumpmap" );
		if ( claim.wireframe &&
		     !s.device.Facts().capabilities.Has( device::Capability::kFillModeLines ) )
			return foundation::MakeUnexpected(
			    std::string( "Wireframe needs the device's line fill (kFillModeLines, D38)" ) );
		if ( claim.energy )
		{
			if ( s.layout == SurfaceVertexLayout::kFlat )
				return foundation::MakeUnexpected(
				    std::string( "the energy point reads the tangent frame, and the "
				                 "resolver's vertex is flat" ) );
			textures.detail = TextureOf( material, "detail1" );
			textures.detail2 = TextureOf( material, "detail2" );
			textures.flowmap = TextureOf( material, "flowmap" );
			textures.flowNoise = TextureOf( material, "flow_noise_texture" );
			textures.flowBounds = TextureOf( material, "flowbounds" );
		}
		if ( claim.twoTexture )
			textures.emission = TextureOf( material, "texture2" );
		if ( s.mesh && s.worldPbr && !claim.twoTexture && !claim.cable && !claim.decalModulate &&
		     !claim.energy && !claim.wireframe &&
		     !IsBlack( material ) )
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
		variant.terms |= s.mesh ? ( ( s.sceneTerms & ~kSurfaceLightmapTerms ) |
		                              ( StageScene( s.sceneTerms ) ? kSurfaceMeshDirect : 0u ) )
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
			    "lightmap-indirect", "lightmap-shadow-mask" };
		return out;
	}
	if ( material.family == "teeth" )
	{
		const TeethClaim teeth = ClaimTeeth( *block );
		if ( !teeth.claimed )
			return foundation::MakeUnexpected( teeth.reason );
		s.teeth = teeth;
		auto resolved = Resolve( AsVertexLit( material ) );
		s.teeth.reset();
		return resolved;
	}
	if ( material.family == "eyes" )
	{
		const EyesClaim eyes = ClaimEyes( *block );
		if ( !eyes.claimed )
			return foundation::MakeUnexpected( eyes.reason );
		s.eyes = eyes;
		s.eyesIris = TextureOf( material, "iris" );
		s.eyesGlint = TextureOf( material, "glint" );
		auto resolved = Resolve( AsVertexLit( material ) );
		s.eyes.reset();
		return resolved;
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
		// Without a stage's terms (a plain map) the point takes Source's model
		// lighting at the draw: its ambient cube and lights.
		variant.terms |= ( s.sceneTerms & ~kSurfaceLightmapTerms ) |
		                 ( StageScene( s.sceneTerms ) ? kSurfaceMeshDirect : 0u );
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
		SurfaceConstants constants = claim.constants;
		if ( s.teeth )
		{
			std::copy_n( s.teeth->forward, 3, constants.teeth[0] );
			constants.teeth[0][3] = s.teeth->illum;
			constants.teeth[1][0] = 1.0f;
		}
		if ( s.eyes )
		{
			// The iris (sRGB) at the emission binding and the glint (data)
			// at MRAO's: VertexLitGeneric's base claim reads neither.
			std::copy_n( s.eyes->origin, 3, constants.eyes[0] );
			constants.eyes[0][3] = 1.0f;
			std::copy_n( s.eyes->up, 3, constants.eyes[1] );
			constants.eyes[1][3] = s.eyes->glint ? 1.0f : 0.0f;
			std::copy_n( s.eyes->irisU, 4, constants.eyes[2] );
			std::copy_n( s.eyes->irisV, 4, constants.eyes[3] );
			std::copy_n( s.eyes->glintU, 4, constants.eyes[4] );
			std::copy_n( s.eyes->glintV, 4, constants.eyes[5] );
			textures.emission = s.eyesIris;
			textures.mrao = s.eyesGlint;
		}
		auto request = s.lightmapped->Program().Request( variant, constants, textures );
		if ( !request )
			return foundation::MakeUnexpected(
			    "the modern mesh pipeline was refused (" +
			    s.lightmapped->Program().PipelineFailure() + ")" );
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
		out.blend = claim.Blend();
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
		if ( claim.refractTarget )
			out.refractInput = TextureOf( material, "refracttexture" );
		if ( claim.reflectTarget )
			out.viewInputs = { TextureOf( material, "reflecttexture" ) };
		else if ( claim.envReflection && TextureOf( material, "envmap" ) == "env_cubemap" )
		{
			// The view's env_cubemap is the stage's reflection probes (RPRB).
			if ( ( s.sceneTerms & kSurfaceReflectionProbes ) == 0 )
				return foundation::MakeUnexpected(
				    std::string( "$envmap needs the stage's native reflection probes" ) );
			variant.terms |= kSurfaceReflectionProbes;
		}
		else if ( claim.envReflection )
			textures.envmap = TextureOf( material, "envmap" );
		// Else nothing reflects: $reflecttint is zero and the env map neutral.
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

foundation::Expected<device::PipelineId, std::string> ProgramResolver::SkinnedPipeline(
    const ResolvedProgram &program )
{
	auto pipeline = m_State->lightmapped->Program().SkinnedPipeline( program.request.pipeline );
	if ( !pipeline )
		return foundation::MakeUnexpected( "the skinned variant of " + program.name +
		                                   " was refused (the reduced model's lit world point only)" );
	return pipeline.Value();
}

foundation::Expected<device::PipelineId, std::string> ProgramResolver::StaticVertexLightPipeline(
    const ResolvedProgram &program )
{
	auto pipeline =
	    m_State->lightmapped->Program().StaticVertexLightPipeline( program.request.pipeline );
	if ( !pipeline )
		return foundation::MakeUnexpected(
		    "the static vertex light variant of " + program.name +
		    " was refused (a world-vertex point of this resolver only)" );
	return pipeline.Value();
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

std::optional<GroupRequest> ProgramResolver::DrawGroup( const ResolvedProgram &program,
    const std::vector<std::string> &inputTextures, const ModelLighting *lighting,
    std::span<const float> bonePalette ) const
{
	const State &s = *m_State;
	if ( !program.request.drawLayout.IsValid() ||
	     inputTextures.size() != program.drawInputs.size() )
		return std::nullopt;
	if ( program.name == "pbr" && inputTextures.size() == 4 )
		return s.lightmapped->Program().DrawGroup(
		    inputTextures[0], {}, {}, inputTextures[1], inputTextures[2], inputTextures[3] );
	if ( ( program.name == "pbr" || program.name == "refract" || program.name == "depth" ||
	         program.name == "unlit" ) &&
	     inputTextures.empty() )
		return s.lightmapped->Program().DrawGroup(
		    "", lighting ? *lighting : ModelLighting{}, {}, {}, {}, {}, bonePalette );
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

namespace
{

// The SurfaceFrame block of a program's frame terms.
SurfaceFrame PackSurfaceFrame( const ResolvedProgram &program, const FrameTerms &terms )
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
	return frame;
}

} // namespace

std::optional<GroupRequest> ProgramResolver::FrameGroup(
    const ResolvedProgram &program, const FrameTerms &terms ) const
{
	const State &s = *m_State;
	if ( !program.request.frameLayout.IsValid() || FrameInputError( program, terms ) ||
	     program.request.frameLayout != s.lightmapped->FrameLayout() )
		return std::nullopt;
	return s.lightmapped->Program().FrameGroup(
	    PackSurfaceFrame( program, terms ), terms.splitSumTable, terms.ltcTable, terms.map );
}

std::optional<std::vector<std::byte>> ProgramResolver::FrameConstants(
    const ResolvedProgram &program, const FrameTerms &terms ) const
{
	const State &s = *m_State;
	if ( !program.request.frameLayout.IsValid() || FrameInputError( program, terms ) ||
	     program.request.frameLayout != s.lightmapped->FrameLayout() )
		return std::nullopt;
	const SurfaceFrame frame = PackSurfaceFrame( program, terms );
	const auto bytes = std::as_bytes( std::span( &frame, 1 ) );
	return std::vector<std::byte>( bytes.begin(), bytes.end() );
}

std::optional<sprite_card::Frame> SpriteCardTermsFor( const MaterialDesc &material )
{
	if ( !IsSpriteCard( material ) )
		return std::nullopt;
	std::string why;
	const std::optional<ParameterBlock> block = BlockFor( material, &why );
	if ( !block )
		return std::nullopt;
	return SpriteCardTerms( *block );
}

} // namespace render::material
