//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Viewport projection for the Hammer editor (RFC 0002,
//			hammer.viewport): the orthographic 2D camera of the Top, Front and
//			Side views and the perspective 3D camera, as plain values. Tools,
//			picking, the grid and presenters project through these types, so
//			one convention owns screen space, world axes and view angles.
//
//			Screen space. Pixels, origin at the viewport's top-left corner, x
//			to the right, y DOWN; the viewport centre is (width/2, height/2).
//			Coordinates are continuous (a pixel's centre is x + 0.5).
//
//			2D views. Each view maps two world axes to the screen: u to the
//			right and v UP (so screen y and v run opposite ways). The mapping
//			is the one EditorController::ViewAxes uses, so tools agree:
//				Top   = X / Y, free axis Z, looking down -Z
//				Front = X / Z, free axis Y, looking along +Y
//				Side  = Y / Z, free axis X, looking along -X
//			(the view direction is -(u x v), toward the viewer's back). A
//			screen point has no unique 3D inverse: ScreenToPlane returns the
//			two view-axis coordinates and PlaneToWorld builds a 3D point from
//			them plus a caller-chosen depth on the free axis.
//
//			3D view. Source angles in degrees: yaw turns about +Z starting at
//			+X (yaw 90 looks along +Y), pitch positive looks DOWN (as Source's
//			QAngle), clamped to +/-kMaxPitch; there is no roll. Forward =
//			(cos p cos y, cos p sin y, -sin p), Right = (sin y, -cos y, 0),
//			Up = Right x Forward. The vertical field of view is in degrees.
//
//			Both cameras are values: no allocation, no host or GL state. A
//			zero-sized viewport is valid but degenerate: queries that need an
//			aspect ratio or a pixel scale report nothing (or false).
//
//=============================================================================//

#ifndef HAMMER_VIEWPORT_CAMERA_H
#define HAMMER_VIEWPORT_CAMERA_H

#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/brush.h"

#include <optional>

namespace hammer::viewport
{

enum class ViewKind
{
	Camera3D,
	Top,   // X / Y (free axis Z)
	Front, // X / Z (free axis Y)
	Side,  // Y / Z (free axis X)
};

// The world axis indices (0 = X, 1 = Y, 2 = Z) a view maps to screen right
// (u), screen up (v), and the free (depth) axis. Camera3D reports the Top
// mapping, as EditorController::ViewAxes does.
struct ViewAxes
{
	int u = 0;
	int v = 1;
	int depth = 2;
};
ViewAxes AxesOf( ViewKind kind );

// The unit direction a 2D view looks along: -(u x v). Camera3D reports Top's.
mapgeometry::Vec3d ViewDirection( ViewKind kind );

struct ScreenPoint
{
	double x = 0.0;
	double y = 0.0;

	friend bool operator==( const ScreenPoint &, const ScreenPoint & ) = default;
};

// World coordinates on a 2D view's two axes.
struct PlanePoint
{
	double u = 0.0;
	double v = 0.0;

	friend bool operator==( const PlanePoint &, const PlanePoint & ) = default;
};

// An axis-aligned rectangle in a view's plane coordinates (min <= max).
struct PlaneRect
{
	PlanePoint min;
	PlanePoint max;
};

class Camera2D
{
public:
	// Zoom is pixels per world unit. The range is the legacy editor's
	// (mapview2dbase ZOOM_MAX 256) with a 1/64 floor (a 64k-unit map in 1024 px).
	static constexpr double kMinZoom = 1.0 / 64.0;
	static constexpr double kMaxZoom = 256.0;
	static constexpr double kDefaultZoom = 0.25;

	// A Top view centred on the origin with a zero-sized viewport.
	Camera2D() = default;

	ViewKind Kind() const { return m_kind; }
	// Selects Top, Front or Side. Camera3D is not a 2D view: returns false and
	// changes nothing.
	bool SetKind( ViewKind kind );
	ViewAxes Axes() const { return AxesOf( m_kind ); }

	PlanePoint Center() const { return m_center; }
	// Returns false (unchanged) for a non-finite centre.
	bool SetCenter( PlanePoint center );

	double Zoom() const { return m_zoom; }
	// Clamps to [kMinZoom, kMaxZoom]. Returns false (unchanged) for a
	// non-finite or non-positive value.
	bool SetZoom( double pixelsPerUnit );

	int Width() const { return m_width; }
	int Height() const { return m_height; }
	// Negative sizes are stored as zero.
	void SetViewport( int width, int height );
	bool HasArea() const { return m_width > 0 && m_height > 0; }

	// --- Projection ----------------------------------------------------------
	PlanePoint ToPlane( const mapgeometry::Vec3d &world ) const;
	double DepthOf( const mapgeometry::Vec3d &world ) const;
	mapgeometry::Vec3d PlaneToWorld( PlanePoint plane, double depth ) const;

	ScreenPoint PlaneToScreen( PlanePoint plane ) const;
	ScreenPoint WorldToScreen( const mapgeometry::Vec3d &world ) const;
	// The view-axis coordinates under a pixel (the free axis is unknown).
	PlanePoint ScreenToPlane( double px, double py ) const;
	// ScreenToPlane plus a caller-chosen free-axis depth.
	mapgeometry::Vec3d ScreenToWorld( double px, double py, double depth ) const;

	// The plane rectangle the viewport shows.
	PlaneRect VisibleRect() const;

	// --- Navigation ----------------------------------------------------------
	// Drags the content with the pointer: the world point under a pixel moves
	// by (dx, dy) pixels. Non-finite deltas are ignored (returns false).
	bool PanPixels( double dx, double dy );
	// Multiplies the zoom by 'factor' (> 1 zooms in), clamped, keeping the
	// world point under (px, py) fixed on screen. Returns false (unchanged)
	// for a non-finite or non-positive factor or a non-finite pixel.
	bool ZoomAt( double px, double py, double factor );
	// Centres 'box' and picks the largest zoom that shows its extent on the
	// view axes with 'marginPixels' free on every side (a flat or point box
	// keeps the current zoom on its zero-extent axes). Returns false
	// (unchanged) without a viewport larger than twice the margin, for a
	// negative or non-finite margin, or for an inverted/non-finite box.
	bool Frame( const scene::Box &box, double marginPixels );

private:
	ViewKind m_kind = ViewKind::Top;
	PlanePoint m_center;
	double m_zoom = kDefaultZoom;
	int m_width = 0;
	int m_height = 0;
};

struct Ray
{
	mapgeometry::Vec3d origin;
	mapgeometry::Vec3d direction; // unit length
};

class Camera3D
{
public:
	static constexpr double kMaxPitch = 89.0;
	static constexpr double kMinFov = 1.0;
	static constexpr double kMaxFov = 170.0;
	static constexpr double kDefaultFov = 60.0; // the GTK shell's projection
	// Points closer than this along Forward (or behind) do not project.
	static constexpr double kNearDepth = 1.0e-3;

	Camera3D() = default;

	const mapgeometry::Vec3d &Position() const { return m_position; }
	// Returns false (unchanged) for a non-finite position.
	bool SetPosition( const mapgeometry::Vec3d &position );

	double Yaw() const { return m_yaw; }
	double Pitch() const { return m_pitch; }
	// Yaw is normalized to (-180, 180]; pitch is clamped to +/-kMaxPitch.
	// Returns false (unchanged) for non-finite angles.
	bool SetAngles( double yawDegrees, double pitchDegrees );

	double Fov() const { return m_fov; }
	// Vertical field of view, clamped to [kMinFov, kMaxFov]. Returns false
	// (unchanged) for a non-finite value.
	bool SetFov( double degrees );

	int Width() const { return m_width; }
	int Height() const { return m_height; }
	void SetViewport( int width, int height );
	bool HasArea() const { return m_width > 0 && m_height > 0; }

	mapgeometry::Vec3d Forward() const;
	mapgeometry::Vec3d Right() const;
	mapgeometry::Vec3d Up() const;

	// The ray from the eye through a pixel (the centre pixel gives Forward).
	// Nothing without a viewport or for a non-finite pixel.
	std::optional<Ray> RayThroughPixel( double px, double py ) const;
	// The pixel a world point projects to; nothing without a viewport or when
	// the point is behind the camera (depth <= kNearDepth). Points outside the
	// frustum's sides still project (off-screen coordinates).
	std::optional<ScreenPoint> WorldToScreen( const mapgeometry::Vec3d &world ) const;

	// Moves the eye 'forward' along Forward, 'right' along Right and 'up'
	// along world +Z (the legacy Z-fly / WASD convention). Non-finite input
	// is ignored (returns false).
	bool Fly( double forward, double right, double up );
	// Turns in place (the eye stays fixed); pitch clamps.
	bool Look( double dyawDegrees, double dpitchDegrees );
	// Turns the camera about 'pivot' keeping its distance to the pivot: the
	// angles change by the deltas (pitch clamped) and the eye moves to
	// pivot - Forward * distance, so afterwards the camera looks at the pivot.
	// With the eye at the pivot it only turns.
	bool Orbit( const mapgeometry::Vec3d &pivot, double dyawDegrees, double dpitchDegrees );
	// Aims at 'target' from the current eye (pitch clamped). Returns false
	// (unchanged) when the target is the eye or non-finite.
	bool LookAt( const mapgeometry::Vec3d &target );
	// Keeps the angles and moves the eye back along Forward until the box's
	// bounding sphere fits the narrower of the vertical and horizontal fields
	// of view (a point box is framed as a sphere of radius 1). Returns false
	// (unchanged) for an inverted or non-finite box.
	bool Frame( const scene::Box &box );

private:
	mapgeometry::Vec3d m_position;
	double m_yaw = 0.0;
	double m_pitch = 0.0;
	double m_fov = kDefaultFov;
	int m_width = 0;
	int m_height = 0;
};

} // namespace hammer::viewport

#endif // HAMMER_VIEWPORT_CAMERA_H
