//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/interaction_policy.h.
//
//=============================================================================//

#include "hammer/tools/interaction_policy.h"

#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>

namespace hammer::tools
{

using mapgeometry::Vec3d;
using viewport::PlanePoint;
using viewport::ScreenPoint;

bool ExceedsDragThreshold( ScreenPoint down, ScreenPoint now, double thresholdPixels )
{
	return std::fabs( now.x - down.x ) > thresholdPixels ||
	       std::fabs( now.y - down.y ) > thresholdPixels;
}

double HandleDistance( ScreenPoint a, ScreenPoint b )
{
	return std::max( std::fabs( a.x - b.x ), std::fabs( a.y - b.y ) );
}

bool WithinHandle( ScreenPoint center, ScreenPoint point, double radius )
{
	return HandleDistance( center, point ) <= radius;
}

bool SnapActive( const viewport::GridPolicy &grid, Modifiers modifiers )
{
	return grid.Enabled() && !modifiers.Alt();
}

double SnapValue( const viewport::GridPolicy &grid, double value, Modifiers modifiers )
{
	return SnapActive( grid, modifiers ) ? grid.Snap( value ) : value;
}

PlanePoint SnapPlanePoint( const viewport::GridPolicy &grid, PlanePoint point, Modifiers modifiers )
{
	return { SnapValue( grid, point.u, modifiers ), SnapValue( grid, point.v, modifiers ) };
}

Vec3d SnapWorldPoint( const viewport::GridPolicy &grid, const Vec3d &point, Modifiers modifiers,
    viewport::AxisMask mask )
{
	return SnapActive( grid, modifiers ) ? grid.SnapPoint( point, mask ) : point;
}

PlanePoint ConstrainToDominantAxis( PlanePoint delta )
{
	if ( std::fabs( delta.v ) > std::fabs( delta.u ) )
		return { 0.0, delta.v };
	return { delta.u, 0.0 };
}

PlanePoint DragDelta( const viewport::GridPolicy &grid, PlanePoint reference, PlanePoint rawDelta,
    Modifiers modifiers )
{
	const bool constrain = modifiers.Shift();
	const bool dominantU = std::fabs( rawDelta.u ) >= std::fabs( rawDelta.v );
	const PlanePoint delta = constrain ? ConstrainToDominantAxis( rawDelta ) : rawDelta;
	const bool keepU = !constrain || dominantU;
	const bool keepV = !constrain || !dominantU;
	PlanePoint out;
	out.u = keepU ? SnapValue( grid, reference.u + delta.u, modifiers ) - reference.u : 0.0;
	out.v = keepV ? SnapValue( grid, reference.v + delta.v, modifiers ) - reference.v : 0.0;
	return out;
}

Vec3d PlaneDeltaToWorld( viewport::ViewAxes axes, PlanePoint delta )
{
	Vec3d out;
	mapgeometry::SetComponent( out, axes.u, delta.u );
	mapgeometry::SetComponent( out, axes.v, delta.v );
	return out;
}

double SnapAngle( double degrees, Modifiers modifiers )
{
	if ( modifiers.Alt() || !std::isfinite( degrees ) )
		return degrees;
	const double step = modifiers.Shift() ? kRotateSnapDegrees : kRotateFineDegrees;
	const double steps = degrees / step;
	const double rounded = steps < 0.0 ? -std::floor( -steps + 0.5 ) : std::floor( steps + 0.5 );
	return rounded * step;
}

std::optional<Vec3d> NudgeDelta( viewport::ViewKind view, Key key, Modifiers modifiers,
    const viewport::GridPolicy &grid, NudgeStep step )
{
	double du = 0.0;
	double dv = 0.0;
	switch ( key )
	{
	case Key::Right:
		du = 1.0;
		break;
	case Key::Left:
		du = -1.0;
		break;
	case Key::Up:
		dv = 1.0;
		break;
	case Key::Down:
		dv = -1.0;
		break;
	default:
		return std::nullopt;
	}
	const bool gridStep = step == NudgeStep::Grid && grid.Enabled() && !modifiers.Ctrl();
	const double unit = gridStep ? static_cast<double>( grid.Size() ) : 1.0;
	return PlaneDeltaToWorld( viewport::AxesOf( view ), { du * unit, dv * unit } );
}

namespace
{

scene::ObjectId Resolve( const scene::DocumentReader &doc, scene::ObjectId object,
    scene::ObjectId owner, app::SelectionGranularity granularity )
{
	const scene::ObjectId resolved = app::ResolvePick( doc, object, granularity );
	if ( resolved.IsValid() )
		return resolved;
	return owner.IsValid() ? owner : object;
}

} // namespace

std::optional<ObjectPick> PickObject2D( const scene::DocumentReader &doc,
    const viewport::Camera2D &camera, double x, double y, const PickSettings &settings )
{
	viewport::PickOptions options;
	options.catalog = settings.catalog;
	options.pointHalfSize = settings.pointHalfSize;
	options.solids = settings.solids;
	options.entities = settings.entities;
	const std::vector<viewport::Hit2D> hits = viewport::Pick2D( doc, camera, x, y, options );
	if ( hits.empty() )
		return std::nullopt;
	ObjectPick pick;
	pick.object = hits.front().object;
	pick.target = Resolve( doc, hits.front().object, hits.front().owner, settings.granularity );
	return pick;
}

std::optional<ObjectPick> PickObject3D( const scene::DocumentReader &doc,
    const viewport::Camera3D &camera, double x, double y, const PickSettings &settings )
{
	const std::optional<viewport::Ray> ray = camera.RayThroughPixel( x, y );
	if ( !ray )
		return std::nullopt;
	viewport::PickOptions options;
	options.catalog = settings.catalog;
	options.pointHalfSize = settings.pointHalfSize;
	options.solids = settings.solids;
	options.entities = settings.entities;
	const std::vector<viewport::RayHit> hits =
	    viewport::PickRay( doc, ray->origin, ray->direction, options );
	if ( hits.empty() )
		return std::nullopt;
	const viewport::RayHit &hit = hits.front();
	ObjectPick pick;
	pick.object = hit.object;
	pick.target = Resolve( doc, hit.object, hit.owner, settings.granularity );
	pick.face = hit.face;
	pick.hasPoint = true;
	pick.point = hit.point;
	pick.normal = hit.normal;
	return pick;
}

std::vector<scene::ObjectId> MarqueeTargets( const scene::DocumentReader &doc,
    const viewport::Camera2D &camera, const viewport::ScreenRect &rect, viewport::MarqueeMode mode,
    const PickSettings &settings )
{
	viewport::MarqueeOptions options;
	options.catalog = settings.catalog;
	options.pointHalfSize = settings.pointHalfSize;
	options.solids = settings.solids;
	options.entities = settings.entities;
	options.ownersForBrushEntities = settings.granularity != app::SelectionGranularity::Solids;
	std::vector<scene::ObjectId> out;
	for ( scene::ObjectId id : viewport::MarqueeSelect( doc, camera, rect, mode, options ) )
		out.push_back( Resolve( doc, id, scene::ObjectId(), settings.granularity ) );
	std::sort( out.begin(), out.end() );
	out.erase( std::unique( out.begin(), out.end() ), out.end() );
	return out;
}

} // namespace hammer::tools
