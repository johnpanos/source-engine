//========= Copyright Valve Corporation, All rights reserved. ============//
// Independent double-precision foliage oracle shared by the vertex and shadow suites.
#ifndef RENDER_LAB_TREE_SWAY_REFERENCE_H
#define RENDER_LAB_TREE_SWAY_REFERENCE_H
#include <algorithm>
#include <array>
#include <cmath>
namespace render::lab::tree_sway_oracle
{
struct Case
{
	float positionTime[4] = {};
	float windMode[4] = {};
	float geometry[4] = { 1000.0f, 0.1f, 300.0f, 0.2f };
	float motion[4] = { 1.0f, 10.0f, 12.0f, 10.0f };
	float curves[4] = { 2.0f, 1.0f, 1.5f, 5.0f };
	float windControls[4] = { 3.0f, 6.0f, 0.0f, 0.0f };
	float objectToWorld[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
};
static_assert( sizeof( Case ) == 160 );

inline std::array<double, 3> Reference( const Case &c )
{
	std::array<double, 3> out = { c.positionTime[0], c.positionTime[1], c.positionTime[2] };
	const int mode = int( c.windMode[2] );
	if ( mode == 0 )
		return out;
	const double wx = c.windControls[2] ? 0.5 : c.windMode[0];
	const double wy = c.windControls[2] ? 0.5 : c.windMode[1];
	const double intensity = std::hypot( wx, wy );
	double wind[3];
	for ( int axis = 0; axis < 3; ++axis )
		wind[axis] = c.objectToWorld[axis] * wx + c.objectToWorld[4 + axis] * wy;
	const double x = out[0], y = out[1], z = out[2], time = c.positionTime[3];
	const double heightScale = std::clamp( ( z - double( c.geometry[0] ) * c.geometry[1] ) /
	                                           ( ( 1.0 - c.geometry[1] ) * c.geometry[0] ),
	    0.0, 1.0 );
	const double radiusStart = double( c.geometry[2] ) * c.geometry[3];
	const double radiusScale = std::clamp( std::hypot( x - radiusStart, y - radiusStart ) /
	                                           ( ( 1.0 - c.geometry[3] ) * c.geometry[2] ),
	    0.0, 1.0 );
	const bool active = mode == 2 ? z <= double( c.geometry[0] ) * c.geometry[1]
	                              : z >= double( c.geometry[0] ) * c.geometry[1];
	const double orthogonal =
	    1.0 - std::clamp( std::abs( wind[0] * x + wind[1] * y ) /
	                          ( std::max( std::hypot( wind[0], wind[1], wind[2] ), 0.0001 ) *
	                              std::max( std::hypot( x, y ), 0.0001 ) ),
	              0.0, 1.0 );
	const double trunk = c.motion[1] * std::pow( heightScale, c.curves[2] );
	const double branch = mode == 2 ? 0.0 : c.motion[1] * orthogonal * radiusScale * active;
	const double phase =
	    ( double( c.objectToWorld[3] ) + c.objectToWorld[7] + c.objectToWorld[11] ) * 19.0;
	const double slowTime = ( time + phase ) * c.motion[0];
	const double t = std::clamp(
	    ( intensity - c.windControls[0] ) / ( c.windControls[1] - c.windControls[0] ), 0.0, 1.0 );
	const double blend = t * t * ( 3.0 - 2.0 * t );
	const double sway0 =
	    std::lerp( std::sin( slowTime ), std::sin( slowTime * c.curves[0] ), blend );
	const double sway1 =
	    std::lerp( std::sin( slowTime * 2.31 ), std::sin( slowTime * 2.14 * c.curves[0] ), blend );
	const double scramble = c.motion[3] * std::pow( radiusScale, c.curves[1] ) * active;
	const double length = std::max( std::hypot( x, y, z ), 0.0001 );
	const double position[] = { x, y, z };
	for ( int axis = 0; axis < 3; ++axis )
	{
		out[axis] += wind[axis] * ( trunk * ( sway0 + 0.1 ) + branch * ( sway1 + 0.4 ) );
		const double scale = mode == 2 && axis < 2 ? 0.5 : 1.0;
		out[axis] += intensity * scramble * scale *
		             std::sin( c.curves[3] * time +
		                       position[( axis + 1 ) % 3] / length * c.motion[2] + phase );
	}
	return out;
}

}
#endif
