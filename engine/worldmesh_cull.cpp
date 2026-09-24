//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conservative CPU culling of WMSH meshlets (RFC 0008 F4).
//
//=============================================================================//

#include "worldmesh_cull.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace worldmesh_cull
{
namespace
{

// A covered cell's corners lie this far (in cells) inside every occluder
// edge, so every GPU pixel center in the cell is inside the triangle despite
// vertex snapping. A box's cell rectangle grows by the same amount.
const double kCoverMargin = 0.01;
// Depth margin: relative float error, an absolute floor, and a depth-buffer
// quantization term (24-bit depth with a 4x safety factor).
const double kDepthRelative = 1e-4;
const double kDepthAbsolute = 0.01;
const double kDepthQuantization = 1.0 / double( 1 << 22 );
// One-sided occluders must face the eye by at least this sine.
const double kFacingMargin = 1e-3;

double Dot( const double a[3], const double b[3] )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void Cross( const double a[3], const double b[3], double out[3] )
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

void Load( const float in[3], double out[3] )
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

} // namespace

bool ConeFacesAway( const float eye[3], const float center[3], float radius, const float axis[3],
    float cutoff, float margin )
{
	// Faces spanning a hemisphere or more always include one that faces the eye.
	if ( !( cutoff > 0.0f ) )
		return false;
	double toCenter[3] = {
	    double( center[0] ) - eye[0], double( center[1] ) - eye[1], double( center[2] ) - eye[2] };
	const double distance = std::sqrt( Dot( toCenter, toCenter ) );
	if ( !( distance > radius ) )
		return false;
	double cone[3];
	Load( axis, cone );
	// For n within angle t of the axis and d = center - eye at angle f from
	// it, min dot(n, d) is |d| cos(f + t); a point of the sphere lowers it by
	// at most the radius.
	const double along = Dot( cone, toCenter );
	if ( along <= 0.0 )
		return false;
	const double across = std::sqrt( std::max( 0.0, distance * distance - along * along ) );
	const double sine = std::sqrt( std::max( 0.0, 1.0 - double( cutoff ) * cutoff ) );
	return along * cutoff - across * sine - radius > double( margin ) * distance;
}

OcclusionBuffer::ClipVertex OcclusionBuffer::Transform( const double point[3] ) const
{
	const double *m = m_matrix;
	return { m[0] * point[0] + m[1] * point[1] + m[2] * point[2] + m[3],
	    m[4] * point[0] + m[5] * point[1] + m[6] * point[2] + m[7],
	    m[12] * point[0] + m[13] * point[1] + m[14] * point[2] + m[15] };
}

void OcclusionBuffer::Begin(
    const float worldToClip[16], const float eye[3], float zNear, float zFar )
{
	for ( int i = 0; i < 16; ++i )
		m_matrix[i] = worldToClip[i];
	Load( eye, m_eye );
	m_zNear = zNear;
	m_zFar = zFar;
	// Cell units per world unit at depth 1, the larger of the two axes: no
	// projection magnifies a length more at that depth.
	const double scaleX = std::sqrt(
	    m_matrix[0] * m_matrix[0] + m_matrix[1] * m_matrix[1] + m_matrix[2] * m_matrix[2] );
	const double scaleY = std::sqrt(
	    m_matrix[4] * m_matrix[4] + m_matrix[5] * m_matrix[5] + m_matrix[6] * m_matrix[6] );
	m_cellScale = float( std::max( 0.5 * kWidth * scaleX, 0.5 * kHeight * scaleY ) );
	m_rasterized = 0;
	m_finished = false;
	m_levelCount = 0;
	int width = kWidth;
	int height = kHeight;
	for ( ;; )
	{
		m_levelWidth[m_levelCount] = width;
		m_levelHeight[m_levelCount] = height;
		m_levels[m_levelCount].assign( size_t( width ) * height, 0.0f );
		++m_levelCount;
		if ( ( width == 1 && height == 1 ) || m_levelCount == 8 )
			break;
		width = std::max( 1, width / 2 );
		height = std::max( 1, height / 2 );
	}
}

bool OcclusionBuffer::MayCoverCell( const float center[3], float radius, float inradius ) const
{
	const double *m = m_matrix;
	const double w = m[12] * center[0] + m[13] * center[1] + m[14] * center[2] + m[15];
	if ( w + radius < m_zNear )
		return false;
	// A triangle containing a unit cell contains its inscribed circle, so its
	// projected inradius is at least half a cell; projection magnifies no
	// length by more than m_cellScale over the nearest depth.
	const double nearest = std::max( w - radius, m_zNear );
	return inradius * m_cellScale >= 0.5 * nearest;
}

void OcclusionBuffer::AddOccluder(
    const float a[3], const float b[3], const float c[3], bool twoSided )
{
	double corner[3][3];
	Load( a, corner[0] );
	Load( b, corner[1] );
	Load( c, corner[2] );
	double edge1[3], edge2[3], normal[3], toEye[3];
	for ( int i = 0; i < 3; ++i )
	{
		edge1[i] = corner[1][i] - corner[0][i];
		edge2[i] = corner[2][i] - corner[0][i];
		toEye[i] = m_eye[i] - corner[0][i];
	}
	Cross( edge1, edge2, normal );
	const double normalLength = std::sqrt( Dot( normal, normal ) );
	if ( !( normalLength > 0.0 ) )
		return;
	if ( !twoSided &&
	     Dot( normal, toEye ) <= kFacingMargin * normalLength * std::sqrt( Dot( toEye, toEye ) ) )
		return;

	// Keep the part the GPU draws, zNear <= w <= zFar (Sutherland-Hodgman).
	ClipVertex polygon[8] = {
	    Transform( corner[0] ), Transform( corner[1] ), Transform( corner[2] ) };
	int count = 3;
	for ( int plane = 0; plane < 2 && count; ++plane )
	{
		ClipVertex clipped[8];
		int kept = 0;
		for ( int i = 0; i < count; ++i )
		{
			const ClipVertex &from = polygon[i];
			const ClipVertex &to = polygon[( i + 1 ) % count];
			const double fromSide = plane ? m_zFar - from.w : from.w - m_zNear;
			const double toSide = plane ? m_zFar - to.w : to.w - m_zNear;
			if ( fromSide >= 0.0 )
				clipped[kept++] = from;
			if ( ( fromSide >= 0.0 ) != ( toSide >= 0.0 ) )
			{
				const double t = fromSide / ( fromSide - toSide );
				clipped[kept++] = { from.x + ( to.x - from.x ) * t, from.y + ( to.y - from.y ) * t,
				    from.w + ( to.w - from.w ) * t };
			}
		}
		count = kept;
		std::copy( clipped, clipped + kept, polygon );
	}
	if ( count < 3 )
		return;

	// Cell space: u, v in [0, kWidth] x [0, kHeight]; q = 1 / w is affine in it.
	double u[8], v[8], q[8];
	double uMin = DBL_MAX, uMax = -DBL_MAX, vMin = DBL_MAX, vMax = -DBL_MAX;
	for ( int i = 0; i < count; ++i )
	{
		const double inverse = 1.0 / polygon[i].w;
		u[i] = ( polygon[i].x * inverse + 1.0 ) * 0.5 * kWidth;
		v[i] = ( polygon[i].y * inverse + 1.0 ) * 0.5 * kHeight;
		q[i] = inverse;
		uMin = std::min( uMin, u[i] );
		uMax = std::max( uMax, u[i] );
		vMin = std::min( vMin, v[i] );
		vMax = std::max( vMax, v[i] );
	}
	const int column0 = std::max( 0, int( std::ceil( uMin ) ) );
	const int column1 = std::min( kWidth, int( std::floor( uMax ) ) );
	const int row0 = std::max( 0, int( std::ceil( vMin ) ) );
	const int row1 = std::min( kHeight, int( std::floor( vMax ) ) );
	if ( column1 - column0 < 1 || row1 - row0 < 1 )
		return;

	// The q plane through the fan triangle with the largest area.
	double area = 0.0;
	int best = 1;
	for ( int i = 1; i + 1 < count; ++i )
	{
		const double fan =
		    ( u[i] - u[0] ) * ( v[i + 1] - v[0] ) - ( u[i + 1] - u[0] ) * ( v[i] - v[0] );
		if ( std::fabs( fan ) > std::fabs( area ) )
		{
			area = fan;
			best = i;
		}
	}
	if ( std::fabs( area ) < 1e-9 )
		return;
	const double du1 = u[best] - u[0], dv1 = v[best] - v[0], dq1 = q[best] - q[0];
	const double du2 = u[best + 1] - u[0], dv2 = v[best + 1] - v[0], dq2 = q[best + 1] - q[0];
	const double qu = ( dq1 * dv2 - dq2 * dv1 ) / area;
	const double qv = ( du1 * dq2 - du2 * dq1 ) / area;
	const double q0 = q[0] - qu * u[0] - qv * v[0];

	// Edges as normalized affine functions, positive inside. The clipped
	// projection of a triangle in front of the eye is convex.
	double edgeU[8], edgeV[8], edgeC[8];
	const double orientation = area > 0.0 ? 1.0 : -1.0;
	for ( int i = 0; i < count; ++i )
	{
		const int next = ( i + 1 ) % count;
		const double eu = u[next] - u[i];
		const double ev = v[next] - v[i];
		const double length = std::sqrt( eu * eu + ev * ev );
		if ( length < 1e-12 )
		{
			edgeU[i] = edgeV[i] = 0.0;
			edgeC[i] = 1.0;
			continue;
		}
		edgeU[i] = -orientation * ev / length;
		edgeV[i] = orientation * eu / length;
		edgeC[i] = -( edgeU[i] * u[i] + edgeV[i] * v[i] );
	}
	// The covered interval of a corner row: every edge >= kCoverMargin.
	auto interval = [&]( int row, double *low, double *high )
	{
		*low = -DBL_MAX;
		*high = DBL_MAX;
		for ( int i = 0; i < count; ++i )
		{
			const double rest = kCoverMargin - edgeV[i] * row - edgeC[i];
			if ( edgeU[i] > 1e-12 )
				*low = std::max( *low, rest / edgeU[i] );
			else if ( edgeU[i] < -1e-12 )
				*high = std::min( *high, rest / edgeU[i] );
			else if ( rest > 0.0 )
				*high = -DBL_MAX;
		}
	};
	// Cells hold 1 / w, the nearness of the farthest covering point: 0 is
	// uncovered, larger is nearer. It is affine across a row, so no division.
	std::vector<float> &nearness = m_levels[0];
	double low0, high0;
	interval( row0, &low0, &high0 );
	bool wrote = false;
	for ( int row = row0; row < row1; ++row )
	{
		double low1, high1;
		interval( row + 1, &low1, &high1 );
		const int first = std::max( column0, int( std::ceil( std::max( low0, low1 ) ) ) );
		const int last = std::min( column1, int( std::floor( std::min( high0, high1 ) ) ) );
		// The farthest point of a cell: q is smallest at one corner.
		const double rowQ = q0 + qv * ( row + ( qv < 0.0 ? 1.0 : 0.0 ) ) + ( qu < 0.0 ? qu : 0.0 );
		float *pRow = nearness.data() + size_t( row ) * kWidth;
		for ( int column = first; column < last; ++column )
		{
			const float farthest = float( rowQ + qu * column );
			if ( farthest > pRow[column] )
			{
				pRow[column] = farthest;
				wrote = true;
			}
		}
		low0 = low1;
		high0 = high1;
	}
	m_rasterized += wrote;
}

void OcclusionBuffer::Finish()
{
	for ( int level = 1; level < m_levelCount; ++level )
	{
		const std::vector<float> &child = m_levels[level - 1];
		const int childWidth = m_levelWidth[level - 1];
		const int childHeight = m_levelHeight[level - 1];
		for ( int y = 0; y < m_levelHeight[level]; ++y )
		{
			for ( int x = 0; x < m_levelWidth[level]; ++x )
			{
				float value = FLT_MAX;
				for ( int cy = 2 * y; cy < std::min( 2 * y + 2, childHeight ); ++cy )
				{
					for ( int cx = 2 * x; cx < std::min( 2 * x + 2, childWidth ); ++cx )
						value = std::min( value, child[size_t( cy ) * childWidth + cx] );
				}
				m_levels[level][size_t( y ) * m_levelWidth[level] + x] = value;
			}
		}
	}
	m_finished = true;
}

float OcclusionBuffer::Hierarchy( int level, int x, int y ) const
{
	return m_levels[level][size_t( y ) * m_levelWidth[level] + x];
}

int OcclusionBuffer::CoveredCells() const
{
	int covered = 0;
	for ( float cell : m_levels[0] )
		covered += cell > 0.0f;
	return covered;
}

bool OcclusionBuffer::IsBoxOccluded( const float mins[3], const float maxs[3] ) const
{
	if ( !m_finished || !m_rasterized )
		return false;
	// Clip-space intervals of the box from its center and half extents.
	const double center[3] = { 0.5 * ( double( mins[0] ) + maxs[0] ),
	    0.5 * ( double( mins[1] ) + maxs[1] ), 0.5 * ( double( mins[2] ) + maxs[2] ) };
	const double half[3] = { 0.5 * ( double( maxs[0] ) - mins[0] ),
	    0.5 * ( double( maxs[1] ) - mins[1] ), 0.5 * ( double( maxs[2] ) - mins[2] ) };
	double mid[3], extent[3];
	const int rows[3] = { 0, 1, 3 };
	for ( int i = 0; i < 3; ++i )
	{
		const double *m = m_matrix + 4 * rows[i];
		mid[i] = m[0] * center[0] + m[1] * center[1] + m[2] * center[2] + m[3];
		extent[i] =
		    std::fabs( m[0] ) * half[0] + std::fabs( m[1] ) * half[1] + std::fabs( m[2] ) * half[2];
	}
	const double nearest = mid[2] - extent[2];
	const double farthest = mid[2] + extent[2];
	// A box reaching the near plane is seen at the nearest possible depth.
	if ( !( nearest >= m_zNear ) )
		return false;
	double cell[2][2];
	const int size[2] = { kWidth, kHeight };
	for ( int axis = 0; axis < 2; ++axis )
	{
		const double low = mid[axis] - extent[axis];
		const double high = mid[axis] + extent[axis];
		const double ndcLow = std::min( low / nearest, low / farthest );
		const double ndcHigh = std::max( high / nearest, high / farthest );
		cell[axis][0] = ( ndcLow + 1.0 ) * 0.5 * size[axis] - kCoverMargin;
		cell[axis][1] = ( ndcHigh + 1.0 ) * 0.5 * size[axis] + kCoverMargin;
		// Entirely off screen: frustum culling owns that decision.
		if ( cell[axis][1] <= 0.0 || cell[axis][0] >= size[axis] )
			return false;
	}
	int x0 = std::max( 0, int( std::floor( cell[0][0] ) ) );
	int x1 = std::min( kWidth - 1, int( std::floor( cell[0][1] ) ) );
	int y0 = std::max( 0, int( std::floor( cell[1][0] ) ) );
	int y1 = std::min( kHeight - 1, int( std::floor( cell[1][1] ) ) );
	// The level where the rectangle spans at most two texels per axis.
	int level = 0;
	while ( level + 1 < m_levelCount &&
	        ( ( x1 >> level ) - ( x0 >> level ) > 1 || ( y1 >> level ) - ( y0 >> level ) > 1 ) )
		++level;
	x0 >>= level;
	x1 >>= level;
	y0 >>= level;
	y1 >>= level;
	float occluder = FLT_MAX;
	for ( int y = y0; y <= y1; ++y )
	{
		for ( int x = x0; x <= x1; ++x )
			occluder = std::min( occluder, Hierarchy( level, x, y ) );
	}
	// An uncovered cell, or an occluder at or behind the far plane rounding.
	if ( !( occluder > 0.0f ) )
		return false;
	// Float storage of 1 / w rounds by at most a relative 2^-24.
	const double far = ( 1.0 + 1e-6 ) / occluder;
	const double margin =
	    far * kDepthRelative + kDepthAbsolute + far * far * kDepthQuantization / m_zNear;
	return nearest > far + margin;
}

} // namespace worldmesh_cull
