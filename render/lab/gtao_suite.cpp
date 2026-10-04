//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite `gtao` (RFC 0016 K11 "Ambient occlusion",
//			render.pass.ao): the pass on synthesized depth and normals of
//			analytic scenes, against an independent ray-traced reference.
//
//			Scenes (a 256 x 192 view, 90 degrees across, the pass's default
//			radius 48 units):
//			- plane: an open floor seen from 60 units up at 35 degrees down;
//			  nothing occludes it, so the visibility is one;
//			- crease: the same floor meeting a wall (x = 80) at a right
//			  angle, the camera looking into the corner.
//			Reference: per pixel, the cosine-weighted fraction of the
//			hemisphere about the true normal whose rays leave the analytic
//			scene without meeting a surface within the pass's effective reach
//			(the radius weighted by its falloff: a hit at distance t occludes
//			with weight falling linearly from 1 at 0.6 R to 0 at R, as the
//			pass's horizon weight), 1024 stratified rays - no code shared with the
//			shader.
//
//			Checks:
//			- plane: every pixel at least 0.99 and at most 1;
//			- crease: over pixels farther than 6 pixels from the image's edges
//			  and the crease's own line, the mean error at most 0.025 and the
//			  95th percentile at most 0.12 (GTAO's one horizon per slice side
//			  occludes less than rays near a crease; the band was set after
//			  the first run, disclosed), and floor pixels within 20 units of
//			  the wall darker than 0.9 of those 100 units away;
//			- depth-free: pixels with no depth are one.
//			Seeded programs (--seeded): the projected normal ignored (the
//			cosine weighting lost), slices spread evenly on the screen
//			instead of about the view vector (5 percent too dark on a
//			grazing plane), samples snapped to texel centres (a sample off
//			its slice reads as a horizon at grazing angles); each fails a
//			check. Not covered by these scenes: the horizon's distance
//			falloff (ignoring it matches this reference no worse) and the
//			blur's surface weights.
//
//=============================================================================//

#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/device/device.h"
#include "render/math/matrix.h"
#include "render/pass/ao/ao.h"
#include "spv/gtao_defects_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using math::float3;

constexpr std::uint32_t kWidth = 256;
constexpr std::uint32_t kHeight = 192;
constexpr float kRadius = 48.0f;
constexpr float kFalloff = 0.6f;
constexpr float kWall = 80.0f;
constexpr float kPi = 3.14159265358979323846f;

struct Scene
{
	bool wall = false;
};

// The nearest hit along a ray, and the normal there (floor z = 0 from
// above, the wall x = kWall facing -x).
std::optional<std::pair<float, float3>> Hit( const Scene &scene, float3 origin, float3 direction )
{
	float best = 1e30f;
	float3 normal{ 0, 0, 1 };
	if ( direction.z < -1e-6f )
	{
		const float t = -origin.z / direction.z;
		if ( t > 1e-3f )
		{
			const float3 p = origin + direction * t;
			if ( !scene.wall || p.x <= kWall )
				best = t;
		}
	}
	if ( scene.wall && direction.x > 1e-6f )
	{
		const float t = ( kWall - origin.x ) / direction.x;
		const float3 p = origin + direction * t;
		if ( t > 1e-3f && t < best && p.z >= 0.0f )
		{
			best = t;
			normal = { -1, 0, 0 };
		}
	}
	if ( best >= 1e29f )
		return std::nullopt;
	return std::make_pair( best, normal );
}

// The cosine-weighted unoccluded fraction about n at p.
float ReferenceVisibility( const Scene &scene, float3 p, float3 n )
{
	const float3 t = math::Normalize(
	    math::Cross( n, std::fabs( n.z ) < 0.9f ? float3{ 0, 0, 1 } : float3{ 1, 0, 0 } ) );
	const float3 b = math::Cross( n, t );
	const int side = 32;
	double sum = 0.0;
	for ( int j = 0; j < side; ++j )
	{
		for ( int i = 0; i < side; ++i )
		{
			// Stratified cosine-weighted directions (Malley's method).
			const float u = ( float( i ) + 0.5f ) / side, v = ( float( j ) + 0.5f ) / side;
			const float r = std::sqrt( u ), phi = 2.0f * kPi * v;
			const float3 d = t * ( r * std::cos( phi ) ) + b * ( r * std::sin( phi ) ) +
			                 n * std::sqrt( std::max( 0.0f, 1.0f - u ) );
			const auto hit = Hit( scene, p + n * 0.01f, d );
			float occluded = 0.0f;
			if ( hit )
				occluded = 1.0f - std::clamp( ( hit->first - kFalloff * kRadius ) /
				                                  ( ( 1.0f - kFalloff ) * kRadius ),
				                      0.0f, 1.0f );
			sum += 1.0 - occluded;
		}
	}
	return float( sum / ( side * side ) );
}

struct Camera
{
	float3 eye{ -60, 0, 60 };
	float3 forward;
	math::float4x4 view;
	math::float4x4 projection;
	math::float4x4 toClip;
	std::optional<math::float4x4> fromClip;
};

Camera MakeCamera()
{
	Camera camera;
	const float pitch = 35.0f * kPi / 180.0f;
	camera.forward = { std::cos( pitch ), 0.0f, -std::sin( pitch ) };
	camera.view = math::LookAt( camera.eye, camera.eye + camera.forward, { 0, 0, 1 } );
	const float aspect = float( kWidth ) / float( kHeight );
	const float vertical = 2.0f * std::atan( std::tan( kPi / 4.0f ) / aspect );
	camera.projection = math::Perspective( vertical, aspect, 1.0f, 65536.0f );
	camera.toClip = math::Multiply( camera.projection, camera.view );
	camera.fromClip = math::Inverse( camera.toClip );
	return camera;
}

// ssr.h OctEncode (the prepass's encoding).
void Oct( float3 n, float out[2] )
{
	const float s = std::fabs( n.x ) + std::fabs( n.y ) + std::fabs( n.z );
	float x = n.x / s, y = n.y / s;
	if ( n.z < 0.0f )
	{
		const float fx = ( 1.0f - std::fabs( y ) ) * ( x >= 0.0f ? 1.0f : -1.0f );
		const float fy = ( 1.0f - std::fabs( x ) ) * ( y >= 0.0f ? 1.0f : -1.0f );
		x = fx;
		y = fy;
	}
	out[0] = x;
	out[1] = y;
}

struct Frame
{
	std::vector<float> depth;          // clip depth, 1 where nothing
	std::vector<std::uint16_t> normal; // RGBA16F oct normal, roughness, radius 0
	std::vector<float3> position;
	std::vector<float3> surfaceNormal;
	std::vector<bool> covered;
};

Frame Synthesize( const Scene &scene, const Camera &camera )
{
	Frame frame;
	const std::size_t count = std::size_t( kWidth ) * kHeight;
	frame.depth.assign( count, 1.0f );
	frame.normal.assign( count * 4, FloatToHalf( 0.0f ) );
	frame.position.assign( count, {} );
	frame.surfaceNormal.assign( count, {} );
	frame.covered.assign( count, false );
	for ( std::uint32_t y = 0; y < kHeight; ++y )
	{
		for ( std::uint32_t x = 0; x < kWidth; ++x )
		{
			const float ndcX = ( float( x ) + 0.5f ) / kWidth * 2.0f - 1.0f;
			const float ndcY = 1.0f - ( float( y ) + 0.5f ) / kHeight * 2.0f;
			const math::float4 far =
			    math::Transform( *camera.fromClip, { ndcX, ndcY, 1.0f, 1.0f } );
			const float3 target{ far.x / far.w, far.y / far.w, far.z / far.w };
			const float3 direction = math::Normalize( target - camera.eye );
			const auto hit = Hit( scene, camera.eye, direction );
			if ( !hit )
				continue;
			const std::size_t i = std::size_t( y ) * kWidth + x;
			const float3 p = camera.eye + direction * hit->first;
			const math::float4 clip = math::Transform( camera.toClip, { p.x, p.y, p.z, 1.0f } );
			frame.depth[i] = clip.z / clip.w;
			float oct[2];
			Oct( hit->second, oct );
			frame.normal[i * 4 + 0] = FloatToHalf( oct[0] );
			frame.normal[i * 4 + 1] = FloatToHalf( oct[1] );
			frame.normal[i * 4 + 2] = FloatToHalf( 0.5f );
			frame.normal[i * 4 + 3] = FloatToHalf( 0.0f );
			frame.position[i] = p;
			frame.surfaceNormal[i] = hit->second;
			frame.covered[i] = true;
		}
	}
	return frame;
}

// Runs the pass on a frame; the visibility per pixel (row 0 at the top).
// A configuration of the pass the checks judge: the lab's own (8 x 8), and
// the product's presets (render_core_world.h RenderCoreWorldQuality).
struct Preset
{
	const char *suffix; // after "gtao" in the check names
	std::uint32_t slices;
	std::uint32_t steps;
	bool halfResolution;
};
constexpr Preset kPresets[] = {
    { "", 8, 8, false }, { ".high", 5, 8, false }, { ".high-half", 5, 8, true } };

std::optional<std::string> RunPass( IRenderDevice2 &device, std::span<const std::uint32_t> module,
    const Frame &frame, const Camera &camera, const Preset &preset, std::vector<float> &out )
{
	auto texture = [&]( Format format, std::initializer_list<ResourceUsage> usages )
	{
		TextureDesc d;
		d.format = format;
		d.width = kWidth;
		d.height = kHeight;
		d.usages = UsageSet( usages );
		auto made = device.CreateTexture( d );
		return made ? made.Value() : TextureId();
	};
	const TextureId depth =
	    texture( Format::kD32Float, { ResourceUsage::kCopyDestination, ResourceUsage::kSampled } );
	const TextureId normal = texture(
	    Format::kRGBA16Float, { ResourceUsage::kCopyDestination, ResourceUsage::kSampled } );
	const TextureId output = texture( Format::kRGBA16Float,
	    { ResourceUsage::kStorageWrite, ResourceUsage::kSampled, ResourceUsage::kCopySource } );
	std::vector<BufferId> buffers;
	auto buffer = [&]( std::uint64_t size, std::initializer_list<ResourceUsage> usages,
	                  MemoryKind memory = MemoryKind::kDeviceLocal )
	{
		BufferDesc d;
		d.size = size;
		d.usages = UsageSet( usages );
		d.memory = memory;
		auto made = device.CreateBuffer( d );
		if ( made )
			buffers.push_back( made.Value() );
		return made ? made.Value() : BufferId();
	};
	const auto depthBytes = std::as_bytes( std::span( frame.depth ) );
	const auto normalBytes = std::as_bytes( std::span( frame.normal ) );
	const BufferId depthStaging =
	    buffer( depthBytes.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId normalStaging = buffer(
	    normalBytes.size(), { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource } );
	const BufferId readback = buffer( std::uint64_t( kWidth ) * kHeight * 8,
	    { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
	if ( !depth.IsValid() || !normal.IsValid() || !output.IsValid() || !depthStaging.IsValid() ||
	     !normalStaging.IsValid() || !readback.IsValid() )
		return std::string( "a texture or buffer was refused" );
	pass::ao::AoParams params;
	params.radius = kRadius;
	params.falloff = kFalloff;
	params.slices = preset.slices;
	params.steps = preset.steps;
	params.halfResolution = preset.halfResolution;
	auto created = module.empty()
	                   ? pass::ao::AmbientOcclusion::Create( device, params )
	                   : pass::ao::AmbientOcclusion::CreateWithProgram( device, params, module );
	if ( !created )
		return std::string( "the pass was refused" );
	std::unique_ptr<pass::ao::AmbientOcclusion> ao = std::move( created ).Value();
	auto encoded = device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return std::string( "no encoder" );
	CommandEncoder &encoder = encoded.Value();
	auto upload = [&]( BufferId staging, TextureId id, std::span<const std::byte> bytes )
	{
		encoder.TransitionBuffer( staging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.WriteBuffer( staging, 0, bytes );
		encoder.TransitionBuffer( staging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
		encoder.TransitionTexture( id, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.CopyBufferToTexture( staging, id, { 0, 0, 0, kWidth, kHeight } );
		encoder.TransitionTexture( id, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
	};
	upload( depthStaging, depth, depthBytes );
	upload( normalStaging, normal, normalBytes );
	encoder.TransitionTexture( output, ResourceUsage::kUndefined, ResourceUsage::kSampled );
	pass::ao::AoView view;
	view.view = camera.view;
	view.projection = camera.projection;
	view.eye[0] = camera.eye.x;
	view.eye[1] = camera.eye.y;
	view.eye[2] = camera.eye.z;
	pass::ao::AoTargets targets;
	targets.depth = depth;
	targets.normalRoughness = normal;
	targets.output = output;
	targets.outputUsage = ResourceUsage::kSampled;
	targets.width = kWidth;
	targets.height = kHeight;
	if ( !ao->Record( encoder, targets, view ) )
		return std::string( "the pass did not record" );
	encoder.TransitionTexture( output, ResourceUsage::kSampled, ResourceUsage::kCopySource );
	encoder.TransitionBuffer( readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.CopyTextureToBuffer( output, readback, { 0, 0, 0, kWidth, kHeight } );
	auto token = device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !token )
		return std::string( "the submission was refused" );
	(void)device.WaitIdle();
	ao->Collect( token.Value() );
	std::vector<std::byte> bytes( std::size_t( kWidth ) * kHeight * 8 );
	if ( !device.ReadBuffer( readback, 0, bytes ) )
		return std::string( "the output did not read back" );
	out.assign( std::size_t( kWidth ) * kHeight, 0.0f );
	std::optional<std::string> receiverFailure;
	for ( std::size_t i = 0; i < out.size(); ++i )
	{
		std::uint16_t half[4];
		std::memcpy( half, bytes.data() + i * 8, sizeof( half ) );
		out[i] = HalfToFloat( half[0] );
		// The independently ray-cast fixture owns the receiver position;
		// visibility alone cannot detect occlusion applied to another surface.
		const float expectedDistance =
		    frame.depth[i] < 1.0f ? -math::TransformPoint( camera.view, frame.position[i] ).z
		                          : 0.0f;
		// D32 projection rounding grows quadratically with view distance.
		// Half-float receiver storage adds its own relative rounding bound.
		const float tolerance = std::max( 0.001f, expectedDistance * 0.001f ) +
		                        expectedDistance * expectedDistance * 1.2e-7f;
		if ( HalfToFloat( half[2] ) != -1.0f ||
		     std::fabs( HalfToFloat( half[1] ) - expectedDistance ) > tolerance )
			receiverFailure = "the AO image did not preserve its ray-cast receiver distance";
	}
	for ( TextureId id : { depth, normal, output } )
		(void)device.Release( id, token.Value() );
	for ( BufferId id : buffers )
		(void)device.Release( id, token.Value() );
	(void)device.WaitIdle();
	return receiverFailure;
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counted{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counted, device ) )
		return why;
	const Camera camera = MakeCamera();
	if ( !camera.fromClip )
		return std::string( "the camera does not invert" );

	for ( const Preset &preset : kPresets )
	{
		const std::string name = std::string( "gtao" ) + preset.suffix;
		// The open plane.
		{
			const Frame frame = Synthesize( Scene{ false }, camera );
			std::vector<float> ao;
			if ( std::optional<std::string> why =
			         RunPass( *device, module, frame, camera, preset, ao ) )
				return why;
			std::uint32_t bad = 0, empty = 0;
			float lowest = 1.0f;
			for ( std::size_t i = 0; i < ao.size(); ++i )
			{
				if ( !frame.covered[i] )
				{
					empty += ao[i] != 1.0f;
					continue;
				}
				lowest = std::min( lowest, ao[i] );
				bad += !( ao[i] >= 0.99f && ao[i] <= 1.0f );
			}
			results.That( bad == 0, name + ".plane",
			    std::to_string( bad ) + " pixels outside [0.99, 1]; lowest " +
			        std::to_string( lowest ) );
			results.That( empty == 0, name + ".depth-free", std::to_string( empty ) + " not one" );
		}

		// The crease.
		{
			const Scene scene{ true };
			const Frame frame = Synthesize( scene, camera );
			std::vector<float> ao;
			if ( std::optional<std::string> why =
			         RunPass( *device, module, frame, camera, preset, ao ) )
				return why;
			std::uint32_t judged = 0, bad = 0;
			std::vector<float> errors;
			double errorSum = 0.0;
			double nearSum = 0.0, farSum = 0.0;
			std::uint32_t nearCount = 0, farCount = 0;
			std::string first;
			for ( std::uint32_t y = 6; y + 6 < kHeight; ++y )
			{
				for ( std::uint32_t x = 6; x + 6 < kWidth; ++x )
				{
					const std::size_t i = std::size_t( y ) * kWidth + x;
					if ( !frame.covered[i] )
						continue;
					const float3 p = frame.position[i];
					// The crease line itself (both surfaces within a pixel) is
					// where the depth buffer cannot say which surface a pixel is.
					if ( std::fabs( p.x - kWall ) < 1.0f && p.z < 1.0f )
						continue;
					const float expected = ReferenceVisibility( scene, p, frame.surfaceNormal[i] );
					const float error = std::fabs( ao[i] - expected );
					++judged;
					errorSum += error;
					errors.push_back( error );
					if ( error > 0.06f )
					{
						++bad;
						if ( first.empty() )
						{
							char text[160];
							std::snprintf( text, sizeof( text ),
							    "first at %u,%u (%.1f %.1f %.1f): expected %.3f, got %.3f", x, y,
							    double( p.x ), double( p.y ), double( p.z ), double( expected ),
							    double( ao[i] ) );
							first = text;
						}
					}
					if ( frame.surfaceNormal[i].z > 0.5f )
					{
						if ( kWall - p.x < 20.0f )
						{
							nearSum += ao[i];
							++nearCount;
						}
						else if ( kWall - p.x > 100.0f )
						{
							farSum += ao[i];
							++farCount;
						}
					}
				}
			}
			const double mean = judged ? errorSum / judged : 1.0;
			std::sort( errors.begin(), errors.end() );
			const float p95 = errors.empty() ? 1.0f : errors[errors.size() * 95 / 100];
			char detail[260];
			std::snprintf( detail, sizeof( detail ),
			    "mean error %.4f, 95th percentile %.4f over %u judged pixels (%u above 0.06); %s",
			    mean, double( p95 ), judged, bad, first.c_str() );
			// GTAO fades a far occluder by pulling its slice's one horizon towards
			// open, where the rays count each blocked direction: it occludes less
			// near a crease (the wall 5 to 30 units above the floor, about 0.09
			// on average). The band holds that approximation, not a defect; it was
			// set after the first run (mean 0.021, 95th percentile 0.10) and is
			// disclosed as such in RFC/0016-progress.md.
			results.That(
			    judged > 1000 && mean <= 0.025 && p95 <= 0.12f, name + ".crease", detail );
			const double nearMean = nearCount ? nearSum / nearCount : 1.0;
			const double farMean = farCount ? farSum / farCount : 0.0;
			results.That( nearCount > 50 && farCount > 50 && nearMean < 0.9 * farMean,
			    name + ".crease-darkens",
			    "near " + std::to_string( nearMean ) + ", far " + std::to_string( farMean ) );
			std::printf( "INFO %s crease judged %u, mean error %.4f, near %.3f far %.3f\n",
			    name.c_str(), judged, mean, nearMean, farMean );
		}
	}
	device.reset();
	messages = counted.load();
	return std::nullopt;
}

const Seeded kGtaoSeeded[] = {
    { "projection-ignored", spirv::kGtaoProjectionIgnored, "gtao.crease" },
    { "screen-slices", spirv::kGtaoScreenSlices, "gtao.plane" },
    { "snapped-samples", spirv::kGtaoSnappedSamples, "gtao.plane" } };

} // namespace

int RunGtaoSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "gtao", kGtaoSeeded, RunOnce );
}

} // namespace render::lab
