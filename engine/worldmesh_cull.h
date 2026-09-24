//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conservative CPU culling of WMSH meshlets (RFC 0008 F4).
//
// Each test may keep a meshlet that a view could skip, never the reverse: a
// meshlet is rejected only when the GPU would draw none of its pixels. The
// interface uses plain arrays so it builds and runs without the engine.
//
//=============================================================================//

#ifndef WORLDMESH_CULL_H
#define WORLDMESH_CULL_H

#include <vector>

namespace worldmesh_cull
{

// True when every triangle of a meshlet faces away from eye. The meshlet's
// triangles lie in the sphere (center, radius) and each unit front-face
// normal n (a WMSH v2 cone) satisfies dot(n, axis) >= cutoff. For every such
// n and every point p in the sphere, dot(n, p - eye) must exceed
// margin * |center - eye|, so faces near edge-on are kept.
bool ConeFacesAway( const float eye[3], const float center[3], float radius, const float axis[3],
    float cutoff, float margin );

// A low-resolution depth buffer of drawn occluders, stored as the farthest
// view depth each cell is certain to be covered by, and a max hierarchy over
// it. Occluders only lower a cell; a box is occluded when its nearest depth
// is behind the farthest occluder depth everywhere its projection can reach.
class OcclusionBuffer
{
public:
	static const int kWidth = 128;
	static const int kHeight = 64;

	// Starts a view. worldToClip is row-major, clip = M * (x, y, z, 1), with w
	// the view depth; the GPU draws clip points with -w <= x, y <= w and
	// zNear <= w <= zFar. eye is the view origin.
	void Begin( const float worldToClip[16], const float eye[3], float zNear, float zFar );
	// Adds a triangle the GPU draws opaque, depth-tested and depth-writing in
	// this view. A one-sided triangle occludes only while its front face,
	// cross(b - a, c - a), faces the eye.
	void AddOccluder( const float a[3], const float b[3], const float c[3], bool twoSided );
	// Builds the hierarchy; boxes may be tested until the next Begin.
	void Finish();
	// True when every point of the box is farther, by a depth-precision
	// margin, than the drawn occluders over the whole region it can cover.
	bool IsBoxOccluded( const float mins[3], const float maxs[3] ) const;

	int RasterizedOccluders() const { return m_rasterized; }
	int CoveredCells() const;

private:
	struct ClipVertex
	{
		double x, y, w;
	};
	ClipVertex Transform( const double point[3] ) const;
	float Hierarchy( int level, int x, int y ) const;

	double m_matrix[16] = {};
	double m_eye[3] = {};
	double m_zNear = 1;
	double m_zFar = 1;
	int m_rasterized = 0;
	bool m_finished = false;
	std::vector<float> m_levels[8];
	int m_levelWidth[8] = {};
	int m_levelHeight[8] = {};
	int m_levelCount = 0;
};

} // namespace worldmesh_cull

#endif // WORLDMESH_CULL_H
