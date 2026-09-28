//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.portal-lights.v1 (render/portal_lights.h, RFC 0016 K7).
//
//			An independent reference builds each portal pair from two frames
//			the way Portal links them (a point at local forward, right, up of
//			one portal appears at minus forward, minus right, up of the other)
//			and decides reach in the exit portal's frame: the receiver is in
//			front of the exit, the light in front of the entry, and the
//			segment from the receiver to the light's image crosses the exit's
//			plane inside its rectangle. The header decides it in the entry's
//			frame. Over 200 random pairs and 500 light/receiver pairs each
//			(samples within 0.01 units of a rectangle edge are left out as
//			numerically undecided) the two must agree exactly, with both
//			outcomes well represented. EntersPortal is checked against a
//			sampled distance to the rectangle, and the image transform
//			against its inverse.
//
//			Seeded defects (sensitivity rows) replace the call under test:
//			PORTAL_LIGHTS_SEEDED_FORWARD maps the receiver with the forward
//			transform instead of its inverse; PORTAL_LIGHTS_SEEDED_NO_APERTURE
//			ignores the rectangle.
//
//=============================================================================//

#include "render/portal_lights.h"
#include "testing/checks.h"

#include <cmath>
#include <cstdint>
#include <cstdio>

namespace
{

// A small deterministic generator (xorshift64*), uniform in [-1, 1].
class Random
{
public:
	explicit Random( std::uint64_t seed ) : m_State( seed ? seed : 1 ) {}
	double Unit()
	{
		m_State ^= m_State >> 12;
		m_State ^= m_State << 25;
		m_State ^= m_State >> 27;
		const std::uint64_t bits = m_State * 2685821657736338717ull;
		return double( bits >> 11 ) / double( 1ull << 53 ) * 2.0 - 1.0;
	}

private:
	std::uint64_t m_State;
};

struct Frame
{
	double o[3], f[3], r[3], u[3];
};

void Cross( const double a[3], const double b[3], double out[3] )
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

double Dot( const double a[3], const double b[3] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void Normalize( double v[3] )
{
	const double length = std::sqrt( Dot( v, v ) );
	for ( int k = 0; k < 3; ++k )
		v[k] /= length;
}

Frame RandomFrame( Random &random )
{
	auto unit = [&random]( Random & )
	{
		return random.Unit();
	};
	Frame frame;
	for ( int k = 0; k < 3; ++k )
	{
		frame.o[k] = 2000.0 * unit( random );
		frame.f[k] = unit( random );
	}
	Normalize( frame.f );
	double helper[3] = { unit( random ), unit( random ), unit( random ) };
	Cross( frame.f, helper, frame.r );
	Normalize( frame.r );
	Cross( frame.r, frame.f, frame.u ); // right-handed: f, r, u
	return frame;
}

void Local( const Frame &frame, const double p[3], double out[3] )
{
	const double d[3] = { p[0] - frame.o[0], p[1] - frame.o[1], p[2] - frame.o[2] };
	out[0] = Dot( d, frame.f );
	out[1] = Dot( d, frame.r );
	out[2] = Dot( d, frame.u );
}

void World( const Frame &frame, const double local[3], double out[3] )
{
	for ( int k = 0; k < 3; ++k )
		out[k] = frame.o[k] + local[0] * frame.f[k] + local[1] * frame.r[k] + local[2] * frame.u[k];
}

// The entry portal as the header takes it, with toLinked built from the two
// frames by composing E's local coordinates with X's (turned about up).
portal_lights::PortalInput Input(
    const Frame &e, const Frame &x, float halfWidth, float halfHeight )
{
	portal_lights::PortalInput in = {};
	for ( int k = 0; k < 3; ++k )
	{
		in.origin[k] = float( e.o[k] );
		in.forward[k] = float( e.f[k] );
		in.right[k] = float( e.r[k] );
		in.up[k] = float( e.u[k] );
	}
	in.halfWidth = halfWidth;
	in.halfHeight = halfHeight;
	// Columns: the images of the world axes and of the origin.
	const double zero[3] = { 0, 0, 0 };
	double base[3], imageBase[3];
	Local( e, zero, base );
	const double turned0[3] = { -base[0], -base[1], base[2] };
	World( x, turned0, imageBase );
	for ( int c = 0; c < 3; ++c )
	{
		double axis[3] = { 0, 0, 0 };
		axis[c] = 1.0;
		double local[3], image[3];
		Local( e, axis, local );
		const double turned[3] = { -local[0], -local[1], local[2] };
		World( x, turned, image );
		for ( int r = 0; r < 3; ++r )
			in.toLinked[r * 4 + c] = float( image[r] - imageBase[r] );
	}
	for ( int r = 0; r < 3; ++r )
		in.toLinked[r * 4 + 3] = float( imageBase[r] );
	return in;
}

// Reference: -1 undecided (near an edge), else 0 or 1.
int Reference( const Frame &e, const Frame &x, double hw, double hh, const double light[3],
    const double receiver[3] )
{
	double l[3], r[3];
	Local( e, light, l );
	Local( x, receiver, r );
	if ( l[0] <= 0.0 || r[0] <= 0.0 )
		return 0;
	// The light's image in X's frame, then the segment receiver -> image.
	const double image[3] = { -l[0], -l[1], l[2] };
	const double t = r[0] / ( r[0] - image[0] );
	const double cross[2] = { r[1] + t * ( image[1] - r[1] ), r[2] + t * ( image[2] - r[2] ) };
	const double edge = std::fmin(
	    std::fabs( std::fabs( cross[0] ) - hw ), std::fabs( std::fabs( cross[1] ) - hh ) );
	if ( edge < 0.01 )
		return -1;
	return std::fabs( cross[0] ) <= hw && std::fabs( cross[1] ) <= hh;
}

bool UnderTest(
    const portal_lights::PortalInput &entry, const float light[3], const float receiver[3] )
{
#if defined( PORTAL_LIGHTS_SEEDED_FORWARD )
	float mapped[3];
	portal_lights::ImagePoint( entry, receiver, mapped );
	return portal_lights::ThroughAperture( entry, light, mapped );
#elif defined( PORTAL_LIGHTS_SEEDED_NO_APERTURE )
	const float d[3] = {
	    light[0] - entry.origin[0], light[1] - entry.origin[1], light[2] - entry.origin[2] };
	float back[3];
	portal_lights::InverseImagePoint( entry, receiver, back );
	const float db[3] = {
	    back[0] - entry.origin[0], back[1] - entry.origin[1], back[2] - entry.origin[2] };
	return portal_lights::Dot( d, entry.forward ) > 0.0f &&
	       portal_lights::Dot( db, entry.forward ) < 0.0f;
#else
	return portal_lights::ReachesThroughPortal( entry, light, receiver );
#endif
}

} // namespace

int main()
{
	testing::Checks checks;
	Random random( 20260928 );
	auto unit = []( Random &r )
	{
		return r.Unit();
	};
	int decided = 0, reached = 0, mismatches = 0, undecided = 0;
	double worstRoundTrip = 0.0;
	int enterMismatches = 0, enterDecided = 0;
	for ( int pair = 0; pair < 200; ++pair )
	{
		const Frame e = RandomFrame( random );
		const Frame x = RandomFrame( random );
		const double hw = 32.0, hh = 54.0;
		const portal_lights::PortalInput entry = Input( e, x, float( hw ), float( hh ) );
		for ( int sample = 0; sample < 500; ++sample )
		{
			// Lights in front of E near its opening, receivers in front of X.
			const double lLocal[3] = { 1.0 + 200.0 * ( unit( random ) * 0.5 + 0.5 ),
			    150.0 * unit( random ), 150.0 * unit( random ) };
			const double rLocal[3] = { 1.0 + 300.0 * ( unit( random ) * 0.5 + 0.5 ),
			    300.0 * unit( random ), 300.0 * unit( random ) };
			double light[3], receiver[3];
			World( e, lLocal, light );
			World( x, rLocal, receiver );
			const int expected = Reference( e, x, hw, hh, light, receiver );
			const float lf[3] = { float( light[0] ), float( light[1] ), float( light[2] ) };
			const float rf[3] = {
			    float( receiver[0] ), float( receiver[1] ), float( receiver[2] ) };
			if ( expected < 0 )
			{
				++undecided;
				continue;
			}
			++decided;
			reached += expected;
			mismatches += UnderTest( entry, lf, rf ) != ( expected == 1 );

			// Round trip of the image transform.
			float image[3], back[3];
			portal_lights::ImagePoint( entry, lf, image );
			portal_lights::InverseImagePoint( entry, image, back );
			for ( int k = 0; k < 3; ++k )
				worstRoundTrip = std::fmax( worstRoundTrip, std::fabs( back[k] - lf[k] ) );

			// EntersPortal against a sampled distance to the rectangle.
			double nearest = 1e30;
			for ( int i = 0; i <= 64; ++i )
				for ( int j = 0; j <= 64; ++j )
				{
					const double p[3] = {
					    0.0, -hw + 2.0 * hw * i / 64.0, -hh + 2.0 * hh * j / 64.0 };
					const double d[3] = { lLocal[0] - p[0], lLocal[1] - p[1], lLocal[2] - p[2] };
					nearest = std::fmin( nearest, std::sqrt( Dot( d, d ) ) );
				}
			const double radius = 50.0 + 250.0 * ( unit( random ) * 0.5 + 0.5 );
			if ( std::fabs( nearest - radius ) > 2.0 ) // the sampling error is below 1.7
			{
				++enterDecided;
				enterMismatches += portal_lights::EntersPortal( entry, lf, float( radius ) ) !=
				                   ( nearest < radius );
			}
		}
	}
	std::printf(
	    "INFO portal lights: %d decided samples (%d reach through the pair), %d near an edge; "
	    "%d mismatches; image round trip %.3g units; EntersPortal %d decided, %d mismatches\n",
	    decided, reached, undecided, mismatches, worstRoundTrip, enterDecided, enterMismatches );
	checks.That( decided > 90000, "reach.most-samples-are-decided" );
	checks.That(
	    reached > decided / 20 && reached < decided - decided / 20, "reach.both-outcomes-occur" );
	checks.Equal( mismatches, 0, "reach.agrees-with-the-exit-frame-reference" );
	checks.That( worstRoundTrip < 1e-2, "image.inverse-undoes-the-transform" );
	checks.Equal( enterMismatches, 0, "enters.agrees-with-the-sampled-distance" );

	// A light behind the entry makes no image, and a receiver behind the exit
	// sees nothing.
	const Frame e = RandomFrame( random ), x = RandomFrame( random );
	const portal_lights::PortalInput entry = Input( e, x, 32.0f, 54.0f );
	const double behindLocal[3] = { -10.0, 0.0, 0.0 };
	double behind[3];
	World( e, behindLocal, behind );
	const float bf[3] = { float( behind[0] ), float( behind[1] ), float( behind[2] ) };
	checks.That( !portal_lights::EntersPortal( entry, bf, 1000.0f ), "enters.not-from-behind" );
	const double lightLocal[3] = { 50.0, 0.0, 0.0 }, receiverLocal[3] = { -50.0, 0.0, 0.0 };
	double light[3], receiver[3];
	World( e, lightLocal, light );
	World( x, receiverLocal, receiver );
	const float lf[3] = { float( light[0] ), float( light[1] ), float( light[2] ) };
	const float rf[3] = { float( receiver[0] ), float( receiver[1] ), float( receiver[2] ) };
	checks.That( !UnderTest( entry, lf, rf ), "reach.not-behind-the-exit" );
	return checks.Report();
}
