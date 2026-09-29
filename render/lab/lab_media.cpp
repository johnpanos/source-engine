//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's participating media (RFC 0016 K11 step g); see
//			lab_media.h.
//
//=============================================================================//

#include "lab_media.h"

#include "render/light_set.h"
#include "render/projected_light.h"
#include "texturecontainer/texture_image.h"
#include "texturecontainer/vtf_image_reader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <span>

namespace render::lab
{

namespace
{

using namespace render::device;
namespace volumetric = render::pass::volumetric;

constexpr double kPi = 3.14159265358979323846;

// The values of a key as floats ( "a b c" ), padded with `fill`.
std::vector<float> Numbers(
    const Entity &entity, const std::string &key, std::size_t count, float fill = 0.0f )
{
	std::vector<float> values;
	const auto found = entity.find( key );
	if ( found != entity.end() )
	{
		const char *at = found->second.c_str();
		char *end = nullptr;
		for ( ;; )
		{
			const float value = std::strtof( at, &end );
			if ( end == at )
				break;
			values.push_back( value );
			at = end;
		}
	}
	values.resize( std::max( values.size(), count ), fill );
	return values;
}

bool Has( const Entity &entity, const std::string &key )
{
	return entity.count( key ) != 0;
}

std::string Value( const Entity &entity, const std::string &key )
{
	const auto found = entity.find( key );
	return found == entity.end() ? std::string() : found->second;
}

math::float3 Vector3( const std::vector<float> &v )
{
	return { v[0], v[1], v[2] };
}

// GammaToLinear( rgb / 255 ) times a brightness / 255.
math::float3 GammaColor( const std::vector<float> &light )
{
	const auto channel = []( float c )
	{
		return float( std::pow( double( c ) / 255.0, 2.2 ) );
	};
	const float scale = light[3] / 255.0f;
	return {
	    channel( light[0] ) * scale, channel( light[1] ) * scale, channel( light[2] ) * scale };
}

// The engine's AngleVectors (pitch positive looks down).
void AngleVectors(
    const std::vector<float> &angles, math::float3 &forward, math::float3 &right, math::float3 &up )
{
	const double p = angles[0] * kPi / 180.0;
	const double y = angles[1] * kPi / 180.0;
	const double r = angles[2] * kPi / 180.0;
	const double sp = std::sin( p ), cp = std::cos( p );
	const double sy = std::sin( y ), cy = std::cos( y );
	const double sr = std::sin( r ), cr = std::cos( r );
	forward = { float( cp * cy ), float( cp * sy ), float( -sp ) };
	right = {
	    float( -sr * sp * cy + cr * sy ), float( -sr * sp * sy - cr * cy ), float( -sr * cp ) };
	up = { float( cr * sp * cy + sr * sy ), float( cr * sp * sy - sr * cy ), float( cr * cp ) };
}

} // namespace

std::optional<std::vector<Entity>> ParseEntityLump( const std::string &text )
{
	std::vector<Entity> entities;
	std::size_t at = 0;
	const auto skipSpace = [&]()
	{
		while ( at < text.size() &&
		        ( std::isspace( static_cast<unsigned char>( text[at] ) ) || text[at] == '\0' ) )
			++at;
	};
	const auto quoted = [&]( std::string &out ) -> bool
	{
		skipSpace();
		if ( at >= text.size() || text[at] != '"' )
			return false;
		const std::size_t end = text.find( '"', at + 1 );
		if ( end == std::string::npos )
			return false;
		out = text.substr( at + 1, end - at - 1 );
		at = end + 1;
		return true;
	};
	for ( ;; )
	{
		skipSpace();
		if ( at >= text.size() )
			return entities;
		if ( text[at] != '{' )
			return std::nullopt;
		++at;
		Entity entity;
		for ( ;; )
		{
			skipSpace();
			if ( at < text.size() && text[at] == '}' )
			{
				++at;
				break;
			}
			std::string key, value;
			if ( !quoted( key ) || !quoted( value ) )
				return std::nullopt;
			entity.emplace( key, value ); // the first of a repeated key wins
		}
		entities.push_back( std::move( entity ) );
	}
}

LabMedia MediaFromEntities( const std::vector<Entity> &entities )
{
	LabMedia media;
	for ( const Entity &entity : entities )
	{
		const std::string classname = Value( entity, "classname" );
		const math::float3 origin = Vector3( Numbers( entity, "origin", 3 ) );
		if ( classname == "env_volumetric_fog_volume" )
		{
			media.present = true;
			volumetric::FogVolume volume;
			const math::float3 lo = Vector3( Numbers( entity, "box_mins", 3 ) );
			const math::float3 hi = Vector3( Numbers( entity, "box_maxs", 3 ) );
			volume.mins = origin + lo;
			volume.maxs = origin + hi;
			volume.extinction = Numbers( entity, "density", 1 )[0];
			volume.albedo = Numbers( entity, "albedo", 1, 1.0f )[0];
			volume.anisotropy = Numbers( entity, "anisotropy", 1 )[0];
			volume.emission = Vector3( Numbers( entity, "emission", 3 ) );
			media.medium.volumes.push_back( volume );
		}
		else if ( classname == "env_volumetric_fog_controller" )
		{
			media.present = true;
			volumetric::HeightFog &fog = media.medium.fog;
			fog.density = Numbers( entity, "density", 1 )[0];
			fog.heightDensity = Numbers( entity, "height_fog_density", 1 )[0];
			fog.heightFalloff = Numbers( entity, "height_fog_falloff", 1 )[0];
			fog.baseHeight = Has( entity, "origin" ) ? origin.z : 0.0f;
			fog.albedo = Numbers( entity, "albedo", 1, 1.0f )[0];
			fog.anisotropy = Numbers( entity, "anisotropy", 1 )[0];
		}
		else if ( classname == "light" || classname == "light_spot" )
		{
			const std::vector<float> attn = { Numbers( entity, "_constant_attn", 1 )[0],
			    Numbers( entity, "_linear_attn", 1 )[0],
			    Numbers( entity, "_quadratic_attn", 1 )[0] };
			if ( !( attn[0] == 0.0f && attn[1] == 0.0f && attn[2] == 1.0f ) )
			{
				++media.unsupportedLights;
				continue;
			}
			volumetric::MediumLight light;
			light.falloff = light_set::LightFalloff::InverseSquare;
			light.position = origin;
			light.color = GammaColor( Numbers( entity, "_light", 4 ) );
			if ( classname == "light_spot" )
			{
				const std::vector<float> angles = Numbers( entity, "angles", 3 );
				const float pitch =
				    Has( entity, "pitch" ) ? Numbers( entity, "pitch", 1 )[0] : angles[0];
				const double p = pitch * kPi / 180.0;
				const double y = angles[1] * kPi / 180.0;
				light.direction = { float( std::cos( y ) * std::cos( p ) ),
				    float( std::sin( y ) * std::cos( p ) ), float( std::sin( p ) ) };
				float inner = Numbers( entity, "_inner_cone", 1, 10.0f )[0];
				float outer = Numbers( entity, "_cone", 1, 0.0f )[0];
				if ( outer == 0.0f )
					outer = inner;
				outer = std::max( outer, inner );
				if ( !( inner == 180.0f && outer == 180.0f ) )
				{
					light.kind = volumetric::MediumLightKind::kSpot;
					light.innerCos = float( std::cos( std::min( inner, 90.0f ) * kPi / 180.0 ) );
					light.outerCos = float( std::cos( std::min( outer, 90.0f ) * kPi / 180.0 ) );
					light.exponent = Numbers( entity, "_exponent", 1 )[0];
				}
			}
			media.lights.push_back( light );
		}
		else if ( classname == "env_projectedtexture" )
		{
			volumetric::MediumProjector projector;
			projected_light::Light &light = projector.light;
			math::float3 forward, right, up;
			AngleVectors( Numbers( entity, "angles", 3 ), forward, right, up );
			const float axes[3][3] = { { forward.x, forward.y, forward.z },
			    { right.x, right.y, right.z }, { up.x, up.y, up.z } };
			std::memcpy( light.forward, axes[0], sizeof( light.forward ) );
			std::memcpy( light.right, axes[1], sizeof( light.right ) );
			std::memcpy( light.up, axes[2], sizeof( light.up ) );
			light.origin[0] = origin.x;
			light.origin[1] = origin.y;
			light.origin[2] = origin.z;
			light.horizontalFovDegrees = light.verticalFovDegrees =
			    Numbers( entity, "lightfov", 1, 90.0f )[0];
			light.nearZ = Numbers( entity, "nearz", 1, 4.0f )[0];
			light.farZ = Numbers( entity, "farz", 1, 750.0f )[0];
			const math::float3 color = GammaColor( Numbers( entity, "lightcolor", 4 ) );
			light.color[0] = color.x;
			light.color[1] = color.y;
			light.color[2] = color.z;
			light.shadows = Value( entity, "enableshadows" ) != "0";
			light.lightsWorld = Value( entity, "lightworld" ) != "0";
			projector.cookieLayer = std::uint32_t( media.projectors.size() );
			media.projectors.push_back( projector );
			media.cookieNames.push_back( Value( entity, "texturename" ) );
		}
	}
	return media;
}

pass::volumetric::FroxelLayout FroxelLayoutOf( const pass::lights::ClusterGrid &grid )
{
	pass::volumetric::FroxelLayout layout;
	layout.tilesX = grid.tilesX;
	layout.tilesY = grid.tilesY;
	layout.slices = grid.slices;
	layout.tileSizePixels = grid.limits.tileSizePixels;
	layout.widthPixels = grid.widthPixels;
	layout.heightPixels = grid.heightPixels;
	layout.sliceScale = grid.sliceScale;
	layout.sliceBias = grid.sliceBias;
	layout.nearZ = grid.nearZ;
	layout.farZ = grid.farZ;
	layout.sliceDepths = grid.sliceDepths;
	layout.view = grid.view;
	layout.rayTopLeft = grid.cornerRays.front();
	layout.rayBottomRight = grid.cornerRays.back();
	return layout;
}

CookieArray::~CookieArray()
{
	if ( !m_Device )
		return;
	(void)m_Device->WaitIdle();
	if ( m_Texture.IsValid() )
		(void)m_Device->Release( m_Texture, CompletionToken() );
	if ( m_Staging.IsValid() )
		(void)m_Device->Release( m_Staging, CompletionToken() );
}

std::optional<std::string> CookieArray::Create(
    IRenderDevice2 &device, const GameFiles &files, const std::vector<std::string> &names )
{
	m_Device = &device;
	std::vector<texturecontainer::TextureImage> images;
	for ( const std::string &name : names )
	{
		std::string bytes;
		if ( !files.Read( "materials/" + name + ".vtf", bytes ) )
			return "cookie " + name + " is missing";
		auto image = texturecontainer::ReadVtfImage(
		    std::as_bytes( std::span( bytes.data(), bytes.size() ) ) );
		if ( !image || image.Value().levels.empty() )
			return "cookie " + name + " does not decode";
		const texturecontainer::PixelFormat format = image.Value().format;
		if ( format != texturecontainer::PixelFormat::Rgba8Unorm &&
		     format != texturecontainer::PixelFormat::Rgba8Srgb )
			return "cookie " + name + " is not 8-bit RGBA after decoding";
		images.push_back( std::move( image ).Value() );
	}
	m_Width = images.empty() ? 1 : images[0].levels[0].width;
	m_Height = images.empty() ? 1 : images[0].levels[0].height;
	m_Layers = std::max<std::uint32_t>( 2, std::uint32_t( images.size() ) + 1 );
	m_LayerBytes = std::uint64_t( m_Width ) * m_Height * 4;
	m_Bytes.assign( m_LayerBytes * m_Layers, std::byte( 255 ) );
	for ( std::size_t i = 0; i < images.size(); ++i )
	{
		const texturecontainer::ImageLevel &level = images[i].levels[0];
		if ( level.width != m_Width || level.height != m_Height ||
		     level.bytes.size() != m_LayerBytes )
			return "cookie " + names[i] + " differs in size from the first";
		std::memcpy( m_Bytes.data() + i * m_LayerBytes, level.bytes.data(), m_LayerBytes );
	}
	TextureDesc desc;
	desc.format = Format::kRGBA8Unorm; // the cookie is linear (pixel / 255)
	desc.width = m_Width;
	desc.height = m_Height;
	desc.depthOrLayers = m_Layers;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	desc.debugName = "render_lab.cookies";
	auto texture = device.CreateTexture( desc );
	BufferDesc staging;
	staging.size = m_Bytes.size();
	staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
	auto buffer = device.CreateBuffer( staging );
	if ( texture )
		m_Texture = texture.Value();
	if ( buffer )
		m_Staging = buffer.Value();
	if ( !texture || !buffer )
		return std::string( "the cookie array was refused" );
	return std::nullopt;
}

void CookieArray::RecordUpload( CommandEncoder &encoder )
{
	encoder.TransitionBuffer(
	    m_Staging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( m_Staging, 0, m_Bytes );
	encoder.TransitionBuffer(
	    m_Staging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
	encoder.TransitionTexture( m_Texture, ResourceUsage::kUndefined,
	    ResourceUsage::kCopyDestination, { 0, 1, 0, m_Layers } );
	for ( std::uint32_t layer = 0; layer < m_Layers; ++layer )
		encoder.CopyBufferToTexture(
		    m_Staging, m_Texture, { layer * m_LayerBytes, 0, layer, m_Width, m_Height } );
	encoder.TransitionTexture( m_Texture, ResourceUsage::kCopyDestination, ResourceUsage::kSampled,
	    { 0, 1, 0, m_Layers } );
}

} // namespace render::lab
