//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.viewport grid (RFC 0002, R08 domain logic; contract
//			viewport.grid.v1): the snapping policy (legacy powers of two,
//			ties away from zero, per-axis masks, disabled = identity) and the
//			visible grid lines of a 2D camera (spacing doubling, Major/Block/
//			Axis classification, world-extent clipping and the per-axis cap).
//			Negative checks: rejected sizes, clamped steps, empty viewports,
//			hidden display, and a seeded over-dense grid the spacing oracle
//			must detect.
//
//=============================================================================//

#include "hammer/viewport/grid.h"
#include "testing/checks.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace hammer::viewport;
using mapgeometry::Vec3d;

namespace
{

// Oracle: neighbouring lines of one orientation are at least 'minPixels' apart.
bool SpacedAtLeast( const std::vector<GridLine> &lines, double minPixels )
{
	for ( GridLineOrientation orientation :
	    { GridLineOrientation::Vertical, GridLineOrientation::Horizontal } )
	{
		std::vector<double> positions;
		for ( const GridLine &line : lines )
		{
			if ( line.orientation == orientation )
			{
				positions.push_back( line.screen );
			}
		}
		std::sort( positions.begin(), positions.end() );
		for ( std::size_t i = 1; i < positions.size(); ++i )
		{
			if ( positions[i] - positions[i - 1] < minPixels - 1.0e-9 )
			{
				return false;
			}
		}
	}
	return true;
}

std::size_t CountOf( const std::vector<GridLine> &lines, GridLineOrientation orientation )
{
	return static_cast<std::size_t>( std::count_if( lines.begin(), lines.end(),
	    [&]( const GridLine &line )
	    {
		    return line.orientation == orientation;
	    } ) );
}

const GridLine *LineAt(
    const std::vector<GridLine> &lines, GridLineOrientation orientation, double world )
{
	for ( const GridLine &line : lines )
	{
		if ( line.orientation == orientation && line.world == world )
		{
			return &line;
		}
	}
	return nullptr;
}

void Snapping( testing::Checks &checks )
{
	GridPolicy grid;
	checks.Equal( grid.Size(), 64, "default grid is 64" );
	checks.That( grid.Enabled(), "snapping enabled by default" );

	checks.That( !grid.SetSize( 0 ), "size 0 rejected (negative)" );
	checks.That( !grid.SetSize( 48 ), "non power of two rejected (negative)" );
	checks.That( !grid.SetSize( 2048 ), "size above 1024 rejected (negative)" );
	checks.That( !grid.SetSize( -8 ), "negative size rejected (negative)" );
	checks.Equal( grid.Size(), 64, "rejected sizes keep the grid" );
	checks.That( grid.SetSize( 16 ), "16 accepted" );

	checks.Near( grid.Snap( 7.9 ), 0.0, 0.0, "snap below half rounds down" );
	checks.Near( grid.Snap( 8.0 ), 16.0, 0.0, "exact half rounds away from zero (up)" );
	checks.Near( grid.Snap( -8.0 ), -16.0, 0.0, "exact half rounds away from zero (down)" );
	checks.Near( grid.Snap( -23.0 ), -16.0, 0.0, "negative snap" );
	checks.Near( grid.Snap( 1000.0 ), 1008.0, 0.0, "snap to nearest multiple" );
	const double nan = std::numeric_limits<double>::quiet_NaN();
	checks.That( std::isnan( grid.Snap( nan ) ), "NaN passes through unchanged (negative)" );

	const Vec3d snapped = grid.SnapPoint( Vec3d( 5, 9, 31 ) );
	checks.That( snapped == Vec3d( 0, 16, 32 ), "SnapPoint snaps every axis" );
	const Vec3d masked = grid.SnapPoint( Vec3d( 5, 9, 31 ), AxisMask{ true, false, true } );
	checks.That( masked == Vec3d( 0, 9, 32 ), "SnapPoint honours the axis mask" );

	grid.SetEnabled( false );
	checks.Near( grid.Snap( 7.25 ), 7.25, 0.0, "disabled snapping is the identity" );
	checks.That( grid.SnapPoint( Vec3d( 1.5, 2.5, 3.5 ) ) == Vec3d( 1.5, 2.5, 3.5 ),
	    "disabled SnapPoint is the identity" );
	grid.SetEnabled( true );

	grid.SetSize( 2 );
	checks.That( grid.StepFiner() && grid.Size() == 1, "StepFiner halves" );
	checks.That( !grid.StepFiner() && grid.Size() == 1, "StepFiner clamps at 1 (negative)" );
	grid.SetSize( 512 );
	checks.That( grid.StepCoarser() && grid.Size() == 1024, "StepCoarser doubles" );
	checks.That(
	    !grid.StepCoarser() && grid.Size() == 1024, "StepCoarser clamps at 1024 (negative)" );
	GridPolicy walk;
	int steps = 0;
	while ( walk.StepFiner() )
	{
		++steps;
	}
	checks.Equal( steps, 6, "64 -> 1 takes six halvings" );
}

void Lines( testing::Checks &checks )
{
	Camera2D camera;
	camera.SetViewport( 800, 600 );
	camera.SetZoom( 1.0 );
	GridPolicy grid;
	grid.SetSize( 16 );

	std::vector<GridLine> lines = VisibleLines( camera, grid );
	// u in [-400, 400], v in [-300, 300], step 16 at 1 px/unit.
	checks.Equal(
	    CountOf( lines, GridLineOrientation::Vertical ), std::size_t( 51 ), "vertical line count" );
	checks.Equal( CountOf( lines, GridLineOrientation::Horizontal ), std::size_t( 37 ),
	    "horizontal line count" );
	checks.That( SpacedAtLeast( lines, 16.0 ), "16 px spacing at 1 px/unit" );
	const GridLine *axis = LineAt( lines, GridLineOrientation::Vertical, 0.0 );
	checks.That(
	    axis && axis->kind == GridLineKind::Axis && axis->screen == 400.0, "axis line at u = 0" );
	const GridLine *major = LineAt( lines, GridLineOrientation::Vertical, 128.0 );
	checks.That( major && major->kind == GridLineKind::Major, "every 64 units is Major" );
	const GridLine *minor = LineAt( lines, GridLineOrientation::Horizontal, 48.0 );
	checks.That( minor && minor->kind == GridLineKind::Minor && minor->screen == 252.0,
	    "minor horizontal line at v = 48 is at y = 252" );
	checks.That( std::is_sorted( lines.begin(), lines.begin() + 51,
	                 []( const GridLine &a, const GridLine &b )
	                 {
		                 return a.world < b.world;
	                 } ),
	    "vertical lines in increasing u" );

	// Zoomed out: 16 units at 1/8 px/unit = 2 px, so the step doubles to 32 (4 px).
	camera.SetZoom( 0.125 );
	checks.Near( DisplayedStep( camera, grid, {} ), 32.0, 0.0, "step doubles until 4 px apart" );
	lines = VisibleLines( camera, grid );
	checks.That( SpacedAtLeast( lines, 4.0 ), "zoomed-out lines are at least 4 px apart" );
	const GridLine *block = LineAt( lines, GridLineOrientation::Vertical, 1024.0 );
	checks.That( block && block->kind == GridLineKind::Block, "every 1024 units is Block" );
	GridDisplay noBlock;
	noBlock.blockEvery = 0;
	lines = VisibleLines( camera, grid, noBlock );
	block = LineAt( lines, GridLineOrientation::Vertical, 1024.0 );
	checks.That( block && block->kind == GridLineKind::Major, "blockEvery 0 falls back to Major" );
	GridDisplay noAxis;
	noAxis.axisLines = false;
	lines = VisibleLines( camera, grid, noAxis );
	axis = LineAt( lines, GridLineOrientation::Vertical, 0.0 );
	checks.That(
	    axis && axis->kind == GridLineKind::Block, "without axis lines 0 is a Block line" );

	// Seeded over-dense display: the spacing oracle must reject it.
	GridDisplay dense;
	dense.minSpacingPixels = 0.0;
	checks.That( !SpacedAtLeast( VisibleLines( camera, grid, dense ), 4.0 ),
	    "oracle detects a 2 px grid (seeded)" );

	// Snapping disabled: display is independent.
	GridPolicy off = grid;
	off.SetEnabled( false );
	checks.That( VisibleLines( camera, off ) == VisibleLines( camera, grid ),
	    "display does not depend on the snap toggle" );
	GridDisplay hidden;
	hidden.show = false;
	checks.That(
	    VisibleLines( camera, grid, hidden ).empty(), "show = false draws nothing (negative)" );
	Camera2D empty;
	checks.That( VisibleLines( empty, grid ).empty(), "no viewport, no lines (negative)" );
}

void Bounds( testing::Checks &checks )
{
	// World extent clipping: a view far outside the map shows nothing.
	Camera2D far;
	far.SetViewport( 400, 400 );
	far.SetZoom( 1.0 );
	far.SetCenter( { 100000.0, 0.0 } );
	GridPolicy grid;
	checks.Equal( CountOf( VisibleLines( far, grid ), GridLineOrientation::Vertical ),
	    std::size_t( 0 ), "no vertical lines beyond the world extent" );
	GridDisplay unbounded;
	unbounded.worldExtent = 0.0;
	checks.That( CountOf( VisibleLines( far, grid, unbounded ), GridLineOrientation::Vertical ) > 0,
	    "worldExtent 0 is unbounded" );

	// The cap: a huge viewport at the finest grid never exceeds maxLinesPerAxis.
	Camera2D huge;
	huge.SetViewport( 2000000, 1500000 );
	huge.SetZoom( Camera2D::kMaxZoom );
	GridPolicy fine;
	fine.SetSize( 1 );
	GridDisplay capped;
	capped.worldExtent = 0.0;
	const std::vector<GridLine> lines = VisibleLines( huge, fine, capped );
	checks.That( CountOf( lines, GridLineOrientation::Vertical ) <= capped.maxLinesPerAxis &&
	                 CountOf( lines, GridLineOrientation::Horizontal ) <= capped.maxLinesPerAxis,
	    "line count is capped per axis" );
	checks.That( CountOf( lines, GridLineOrientation::Vertical ) > capped.maxLinesPerAxis / 4,
	    "the cap coarsens rather than dropping the grid" );
	GridDisplay tiny;
	tiny.maxLinesPerAxis = 0;
	tiny.worldExtent = 0.0;
	const std::vector<GridLine> one = VisibleLines( huge, fine, tiny );
	checks.That( CountOf( one, GridLineOrientation::Vertical ) <= 1 &&
	                 CountOf( one, GridLineOrientation::Horizontal ) <= 1,
	    "maxLinesPerAxis 0 behaves as 1" );

	// Zoomed fully out over the whole world: bounded and spaced.
	Camera2D out;
	out.SetViewport( 1920, 1080 );
	out.SetZoom( Camera2D::kMinZoom );
	const std::vector<GridLine> world = VisibleLines( out, fine );
	checks.That( world.size() <= 2 * GridDisplay{}.maxLinesPerAxis && SpacedAtLeast( world, 4.0 ),
	    "minimum zoom stays bounded and spaced" );
	for ( const GridLine &line : world )
	{
		if ( std::fabs( line.world ) > 16384.0 )
		{
			checks.That( false, "a line lies outside the world extent" );
			break;
		}
	}
	GridDisplay nanSpacing;
	nanSpacing.minSpacingPixels = std::numeric_limits<double>::quiet_NaN();
	checks.Near( DisplayedStep( out, fine, nanSpacing ), 1.0, 0.0,
	    "NaN spacing is treated as no minimum (negative)" );
}

} // namespace

int main()
{
	testing::Checks checks;
	Snapping( checks );
	Lines( checks );
	Bounds( checks );
	return checks.Report();
}
