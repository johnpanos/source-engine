//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite `bounce` (RFC 0016 K11, render.pass.bounce): the
//			projected lights' one bounce against an independent oracle.
//
//			Scene: a floor at z = 0 of albedo (0.8, 0.5, 0.3), a white
//			projector 256 units above it looking down (60 degrees, Portal's
//			attenuation 0, 100, 0), and a 3 x 3 x 2 probe grid above the floor.
//			The reflective shadow map is synthesized (each texel's ray meets
//			the floor analytically), so the pass is judged alone, not the
//			surface program that draws a real one.
//
//			Oracle: the bounce's definition as an area integral over the lit
//			floor (a 600 x 600 grid of cells over the projector's footprint):
//			per probe texel direction n, the sum of albedo x the projector's
//			light at the cell (projected_light::IrradianceAt: color x
//			attenuation x its cosine) x cos at the floor x cos at the probe /
//			(pi r^2) x the cell's area, r at least a quarter of the spacing, a
//			probe's visibility by its depth moments (Chebyshev) - no code
//			shared with the shader, whose light is per map texel over its solid
//			angle.
//
//			Checks:
//			- interior: every interior texel of every probe within 3 percent
//			  (+ 1e-5 of the largest value) of the oracle;
//			- border: each border texel equals its interior twin;
//			- outside: texels outside the layer-0 tiles are zero;
//			- occluded: a probe whose moments put every surface within 30
//			  units (the floor is 64 away) gathers nothing, as the oracle;
//			- lit: the gathered light is not zero where the oracle's is not.
//			Seeded programs (--seeded): the patch's cosine ignored, the
//			visibility ignored, the solid angle flat; each fails a check.
//
//=============================================================================//

#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "mapcontainer/probe_volume.h"
#include "render/device/device.h"
#include "render/math/matrix.h"
#include "render/pass/bounce/bounce.h"
#include "render/pass/shadows/shadow_views.h"
#include "render/projected_light.h"
#include "spv/bounce_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;

constexpr float kPi = 3.14159265358979323846f;
constexpr float kAlbedo[3] = { 0.8f, 0.5f, 0.3f };
constexpr std::uint32_t kMapSize = 128;
constexpr std::uint32_t kDims[3] = { 3, 3, 2 };
constexpr float kOrigin[3] = { -128.0f, -128.0f, 64.0f };
constexpr float kSpacing[3] = { 128.0f, 128.0f, 96.0f };
constexpr std::uint32_t kTilesPerRow = 6;
constexpr float kMaxDistance = 4096.0f;
// The probe whose moments hide the floor.
constexpr std::uint32_t kOccluded = 4;

std::uint32_t ProbeCount()
{
	return kDims[0] * kDims[1] * kDims[2];
}

mapcontainer::ProbeVolumeLayout Layout( std::uint32_t &width, std::uint32_t &height )
{
	mapcontainer::ProbeVolumeLayout layout{};
	layout.gridCount = 1;
	layout.layerCount = 1;
	mapcontainer::ProbeGridLayout &grid = layout.grids[0];
	for ( int i = 0; i < 3; ++i )
	{
		grid.origin[i] = kOrigin[i];
		grid.spacing[i] = kSpacing[i];
		grid.dims[i] = kDims[i];
	}
	grid.probeCount = ProbeCount();
	grid.tilesPerRow = kTilesPerRow;
	grid.maxDistance = kMaxDistance;
	const std::uint32_t rows = ( ProbeCount() + kTilesPerRow - 1 ) / kTilesPerRow;
	grid.irradianceOrigin[0][0] = 0;
	grid.irradianceOrigin[0][1] = 0;
	grid.visibilityOrigin[0] = 0;
	grid.visibilityOrigin[1] = rows * 8;
	grid.stateOrigin[0] = 0;
	grid.stateOrigin[1] = rows * 8 + rows * 16;
	width = kTilesPerRow * 16;
	height = grid.stateOrigin[1] + 1;
	return layout;
}

projected_light::Light Projector()
{
	projected_light::Light light;
	light.origin[2] = 256.0f;
	const float forward[3] = { 0, 0, -1 }, right[3] = { 0, -1, 0 }, up[3] = { 1, 0, 0 };
	std::memcpy( light.forward, forward, sizeof( forward ) );
	std::memcpy( light.right, right, sizeof( right ) );
	std::memcpy( light.up, up, sizeof( up ) );
	light.horizontalFovDegrees = light.verticalFovDegrees = 60.0f;
	light.nearZ = 4.0f;
	light.farZ = 1000.0f;
	light.color[0] = light.color[1] = light.color[2] = 2.0f;
	return light;
}

void ProbePosition( std::uint32_t probe, float out[3] )
{
	const std::uint32_t i[3] = { probe % kDims[0], ( probe / kDims[0] ) % kDims[1],
	    probe / ( kDims[0] * kDims[1] ) };
	for ( int k = 0; k < 3; ++k )
		out[k] = kOrigin[k] + float( i[k] ) * kSpacing[k];
}

// probe_volume.py oct_decode of an interior texel's centre.
void Direction( int ix, int iy, int n, float out[3] )
{
	const float u = ( float( ix ) + 0.5f ) / float( n ) * 2.0f - 1.0f;
	const float v = ( float( iy ) + 0.5f ) / float( n ) * 2.0f - 1.0f;
	float x = u, y = v;
	const float z = 1.0f - std::fabs( u ) - std::fabs( v );
	if ( z < 0.0f )
	{
		x = ( 1.0f - std::fabs( v ) ) * ( u >= 0.0f ? 1.0f : -1.0f );
		y = ( 1.0f - std::fabs( u ) ) * ( v >= 0.0f ? 1.0f : -1.0f );
	}
	const float length = std::sqrt( x * x + y * y + z * z );
	out[0] = x / length;
	out[1] = y / length;
	out[2] = z / length;
}

// The oracle's value for one probe and interior direction.
void Oracle( std::uint32_t probe, const float n[3], float out[3] )
{
	out[0] = out[1] = out[2] = 0.0f;
	if ( probe == kOccluded )
		return; // every surface is beyond its moments' mean, variance zero
	const projected_light::Light light = Projector();
	float p[3];
	ProbePosition( probe, p );
	const float extent = 256.0f * std::tan( 30.0f * kPi / 180.0f ) * 1.01f;
	const int cells = 600;
	const double area = double( 2.0f * extent / cells ) * double( 2.0f * extent / cells );
	const float nearest = 0.25f * std::min( { kSpacing[0], kSpacing[1], kSpacing[2] } );
	const float up[3] = { 0, 0, 1 };
	double sum[3] = {};
	for ( int j = 0; j < cells; ++j )
	{
		for ( int i = 0; i < cells; ++i )
		{
			const float x[3] = { -extent + ( float( i ) + 0.5f ) * 2.0f * extent / cells,
			    -extent + ( float( j ) + 0.5f ) * 2.0f * extent / cells, 0.0f };
			float light3[3];
			projected_light::IrradianceAt(
			    light, x, up,
			    []( float, float, float rgb[3] )
			    {
				    rgb[0] = rgb[1] = rgb[2] = 1.0f;
			    },
			    light3 );
			if ( light3[0] <= 0.0f )
				continue;
			const float d[3] = { x[0] - p[0], x[1] - p[1], x[2] - p[2] };
			const float r2raw = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
			const float r = std::sqrt( r2raw );
			const float w[3] = { d[0] / r, d[1] / r, d[2] / r };
			const float atProbe = n[0] * w[0] + n[1] * w[1] + n[2] * w[2];
			const float atFloor = -w[2];
			if ( atProbe <= 0.0f || atFloor <= 0.0f )
				continue;
			const double r2 = std::max( r2raw, nearest * nearest );
			const double scale = double( atProbe ) * atFloor / ( double( kPi ) * r2 ) * area;
			for ( int c = 0; c < 3; ++c )
				sum[c] += double( kAlbedo[c] ) * light3[c] * scale;
		}
	}
	for ( int c = 0; c < 3; ++c )
		out[c] = float( sum[c] );
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counted{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counted, device ) )
		return why;
	std::uint32_t width = 0, height = 0;
	const mapcontainer::ProbeVolumeLayout layout = Layout( width, height );
	const mapcontainer::ProbeGridLayout &grid = layout.grids[0];

	// The probe atlas: visibility moments (a mean at the maximum distance,
	// the occluded probe's at 30 units) and every probe active.
	std::vector<std::uint16_t> atlas( std::size_t( width ) * height * 4, 0 );
	auto texel = [&]( std::uint32_t x, std::uint32_t y, float r, float g, float b, float a )
	{
		std::uint16_t *t = &atlas[( std::size_t( y ) * width + x ) * 4];
		t[0] = FloatToHalf( r );
		t[1] = FloatToHalf( g );
		t[2] = FloatToHalf( b );
		t[3] = FloatToHalf( a );
	};
	for ( std::uint32_t probe = 0; probe < ProbeCount(); ++probe )
	{
		const std::uint32_t tx = grid.visibilityOrigin[0] + ( probe % kTilesPerRow ) * 16;
		const std::uint32_t ty = grid.visibilityOrigin[1] + ( probe / kTilesPerRow ) * 16;
		const float mean = probe == kOccluded ? 30.0f / kMaxDistance : 1.0f;
		for ( std::uint32_t y = 0; y < 16; ++y )
			for ( std::uint32_t x = 0; x < 16; ++x )
				texel( tx + x, ty + y, mean, mean * mean, 0, 0 );
		texel( grid.stateOrigin[0] + probe, grid.stateOrigin[1], 0, 0, 0, 1 );
	}
	std::vector<float> table( mapcontainer::kProbeGridTableFloats );
	mapcontainer::WriteProbeGridTable( layout, table.data() );

	// The reflective shadow map: each texel's ray through the frustum meets
	// the floor.
	const projected_light::Light light = Projector();
	pass::shadows::FlashlightShadowDesc desc;
	desc.position = { light.origin[0], light.origin[1], light.origin[2] };
	desc.forward = { light.forward[0], light.forward[1], light.forward[2] };
	desc.up = { light.up[0], light.up[1], light.up[2] };
	desc.horizontalFovRadians = desc.verticalFovRadians = light.horizontalFovDegrees * kPi / 180.0f;
	desc.nearZ = light.nearZ;
	desc.farZ = light.farZ;
	auto view = pass::shadows::BuildFlashlightShadowView( desc );
	if ( !view )
		return std::string( "the projector's view does not build" );
	const math::float4x4 toClip = view.Value().viewProjection;
	const auto fromClip = math::Inverse( toClip );
	if ( !fromClip )
		return std::string( "the projector's view does not invert" );
	std::vector<float> depth( std::size_t( kMapSize ) * kMapSize, 1.0f );
	std::vector<std::uint16_t> albedo( std::size_t( kMapSize ) * kMapSize * 4, 0 );
	for ( std::uint32_t y = 0; y < kMapSize; ++y )
	{
		for ( std::uint32_t x = 0; x < kMapSize; ++x )
		{
			const float ndcX = ( float( x ) + 0.5f ) / kMapSize * 2.0f - 1.0f;
			const float ndcY = 1.0f - ( float( y ) + 0.5f ) / kMapSize * 2.0f;
			const math::float4 nearH = math::Transform( *fromClip, { ndcX, ndcY, 0.0f, 1.0f } );
			const math::float4 farH = math::Transform( *fromClip, { ndcX, ndcY, 1.0f, 1.0f } );
			const math::float3 a{ nearH.x / nearH.w, nearH.y / nearH.w, nearH.z / nearH.w };
			const math::float3 b{ farH.x / farH.w, farH.y / farH.w, farH.z / farH.w };
			if ( !( a.z > 0.0f && b.z < 0.0f ) )
				continue;
			const float t = a.z / ( a.z - b.z );
			const math::float4 hit{ a.x + ( b.x - a.x ) * t, a.y + ( b.y - a.y ) * t, 0.0f, 1.0f };
			const math::float4 clip = math::Transform( toClip, hit );
			const std::size_t i = std::size_t( y ) * kMapSize + x;
			depth[i] = clip.z / clip.w;
			for ( int c = 0; c < 3; ++c )
				albedo[i * 4 + std::size_t( c )] = FloatToHalf( kAlbedo[c] );
			albedo[i * 4 + 3] = FloatToHalf( 1.0f );
		}
	}

	// Device resources.
	auto texture = [&]( Format format, std::uint32_t w, std::uint32_t h, std::uint32_t layers,
	                   std::initializer_list<ResourceUsage> usages )
	{
		TextureDesc d;
		d.format = format;
		d.width = w;
		d.height = h;
		d.depthOrLayers = layers;
		d.usages = UsageSet( usages );
		auto made = device->CreateTexture( d );
		return std::make_pair( made ? made.Value() : TextureId(), d );
	};
	const auto [atlasTexture, atlasDesc] = texture( Format::kRGBA16Float, width, height, 1,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kSampled } );
	const auto [gridTexture, gridDesc] = texture( Format::kRGBA32Float,
	    mapcontainer::kProbeGridTableTexels, 1, 1,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kSampled } );
	const auto [albedoTexture, albedoDesc] = texture( Format::kRGBA16Float, kMapSize, kMapSize, 1,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kSampled } );
	const auto [depthTexture, depthDesc] = texture( Format::kD32Float, kMapSize, kMapSize, 1,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kSampled } );
	const auto [cookieTexture, cookieDesc] = texture( Format::kRGBA8Unorm, 1, 1, 2,
	    { ResourceUsage::kCopyDestination, ResourceUsage::kSampled } );
	const auto [output, outputDesc] = texture( Format::kRGBA16Float, width, height, 1,
	    { ResourceUsage::kStorageWrite, ResourceUsage::kSampled, ResourceUsage::kCopySource } );
	(void)atlasDesc;
	(void)gridDesc;
	(void)albedoDesc;
	(void)depthDesc;
	(void)cookieDesc;
	(void)outputDesc;
	BufferDesc readbackDesc;
	readbackDesc.size = std::uint64_t( width ) * height * 8;
	readbackDesc.usages = { ResourceUsage::kCopyDestination };
	readbackDesc.memory = MemoryKind::kReadback;
	auto readback = device->CreateBuffer( readbackDesc );
	if ( !atlasTexture.IsValid() || !gridTexture.IsValid() || !albedoTexture.IsValid() ||
	     !depthTexture.IsValid() || !cookieTexture.IsValid() || !output.IsValid() || !readback )
		return std::string( "a texture or buffer was refused" );
	auto created = module.empty() ? pass::bounce::ProjectorBounce::Create( *device )
	                              : pass::bounce::ProjectorBounce::CreateWithProgram( *device, module );
	if ( !created )
		return std::string( "the bounce pass was refused" );
	std::unique_ptr<pass::bounce::ProjectorBounce> bouncer = std::move( created ).Value();

	auto encoded = device->BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return std::string( "no encoder" );
	CommandEncoder &encoder = encoded.Value();
	std::vector<BufferId> staging;
	auto upload = [&]( TextureId id, std::span<const std::byte> bytes, std::uint32_t w,
	                  std::uint32_t h, std::uint32_t layers )
	{
		BufferDesc d;
		d.size = bytes.size();
		d.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
		auto buffer = device->CreateBuffer( d );
		if ( !buffer )
			return;
		staging.push_back( buffer.Value() );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( buffer.Value(), 0, bytes );
		encoder.TransitionBuffer(
		    buffer.Value(), ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		encoder.TransitionTexture( id, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		for ( std::uint32_t layer = 0; layer < layers; ++layer )
			encoder.CopyBufferToTexture(
			    buffer.Value(), id, { bytes.size() / layers * layer, 0, layer, w, h } );
		encoder.TransitionTexture( id, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	};
	upload( atlasTexture, std::as_bytes( std::span( atlas ) ), width, height, 1 );
	upload( gridTexture, std::as_bytes( std::span( table ) ), mapcontainer::kProbeGridTableTexels,
	    1, 1 );
	upload( albedoTexture, std::as_bytes( std::span( albedo ) ), kMapSize, kMapSize, 1 );
	upload( depthTexture, std::as_bytes( std::span( depth ) ), kMapSize, kMapSize, 1 );
	const std::uint8_t white[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };
	upload( cookieTexture, std::as_bytes( std::span( white ) ), 1, 1, 2 );
	encoder.TransitionTexture( output, ResourceUsage::kUndefined, ResourceUsage::kSampled );
	pass::bounce::ReflectiveShadowMap map;
	map.albedo = albedoTexture;
	map.depth = depthTexture;
	map.size = kMapSize;
	map.viewProjection = toClip;
	map.light = projected_light::PackLightGpu( light, 0 );
	pass::bounce::BounceInputs inputs;
	inputs.probeAtlas = atlasTexture;
	inputs.probeAtlasDesc = atlasDesc;
	inputs.probeGrids = gridTexture;
	inputs.gridCount = 1;
	inputs.maxProbesPerGrid = ProbeCount();
	inputs.cookies = cookieTexture;
	inputs.maps = std::span( &map, 1 );
	inputs.output = output;
	inputs.outputUsage = ResourceUsage::kSampled;
	if ( !bouncer->Record( encoder, inputs ) )
		return std::string( "the bounce pass did not record" );
	encoder.TransitionTexture( output, ResourceUsage::kSampled, ResourceUsage::kCopySource );
	encoder.TransitionBuffer(
	    readback.Value(), ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.CopyTextureToBuffer( output, readback.Value(), { 0, 0, 0, width, height } );
	auto token = device->Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !token )
		return std::string( "the submission was refused" );
	(void)device->WaitIdle();
	bouncer->Collect( token.Value() );
	std::vector<std::byte> bytes( std::size_t( width ) * height * 8 );
	if ( !device->ReadBuffer( readback.Value(), 0, bytes ) )
		return std::string( "the atlas did not read back" );
	auto read = [&]( std::uint32_t x, std::uint32_t y, float out[3] )
	{
		for ( int c = 0; c < 3; ++c )
		{
			std::uint16_t half;
			std::memcpy( &half, bytes.data() + ( std::size_t( y ) * width + x ) * 8 + c * 2, 2 );
			out[c] = HalfToFloat( half );
		}
	};

	// The oracle at every interior texel, then the judgement.
	float largest = 0.0f;
	std::vector<float> expected( std::size_t( ProbeCount() ) * 36 * 3 );
	for ( std::uint32_t probe = 0; probe < ProbeCount(); ++probe )
		for ( int iy = 0; iy < 6; ++iy )
			for ( int ix = 0; ix < 6; ++ix )
			{
				float n[3];
				Direction( ix, iy, 6, n );
				float *e = &expected[( std::size_t( probe ) * 36 + std::size_t( iy * 6 + ix ) ) * 3];
				Oracle( probe, n, e );
				largest = std::max( { largest, e[0], e[1], e[2] } );
			}
	std::uint32_t interiorBad = 0, borderBad = 0, occludedBad = 0, litMissing = 0;
	std::string firstBad;
	for ( std::uint32_t probe = 0; probe < ProbeCount(); ++probe )
	{
		const std::uint32_t cx = ( probe % kTilesPerRow ) * 8;
		const std::uint32_t cy = ( probe / kTilesPerRow ) * 8;
		for ( int iy = 0; iy < 6; ++iy )
			for ( int ix = 0; ix < 6; ++ix )
			{
				const float *e =
				    &expected[( std::size_t( probe ) * 36 + std::size_t( iy * 6 + ix ) ) * 3];
				float a[3];
				read( cx + std::uint32_t( ix ) + 1, cy + std::uint32_t( iy ) + 1, a );
				for ( int c = 0; c < 3; ++c )
				{
					const bool near = std::fabs( a[c] - e[c] ) <= 0.03f * e[c] + 1e-5f * largest;
					if ( probe == kOccluded )
						occludedBad += a[c] != 0.0f;
					else if ( !near )
					{
						++interiorBad;
						if ( firstBad.empty() )
						{
							char text[200];
							std::snprintf( text, sizeof( text ),
							    "probe %u texel %d,%d channel %d: expected %.6g, got %.6g", probe, ix,
							    iy, c, double( e[c] ), double( a[c] ) );
							firstBad = text;
						}
					}
					if ( e[c] > 1e-3f * largest && !( a[c] > 0.0f ) )
						++litMissing;
				}
			}
		// Each border texel equals its interior twin (probe_volume.py).
		const int n = 6;
		for ( int y = 0; y < n + 2; ++y )
			for ( int x = 0; x < n + 2; ++x )
			{
				if ( x >= 1 && x <= n && y >= 1 && y <= n )
					continue;
				int ix = x - 1, iy = y - 1;
				if ( y == 0 || y == n + 1 )
				{
					iy = y == 0 ? 0 : n - 1;
					ix = x == 0 ? n - 1 : ( x == n + 1 ? 0 : n - x );
					if ( x == 0 || x == n + 1 )
						iy = y == 0 ? n - 1 : 0;
				}
				else
				{
					ix = x == 0 ? 0 : n - 1;
					iy = n - y;
				}
				float border[3], twin[3];
				read( cx + std::uint32_t( x ), cy + std::uint32_t( y ), border );
				read( cx + std::uint32_t( ix ) + 1, cy + std::uint32_t( iy ) + 1, twin );
				for ( int c = 0; c < 3; ++c )
					borderBad += border[c] != twin[c];
			}
	}
	std::uint32_t outsideBad = 0;
	const std::uint32_t tileRows = ( ProbeCount() + kTilesPerRow - 1 ) / kTilesPerRow;
	for ( std::uint32_t y = 0; y < height; ++y )
		for ( std::uint32_t x = 0; x < width; ++x )
		{
			const bool inTile = y < tileRows * 8 && x < kTilesPerRow * 8 &&
			                    ( y / 8 ) * kTilesPerRow + ( x / 8 ) < ProbeCount();
			if ( inTile )
				continue;
			float a[3];
			read( x, y, a );
			outsideBad += a[0] != 0.0f || a[1] != 0.0f || a[2] != 0.0f;
		}
	char detail[160];
	std::snprintf( detail, sizeof( detail ), "%u of %u channel values outside; largest %.4g; %s",
	    interiorBad, ( ProbeCount() - 1 ) * 36 * 3, double( largest ), firstBad.c_str() );
	results.That( interiorBad == 0, "bounce.interior", detail );
	results.That( borderBad == 0, "bounce.border", std::to_string( borderBad ) + " differ" );
	results.That( outsideBad == 0, "bounce.outside", std::to_string( outsideBad ) + " non-zero" );
	results.That( occludedBad == 0, "bounce.occluded", std::to_string( occludedBad ) + " non-zero" );
	results.That( litMissing == 0 && largest > 0.0f, "bounce.lit",
	    std::to_string( litMissing ) + " lit values missing" );
	std::printf( "INFO bounce largest %.5g, oracle cells 600 x 600, map %u x %u\n",
	    double( largest ), kMapSize, kMapSize );

	for ( TextureId id : { atlasTexture, gridTexture, albedoTexture, depthTexture, cookieTexture,
	          output } )
		(void)device->Release( id, token.Value() );
	(void)device->Release( readback.Value(), token.Value() );
	for ( BufferId id : staging )
		(void)device->Release( id, token.Value() );
	(void)device->WaitIdle();
	bouncer.reset();
	device.reset();
	messages = counted.load();
	return std::nullopt;
}

const Seeded kBounceSeeded[] = {
    { "emit-ignored", spirv::kBounceEmitIgnored, "bounce.interior" },
    { "visibility-ignored", spirv::kBounceVisibilityIgnored, "bounce.occluded" },
    { "flat-solid-angle", spirv::kBounceFlatSolidAngle, "bounce.interior" } };

} // namespace

int RunBounceSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "bounce", kBounceSeeded, RunOnce );
}

} // namespace render::lab
