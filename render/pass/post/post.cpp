//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.post (RFC 0016 K8 "Post and screen effects"); see
//			post.h.
//
//=============================================================================//

#include "render/pass/post/post.h"

#include "render/shaderlib/core_artifacts.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <string>

namespace render::pass::post
{

namespace
{

using namespace render::device;

constexpr const char *kFragment = "render/pass/post/post.frag";
constexpr const char *kVertex = "render/pass/output/output.vert";

// post.frag's modes.
constexpr std::uint32_t kModeDownsample = 0;
constexpr std::uint32_t kModeBlurX = 1;
constexpr std::uint32_t kModeBlurY = 2;

// The fragment stage's draw constants (48 bytes).
struct Constants
{
	float tint[4] = {};   // the bloom tint and exponent
	float params[4] = {}; // x exposure, y bloom amount, zw the source's texel size (1/w, 1/h)
	std::uint32_t modes[4] = {}; // x the mode
};
static_assert( sizeof( Constants ) == 48 );

bool Finite( float value )
{
	return std::isfinite( value );
}

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	return a.size() == b.size() &&
	       std::equal( a.begin(), a.end(), b.begin(),
	           []( char x, char y )
	           {
		           return std::tolower( (unsigned char)x ) == std::tolower( (unsigned char)y );
	           } );
}

std::optional<std::string_view> Find(
    std::span<const PostDrawVariable> variables, std::string_view key )
{
	for ( const PostDrawVariable &variable : variables )
		if ( EqualsNoCase( variable.key, key ) )
			return variable.value;
	return std::nullopt;
}

float FloatOf( std::string_view text, float fallback )
{
	const std::string copy( text );
	char *end = nullptr;
	const float value = std::strtof( copy.c_str(), &end );
	return end != copy.c_str() && Finite( value ) ? value : fallback;
}

} // namespace

foundation::Expected<PostClaim, std::string_view> ClaimPostDraw(
    std::string_view shader, std::span<const PostDrawVariable> variables )
{
	PostClaim claim;
	if ( EqualsNoCase( shader, "Downsample_nohdr" ) )
	{
		claim.role = PostRole::kDownsample;
		// BLOOMTINTENABLE (default 1); $cstrike selects Counter-Strike's shape.
		if ( auto value = Find( variables, "$cstrike" ); value && FloatOf( *value, 0.0f ) != 0.0f )
			return foundation::MakeUnexpected(
			    std::string_view( "post: Downsample_nohdr $cstrike shape is not claimed" ) );
		if ( auto value = Find( variables, "$bloomtint" ) )
		{
			const std::string text( *value );
			const char *cursor = text.c_str();
			for ( float &channel : claim.tint )
			{
				while ( *cursor == '[' || *cursor == ' ' )
					++cursor;
				char *end = nullptr;
				const float parsed = std::strtof( cursor, &end );
				if ( end == cursor || !Finite( parsed ) )
					return foundation::MakeUnexpected( std::string_view(
					    "post: Downsample_nohdr $bloomtint is not four numbers" ) );
				channel = parsed;
				cursor = end;
			}
		}
		if ( auto value = Find( variables, "$bloomtintenable" );
		    value && FloatOf( *value, 1.0f ) == 0.0f )
		{
			claim.tint[0] = claim.tint[1] = claim.tint[2] = 0.333f;
			claim.tint[3] = 1.0f;
		}
		return claim;
	}
	if ( EqualsNoCase( shader, "BlurFilterX" ) )
	{
		claim.role = PostRole::kBlurX;
		return claim;
	}
	if ( EqualsNoCase( shader, "BlurFilterY" ) )
	{
		claim.role = PostRole::kBlurY;
		auto value = Find( variables, "$bloomamount" );
		claim.bloomAmount = value ? FloatOf( *value, 1.0f ) : 1.0f;
		if ( claim.bloomAmount < 0.0f )
			return foundation::MakeUnexpected(
			    std::string_view( "post: BlurFilterY $bloomamount is negative" ) );
		return claim;
	}
	if ( EqualsNoCase( shader, "Engine_Post" ) || EqualsNoCase( shader, "Engine_Post_dx9" ) )
	{
		claim.role = PostRole::kAdd;
		// $AAINTERNAL1.x is the software AA's strength; 0 is off.
		if ( auto value = Find( variables, "$aainternal1" ) )
		{
			std::string_view text = *value;
			while ( !text.empty() && ( text.front() == '[' || text.front() == ' ' ) )
				text.remove_prefix( 1 );
			if ( FloatOf( text, 0.0f ) != 0.0f )
				return foundation::MakeUnexpected(
				    std::string_view( "post: Engine_Post software AA is not claimed" ) );
		}
		if ( auto value = Find( variables, "$bloomenable" ) )
			claim.bloomEnabled = FloatOf( *value, 1.0f ) != 0.0f;
		return claim;
	}
	if ( EqualsNoCase( shader, "screenspace_general" ) ||
	     EqualsNoCase( shader, "screenspace_general_dx9" ) )
	{
		// Portal 2's dev/bloomadd: the blurred bloom added to the frame.
		auto pixel = Find( variables, "$pixshader" );
		if ( pixel && ( EqualsNoCase( *pixel, "bloomadd_ps20" ) ||
		                  EqualsNoCase( *pixel, "bloomadd_ps20b" ) ||
		                  EqualsNoCase( *pixel, "bloomadd_ps30" ) ) )
		{
			claim.role = PostRole::kAdd;
			return claim;
		}
		return foundation::MakeUnexpected(
		    std::string_view( "post: screenspace_general program is not claimed" ) );
	}
	return foundation::MakeUnexpected( std::string_view( "post: shader is not a post draw" ) );
}

foundation::Expected<std::unique_ptr<BloomRenderer>, PostStatus> BloomRenderer::Create(
    IRenderDevice2 &device )
{
	return CreateWithFragment( device, {} );
}

foundation::Expected<std::unique_ptr<BloomRenderer>, PostStatus> BloomRenderer::CreateWithFragment(
    IRenderDevice2 &device, std::span<const std::uint32_t> fragmentSpirv )
{
	std::unique_ptr<BloomRenderer> renderer( new BloomRenderer( device ) );
	const BindingDesc draw[] = { { 0, BindingKind::kSampledTexture, 1, { ShaderStage::kFragment } },
	    { 1, BindingKind::kSampler, 1, { ShaderStage::kFragment } } };
	auto layout = device.CreateBindGroupLayout( { BindGroupRole::kDraw, draw } );
	if ( !layout )
		return foundation::MakeUnexpected( PostStatus::kDevice );
	renderer->m_Layout = layout.Value();
	SamplerDesc samplerDesc;
	samplerDesc.minFilter = samplerDesc.magFilter = Filter::kLinear;
	samplerDesc.mipFilter = Filter::kNearest;
	samplerDesc.address = AddressMode::kClampToEdge;
	auto sampler = device.CreateSampler( samplerDesc );
	if ( !sampler )
		return foundation::MakeUnexpected( PostStatus::kDevice );
	renderer->m_Sampler = sampler.Value();

	shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
	if ( !fragmentSpirv.empty() &&
	     !artifacts.ReplaceSpirv( kFragment, fragmentSpirv, device.Facts().artifactFormat ) )
		return foundation::MakeUnexpected( PostStatus::kDevice );
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe( { kVertex, kFragment } );
	recipe.layouts = {
	    BindGroupLayoutId(), BindGroupLayoutId(), BindGroupLayoutId(), renderer->m_Layout };
	recipe.topology = PrimitiveTopology::kTriangleList;
	recipe.raster.cull = CullMode::kNone;
	// assign(1, ...): GCC 13 misreports a one-element initializer list (-Warray-bounds).
	recipe.colorFormats.assign( 1, Format::kRGBA8Unorm );
	recipe.debugName = "render.pass.post";
	auto resolved = shaderlib::Resolve( recipe, artifacts, device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected( PostStatus::kDevice );
	PipelineDesc desc = resolved.Value().Desc();
	desc.drawConstantBytes = sizeof( Constants );
	auto pipeline = device.CreatePipeline( desc );
	if ( !pipeline )
		return foundation::MakeUnexpected( PostStatus::kDevice );
	renderer->m_Pipeline = pipeline.Value();
	return renderer;
}

BloomRenderer::~BloomRenderer()
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, m_LastToken );
	for ( TextureId image : m_Retired )
		(void)m_Device.Release( ResourceId( image ), m_LastToken );
	for ( TextureId image : m_Images )
		if ( image.IsValid() )
			(void)m_Device.Release( ResourceId( image ), m_LastToken );
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, m_LastToken );
	if ( m_Sampler.IsValid() )
		(void)m_Device.Release( m_Sampler, m_LastToken );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, m_LastToken );
}

bool BloomRenderer::EnsureImages( std::uint32_t width, std::uint32_t height )
{
	if ( m_Images[0].IsValid() && width == m_Width && height == m_Height )
		return true;
	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		for ( TextureId &image : m_Images )
		{
			if ( image.IsValid() )
				m_Retired.push_back( image );
			image = TextureId();
		}
	}
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm;
	desc.width = width;
	desc.height = height;
	// Copy source: suites and captures read the bloom back.
	desc.usages = {
	    ResourceUsage::kColorAttachment, ResourceUsage::kSampled, ResourceUsage::kCopySource };
	desc.debugName = "render.pass.post bloom";
	for ( TextureId &image : m_Images )
	{
		auto made = m_Device.CreateTexture( desc );
		if ( !made )
			return false;
		image = made.Value();
	}
	m_Width = width;
	m_Height = height;
	return true;
}

bool BloomRenderer::Step( CommandEncoder &encoder, std::uint32_t mode, TextureId source,
    TextureId target, std::uint32_t sourceWidth, std::uint32_t sourceHeight,
    const BloomParams &params )
{
	const BindGroupEntry entries[] = {
	    { 0, {}, 0, 0, source, {} }, { 1, {}, 0, 0, {}, m_Sampler } };
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	{
		std::lock_guard<std::mutex> lock( m_PendingLock );
		if ( !group )
		{
			++m_RecordFailures;
			return false;
		}
		m_Pending.push_back( group.Value() );
	}
	Constants constants;
	std::copy( params.tint, params.tint + 4, constants.tint );
	constants.params[0] = params.exposure;
	constants.params[1] = params.bloomAmount;
	constants.params[2] = 1.0f / float( sourceWidth );
	constants.params[3] = 1.0f / float( sourceHeight );
	constants.modes[0] = mode;
	encoder.TransitionTexture( target, ResourceUsage::kUndefined, ResourceUsage::kColorAttachment );
	ColorAttachment color;
	color.texture = target;
	color.load = LoadOp::kDiscard;
	const ColorAttachment colors[] = { color };
	RenderingDesc rendering;
	rendering.colors = colors;
	rendering.width = m_Width;
	rendering.height = m_Height;
	encoder.BeginRendering( rendering );
	encoder.SetViewport( { 0.0f, 0.0f, float( m_Width ), float( m_Height ), 0.0f, 1.0f } );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.SetDrawConstants( 0, std::as_bytes( std::span<const Constants>( &constants, 1 ) ) );
	encoder.Draw( 3 );
	encoder.EndRendering();
	encoder.TransitionTexture( target, ResourceUsage::kColorAttachment, ResourceUsage::kSampled );
	return true;
}

foundation::Expected<TextureId, PostStatus> BloomRenderer::Record(
    CommandEncoder &encoder, const BloomSource &source, const BloomParams &params )
{
	if ( !source.scene.IsValid() || source.width == 0 || source.height == 0 )
		return foundation::MakeUnexpected( PostStatus::kInvalidTarget );
	if ( !Finite( params.exposure ) || params.exposure < 0.0f || !Finite( params.bloomAmount ) ||
	     params.bloomAmount < 0.0f ||
	     !std::all_of(
	         params.tint, params.tint + 4,
	         []( float v )
	         {
		         return Finite( v );
	         } ) )
		return foundation::MakeUnexpected( PostStatus::kInvalidParams );
	if ( !EnsureImages( std::max( 1u, source.width / 4 ), std::max( 1u, source.height / 4 ) ) )
		return foundation::MakeUnexpected( PostStatus::kDevice );
	if ( source.sceneUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( source.scene, source.sceneUsage, ResourceUsage::kSampled );
	bool drawn = Step(
	    encoder, kModeDownsample, source.scene, m_Images[0], source.width, source.height, params );
	// Both blurs step by the quarter image's width (BlurFilterY's quirk).
	drawn =
	    drawn && Step( encoder, kModeBlurX, m_Images[0], m_Images[1], m_Width, m_Height, params );
	drawn =
	    drawn && Step( encoder, kModeBlurY, m_Images[1], m_Images[0], m_Width, m_Height, params );
	if ( source.sceneUsage != ResourceUsage::kSampled )
		encoder.TransitionTexture( source.scene, ResourceUsage::kSampled, source.sceneUsage );
	if ( !drawn )
		return foundation::MakeUnexpected( PostStatus::kDevice );
	return m_Images[0];
}

void BloomRenderer::Collect( CompletionToken token )
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	for ( BindGroupId group : m_Pending )
		(void)m_Device.Release( group, token );
	m_Pending.clear();
	for ( TextureId image : m_Retired )
		(void)m_Device.Release( ResourceId( image ), token );
	m_Retired.clear();
	m_LastToken = token;
}

std::uint32_t BloomRenderer::RecordFailures() const
{
	std::lock_guard<std::mutex> lock( m_PendingLock );
	return m_RecordFailures;
}

} // namespace render::pass::post
