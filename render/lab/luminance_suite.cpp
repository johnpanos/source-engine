//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite luminance (RFC 0016 K8 post cohort;
//			render.pass.luminance): the counts behind auto exposure, judged
//			against the legacy luminance_compare_ps2x test restated here (the
//			NTSC weights 0.2125, 0.7154, 0.0721 on the texel as stored times
//			the scale, counted when minimum <= L <= maximum) over a fixture
//			whose every texel is counted on the CPU:
//			- the whole texture, a sub-rectangle, a scale;
//			- the range's maximum is inclusive (a texel exactly at it counts);
//			- a count is pending until its frame's completion token is
//			  reported, then read once;
//			- a slot recorded again (a capture) dispatches nothing more;
//			- malformed queries are refused; a texture that does not import
//			  fails the query by name.
//
//			Seeded (--sensitivity): half-open (the maximum excluded) must fail
//			luminance.range.maximum-inclusive.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/pass/luminance/luminance.h"
#include "spv/luminance_defects_spv.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using pass::luminance::LuminanceCounter;
using pass::luminance::LuminanceTarget;
using pass::luminance::Query;

constexpr std::uint32_t kSize = 24;
constexpr int kKey = 5;

class LabTextures final : public pass::luminance::ILuminanceTextures
{
public:
	TextureId texture;
	TextureId Import( int key ) override { return key == kKey ? texture : TextureId(); }
};

// The fixture: a ramp of grays and colors, with a pure red column (L
// exactly 0.2125, the inclusive-maximum case).
std::uint8_t Texel( std::uint32_t x, std::uint32_t y, int c )
{
	if ( x == 3 )
		return c == 0 ? 255 : c == 3 ? 255 : 0;
	const std::uint32_t v = ( x * 11 + y * 7 ) % 256;
	return std::uint8_t( c == 0 ? v : c == 1 ? ( v * 3 ) % 256 : c == 2 ? 255 - v : 255 );
}

// The legacy test, restated: the texel as stored times the scale.
std::int64_t Expected( int x0, int y0, int x1, int y1, float minimum, float maximum, float scale )
{
	std::int64_t count = 0;
	for ( int y = y0; y <= y1; ++y )
		for ( int x = x0; x <= x1; ++x )
		{
			const float r = Texel( x, y, 0 ) / 255.0f * scale;
			const float g = Texel( x, y, 1 ) / 255.0f * scale;
			const float b = Texel( x, y, 2 ) / 255.0f * scale;
			const float l = r * 0.2125f + g * 0.7154f + b * 0.0721f;
			count += l >= minimum && l <= maximum ? 1 : 0;
		}
	return count;
}

std::optional<std::string> RunChecks( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( auto why = CreateLabDevice( validate, counter, device ) )
		return why;
	resources::TextureCache textures( *device );
	material::GroupResidency groups( *device, textures );
	std::unique_ptr<Canvas> canvas;
	if ( auto why = Canvas::Create( *device, 4, 4, canvas ) )
		return why;
	LabTextures imports;
	{
		TextureDesc desc;
		desc.format = Format::kRGBA8Unorm;
		desc.width = desc.height = kSize;
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		std::vector<std::byte> texels( std::size_t( kSize ) * kSize * 4 );
		for ( std::uint32_t y = 0; y < kSize; ++y )
			for ( std::uint32_t x = 0; x < kSize; ++x )
				for ( int c = 0; c < 4; ++c )
					texels[( y * kSize + x ) * 4 + c] = std::byte( Texel( x, y, c ) );
		auto staged = textures.Stage( "luminance/fixture", desc, texels );
		if ( !staged )
			return std::string( "the luminance fixture did not stage" );
		imports.texture = staged.Value().texture;
	}
	LuminanceCounter luminance( module );
	std::uint64_t frame = 0;
	// Records the tags in one frame, then reports a later completed token.
	auto record = [&]( const std::vector<std::uint32_t> &tags ) -> std::optional<std::string>
	{
		CanvasPost post = [&]( CommandEncoder &encoder, TextureId,
		                      TextureId ) -> std::optional<std::string>
		{
			LuminanceTarget target;
			target.device = device.get();
			target.textures = &imports;
			target.frame = ++frame;
			for ( std::uint32_t tag : tags )
				luminance.Record( tag, encoder, target );
			return std::nullopt;
		};
		if ( auto why = canvas->Render( textures, groups, {}, { 0, 0, 0, 1 }, nullptr, post ) )
			return why;
		auto encoded = device->BeginEncoder( QueueKind::kGraphics );
		if ( !encoded )
			return std::string( "a marker encoder was refused" );
		CommandEncoder marker = std::move( encoded ).Value();
		auto token = device->Submit( QueueKind::kGraphics, { &marker, 1 }, {} );
		if ( !token || !device->WaitIdle() )
			return std::string( "the marker submission failed" );
		luminance.FrameSubmitted( *device, token.Value() );
		return std::nullopt;
	};

	struct Case
	{
		const char *name;
		Query query;
	};
	const Case kCases[] = {
	    { "whole", { kKey, 0, 0, int( kSize ) - 1, int( kSize ) - 1, 0.2f, 0.6f, 1.0f } },
	    { "sub-rectangle", { kKey, 2, 5, 17, 11, 0.1f, 0.45f, 1.0f } },
	    { "scale", { kKey, 0, 0, int( kSize ) - 1, int( kSize ) - 1, 0.5f, 10000.0f, 2.0f } },
	    { "maximum-inclusive", { kKey, 3, 0, 3, int( kSize ) - 1, 0.0f, 0.2125f, 1.0f } },
	};
	std::vector<std::uint32_t> tags;
	for ( const Case &c : kCases )
		tags.push_back( luminance.Queue( c.query ) );
	bool pendingBefore = true;
	for ( std::uint32_t tag : tags )
		pendingBefore = pendingBefore && tag != 0 && !luminance.Result( tag ).has_value();
	if ( auto why = record( tags ) )
		return why;
	results.That( pendingBefore, "luminance.pending-until-complete" );
	for ( std::size_t i = 0; i < tags.size(); ++i )
	{
		const Query &q = kCases[i].query;
		const std::int64_t expected =
		    Expected( q.x0, q.y0, q.x1, q.y1, q.minimum, q.maximum, q.scale );
		const std::optional<std::int64_t> got = luminance.Result( tags[i] );
		const std::string name = std::string( kCases[i].name ) == "maximum-inclusive"
		                             ? "luminance.range.maximum-inclusive"
		                             : std::string( "luminance.count." ) + kCases[i].name;
		results.That( got && *got == expected && expected > 0, name,
		    "expected " + std::to_string( expected ) + ", got " +
		        ( got ? std::to_string( *got ) : std::string( "pending" ) ) );
	}
	results.That( !luminance.Result( tags[0] ).has_value(), "luminance.result-read-once" );

	// A slot recorded again dispatches nothing more.
	const std::uint64_t recorded = luminance.Stats().recorded;
	if ( auto why = record( { tags[0] } ) )
		return why;
	results.That( luminance.Stats().recorded == recorded && luminance.Stats().failed == 0,
	    "luminance.recorded-again-dispatches-nothing", luminance.Stats().lastFailure );

	// Refusals and failures.
	results.That( luminance.Queue( { kKey, 4, 0, 2, 3, 0.0f, 1.0f, 1.0f } ) == 0 &&
	                  luminance.Queue( { 0, 0, 0, 1, 1, 0.0f, 1.0f, 1.0f } ) == 0 &&
	                  luminance.Queue( { kKey, 0, 0, 1, 1, 0.0f, 1.0f, 0.0f } ) == 0,
	    "luminance.refuse-malformed" );
	const std::uint32_t missing = luminance.Queue( { 9, 0, 0, 1, 1, 0.0f, 1.0f, 1.0f } );
	if ( auto why = record( { missing } ) )
		return why;
	const std::optional<std::int64_t> failed = luminance.Result( missing );
	results.That( failed && *failed == -1 &&
	                  luminance.Stats().lastFailure.find( "does not import" ) != std::string::npos,
	    "luminance.missing-texture-fails-by-name", luminance.Stats().lastFailure );

	(void)device->WaitIdle();
	luminance.ReleaseDevice( *device );
	messages = counter.load();
	results.That( messages == 0, "luminance.validation-silent" );
	return std::nullopt;
}

} // namespace

int RunLuminanceSuite( int argc, char **argv )
{
	static const Seeded kSeeded[] = {
	    { "half-open", spirv::kLuminanceHalfOpen, "luminance.range.maximum-inclusive" },
	};
	return RunSeededSuite( argc, argv, "luminance", kSeeded, RunChecks );
}

} // namespace render::lab
