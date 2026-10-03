//========= Copyright Valve Corporation, All rights reserved. ============//
#include "materialsystem/procedural_texture_sample.h"
#include "testing/checks.h"
#include "render/world_panel.h"

#include <array>
#include <vector>

int main()
{
	using namespace procedural_texture_sample;
	testing::Checks checks;
	std::array<std::uint8_t, kBytes> output;
	const std::uint8_t red[] = { 0, 0, 255, 17, 18, 19 }; // BGR and unused row padding
	checks.That( Build( red, 1, 1, 6, true, output ), "padded BGR input accepted" );
	bool redFrame = true;
	for ( int i = 0; i < kBytes; i += 3 )
		redFrame = redFrame && output[i] == 255 && output[i + 1] == 0 && output[i + 2] == 0;
	checks.That( redFrame, "BGR channel order and row padding" );
	const std::uint8_t green[] = { 0, 255, 0 };
	checks.That( Build( green, 1, 1, 3, false, output ), "next RGB frame accepted" );
	checks.That( output[0] == 0 && output[1] == 255 && output[2] == 0,
	    "current frame replaces previous color" );
	std::vector<std::uint8_t> image( 256 * 256 * 3 );
	for ( int y = 0; y < 256; ++y )
		for ( int x = 0; x < 256; ++x )
			for ( int k = 0; k < 3; ++k )
				image[( y * 256 + x ) * 3 + k] = ( x + y ) % 2 ? 255 : 0;
	checks.That( Build( image, 256, 256, 256 * 3, false, output ), "checker accepted" );
	checks.Near( output[0], 188, 1, "linear mean is encoded once, gamma mean 128 rejected" );
	// The texture sheet's lower half is black padding; the frame occupies its upper half.
	for ( int y = 0; y < 256; ++y )
		for ( int x = 0; x < 256; ++x )
			for ( int k = 0; k < 3; ++k )
				image[( y * 256 + x ) * 3 + k] = y < 128 ? 255 : 0;
	checks.That( Build( image, 256, 256, 256 * 3, false, output ), "sheet accepted" );
	checks.That( output[0] == 255 && output[( 63 * 64 ) * 3] == 0,
	    "sheet UV regions retain frame and black padding" );
	checks.That( !Build( image, 0, 256, 768, false, output ), "empty dimensions refused" );
	checks.That( !Build( image, 256, 256, 2, false, output ), "short row refused" );
	checks.That( !Build( std::span( image ).first( 9 ), 256, 256, 768, false, output ),
	    "truncated input refused" );
	checks.That( !Build( image, 256, 256, 768, false, std::span( output ).first( 9 ) ),
	    "short output refused" );
	world_panel::Quad q = {};
	q.x1 = q.y1 = 128;
	q.texture = world_panel::kWhite;
	q.color[3] = 255;
	q.blend = world_panel::kBlendAlpha;
	q.coverage = -1.0f;
	checks.That(
	    world_panel::CoversFace( &q, 1, 128, 128 ), "opaque white foundation covers face" );
	q.color[3] = 254;
	checks.That( !world_panel::CoversFace( &q, 1, 128, 128 ), "partial alpha is not opaque" );
	q.color[3] = 255;
	q.texture = 0;
	checks.That(
	    !world_panel::CoversFace( &q, 1, 128, 128 ), "unknown texture alpha is not opaque" );
	q.coverage = 1.0f;
	checks.That( world_panel::CoversFace( &q, 1, 128, 128 ), "opaque image covers face" );
	q.blend = world_panel::kBlendAdditive;
	checks.That(
	    !world_panel::CoversFace( &q, 1, 128, 128 ), "additive image does not cover face" );
	q.blend = world_panel::kBlendOpaque;
	q.x1 = 127;
	checks.That( !world_panel::CoversFace( &q, 1, 128, 128 ), "edge gap retains transparency" );
	q.x1 = 128;
	q.layer = world_panel::kLayerCoating;
	checks.That(
	    !world_panel::CoversFace( &q, 1, 128, 128 ), "coating is not a display foundation" );
	checks.That( !world_panel::CoversFace( nullptr, 0, 128, 128 ), "empty face is not covered" );
	return checks.Report();
}
