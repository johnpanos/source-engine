//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor grid (RFC 0002, hammer.viewport): the one snapping
//			policy tools share and the grid lines a 2D view shows.
//
//			Snapping. GridPolicy holds the grid size, a power of two from 1 to
//			1024 units (legacy Hammer's range; default 64), and whether snapping
//			is enabled. Snap rounds to the nearest multiple with ties away from
//			zero (legacy V_rint, as mapgeometry::RoundHalfAwayFromZero, here in
//			double precision); a disabled policy returns its input unchanged.
//
//			Display. VisibleLines lists the lines a Camera2D shows: one line for
//			every multiple of the displayed step inside the visible rectangle
//			(clipped to +/- worldExtent). The displayed step starts at the grid
//			size and doubles until neighbouring lines are at least
//			minSpacingPixels apart (default 4, legacy "hide small grid" below
//			4 px), then keeps doubling while either axis would exceed
//			maxLinesPerAxis lines, so the result is always bounded. Each line is
//			classified by its world coordinate: Axis at 0, Block on multiples of
//			blockEvery (legacy "highlight 1024"), Major on multiples of
//			majorEvery (legacy "highlight 64"), otherwise Minor; the first rule
//			that applies wins. Display is independent of snapping: a disabled
//			snap still shows the grid (the legacy toggles are separate).
//
//=============================================================================//

#ifndef HAMMER_VIEWPORT_GRID_H
#define HAMMER_VIEWPORT_GRID_H

#include "hammer/viewport/camera.h"
#include "mapgeometry/brush.h"

#include <cstddef>
#include <vector>

namespace hammer::viewport
{

// Which world axes SnapPoint snaps.
struct AxisMask
{
	bool x = true;
	bool y = true;
	bool z = true;
};

class GridPolicy
{
public:
	static constexpr int kMinSize = 1;
	static constexpr int kMaxSize = 1024;
	static constexpr int kDefaultSize = 64;

	int Size() const { return m_size; }
	// Accepts a power of two in [kMinSize, kMaxSize]; returns false and keeps
	// the size otherwise.
	bool SetSize( int size );
	// Halve / double the size, clamped to the range. Return whether it changed.
	bool StepFiner();
	bool StepCoarser();

	bool Enabled() const { return m_enabled; }
	void SetEnabled( bool enabled ) { m_enabled = enabled; }

	// The nearest multiple of Size() (ties away from zero); the input itself
	// when disabled or non-finite.
	double Snap( double value ) const;
	// Snaps the axes 'mask' selects.
	mapgeometry::Vec3d SnapPoint( const mapgeometry::Vec3d &point, AxisMask mask = {} ) const;

private:
	int m_size = kDefaultSize;
	bool m_enabled = true;
};

enum class GridLineKind
{
	Minor,
	Major, // multiples of majorEvery
	Block, // multiples of blockEvery
	Axis,  // the world axis (coordinate 0)
};

// Which screen direction a line runs in.
enum class GridLineOrientation
{
	Vertical,   // constant u: screen x = 'screen'
	Horizontal, // constant v: screen y = 'screen'
};

struct GridLine
{
	GridLineKind kind = GridLineKind::Minor;
	GridLineOrientation orientation = GridLineOrientation::Vertical;
	double world = 0.0;  // the constant u or v coordinate
	double screen = 0.0; // the constant screen x or y

	friend bool operator==( const GridLine &, const GridLine & ) = default;
};

struct GridDisplay
{
	bool show = true;
	double minSpacingPixels = 4.0;
	int majorEvery = 64;   // 0 disables Major
	int blockEvery = 1024; // 0 disables Block
	bool axisLines = true;
	std::size_t maxLinesPerAxis = 512;
	double worldExtent = 16384.0; // lines are clipped to +/- this; 0 = unbounded
};

// The displayed line step (world units) for 'camera' before the per-axis line
// cap: the grid size doubled until lines are minSpacingPixels apart.
double DisplayedStep( const Camera2D &camera, const GridPolicy &grid, const GridDisplay &display );

// The visible grid lines: vertical lines in increasing u, then horizontal
// lines in increasing v. Empty when display.show is false or the camera has
// no viewport. At most 2 * maxLinesPerAxis lines (maxLinesPerAxis 0 = 1).
std::vector<GridLine> VisibleLines(
    const Camera2D &camera, const GridPolicy &grid, const GridDisplay &display = {} );

} // namespace hammer::viewport

#endif // HAMMER_VIEWPORT_GRID_H
