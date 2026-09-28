//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/displacement_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/displacement_ops.h"

#include "hammer/scene/displacement_geometry.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>

namespace hammer::app::ops
{

using mapgeometry::Vec3d;
using scene::FaceRef;

namespace
{

constexpr double kWalkableNormalZ = 0.7;
constexpr double kBuildableNormalZ = 0.8;
constexpr double kSewEpsilon = 0.01;
constexpr int kForceBits = kDispTagForceWalkableBit | kDispTagForceWalkableValue |
                           kDispTagForceBuildableBit | kDispTagForceBuildableValue;

// Index of the side a face reference names, or nothing.
std::optional<std::size_t> SideIndex( const scene::Solid &solid, std::uint32_t sideId )
{
	for ( std::size_t i = 0; i < solid.sides.size(); ++i )
	{
		if ( solid.sides[i].vmfId == sideId )
		{
			return i;
		}
	}
	return std::nullopt;
}

struct Target
{
	FaceRef face;
	std::size_t side = 0;
};

// Resolves every face before any write; 'displaced' requires (or forbids)
// an existing displacement.
EditResult Resolve( const scene::DocumentEdit &edit, const std::vector<FaceRef> &faces,
    bool displaced, std::vector<Target> &out )
{
	if ( faces.empty() )
	{
		return NothingToDo( "no faces selected" );
	}
	for ( const FaceRef &f : faces )
	{
		const scene::Solid *solid = edit.FindSolid( f.solid );
		const std::optional<std::size_t> index = solid ? SideIndex( *solid, f.side ) : std::nullopt;
		if ( !index )
		{
			return Reject( "unknown face " + std::to_string( f.side ) );
		}
		if ( solid->sides[*index].displacement.has_value() != displaced )
		{
			return Reject(
			    displaced ? "face " + std::to_string( f.side ) + " is not a displacement"
			              : "face " + std::to_string( f.side ) + " is already a displacement" );
		}
		out.push_back( { f, *index } );
	}
	return {};
}

// Displaced minus base, offset and elevation: the displacement vector.
Vec3d DisplacementVector( const scene::Displacement &d, std::size_t i )
{
	const Vec3d n = d.normals ? ( *d.normals )[i] : Vec3d();
	const double dist = d.distances ? ( *d.distances )[i] : 0.0;
	return n * dist;
}

// Stores 'vectors' as unit normals and distances. A zero vector keeps the
// previous normal (or 'fallback') with distance 0.
void StoreVectors(
    scene::Displacement &d, const std::vector<Vec3d> &vectors, const Vec3d &fallback )
{
	std::vector<Vec3d> normals( vectors.size() );
	std::vector<double> distances( vectors.size() );
	for ( std::size_t i = 0; i < vectors.size(); ++i )
	{
		const double len = mapgeometry::Length( vectors[i] );
		if ( len > 1.0e-9 )
		{
			normals[i] = vectors[i] / len;
			distances[i] = len;
		}
		else
		{
			normals[i] = ( d.normals && i < d.normals->size() &&
			                 mapgeometry::Length( ( *d.normals )[i] ) > 0.5 )
			                 ? ( *d.normals )[i]
			                 : fallback;
			distances[i] = 0.0;
		}
	}
	d.normals = std::move( normals );
	d.distances = std::move( distances );
}

void RefreshTags( scene::Solid &solid, std::size_t side )
{
	scene::Displacement &d = *solid.sides[side].displacement;
	const std::vector<int> previous = d.triangleTags ? *d.triangleTags : std::vector<int>();
	d.triangleTags = ComputeTriangleTags( solid, side, previous.empty() ? nullptr : &previous );
}

double Weight( double distance, double radius, bool falloff )
{
	if ( distance > radius )
	{
		return 0.0;
	}
	return falloff ? 1.0 - distance / radius : 1.0;
}

// Bilinear sample of a row-major grid of side 'n' at fractional (row, col).
template <typename T> T Sample( const std::vector<T> &grid, int n, double row, double col )
{
	const int r0 = std::clamp( static_cast<int>( std::floor( row ) ), 0, n - 1 );
	const int c0 = std::clamp( static_cast<int>( std::floor( col ) ), 0, n - 1 );
	const int r1 = std::min( r0 + 1, n - 1 );
	const int c1 = std::min( c0 + 1, n - 1 );
	const double fr = row - r0;
	const double fc = col - c0;
	auto at = [&]( int r, int c )
	{
		return grid[static_cast<std::size_t>( r * n + c )];
	};
	return ( at( r0, c0 ) * ( 1 - fc ) + at( r0, c1 ) * fc ) * ( 1 - fr ) +
	       ( at( r1, c0 ) * ( 1 - fc ) + at( r1, c1 ) * fc ) * fr;
}

} // namespace

std::vector<int> ComputeTriangleTags(
    const scene::Solid &solid, std::size_t sideIndex, const std::vector<int> *previous )
{
	const std::optional<mapgeometry::DisplacementSurface> surface =
	    scene::BuildDisplacement( solid, sideIndex );
	if ( !surface )
	{
		return {};
	}
	const Vec3d faceNormal = solid.sides[sideIndex].Plane().normal;
	std::vector<int> tags( surface->triangles.size(), 0 );
	for ( std::size_t t = 0; t < surface->triangles.size(); ++t )
	{
		const auto &tri = surface->triangles[t];
		const Vec3d a = surface->vertices[static_cast<std::size_t>( tri[0] )];
		const Vec3d b = surface->vertices[static_cast<std::size_t>( tri[1] )];
		const Vec3d c = surface->vertices[static_cast<std::size_t>( tri[2] )];
		Vec3d n = mapgeometry::Normalize( mapgeometry::Cross( b - a, c - a ) );
		if ( mapgeometry::Dot( n, faceNormal ) < 0.0 )
		{
			n = -n;
		}
		int tag = 0;
		if ( n.z >= kWalkableNormalZ )
		{
			tag |= kDispTagWalkable;
		}
		if ( n.z >= kBuildableNormalZ )
		{
			tag |= kDispTagBuildable;
		}
		if ( previous && t < previous->size() )
		{
			tag |= ( *previous )[t] & kForceBits;
		}
		tags[t] = tag;
	}
	return tags;
}

EditResult CreateDisplacement(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, int power )
{
	if ( power < 2 || power > 4 )
	{
		return Reject( "displacement power is 2, 3 or 4" );
	}
	std::vector<Target> targets;
	if ( EditResult r = Resolve( edit, faces, false, targets ); !r )
	{
		return r;
	}
	for ( const Target &t : targets )
	{
		if ( !scene::QuadCorners( *edit.FindSolid( t.face.solid ), t.side ) )
		{
			return Reject( "face " + std::to_string( t.face.side ) + " is not a quadrilateral" );
		}
	}
	for ( const Target &t : targets )
	{
		scene::Solid *solid = edit.MutableSolid( t.face.solid );
		const std::array<Vec3d, 4> corners = *scene::QuadCorners( *solid, t.side );
		const Vec3d start = *std::min_element( corners.begin(), corners.end(),
		    []( const Vec3d &a, const Vec3d &b )
		    {
			    return std::tie( a.x, a.y, a.z ) < std::tie( b.x, b.y, b.z );
		    } );
		const Vec3d normal = solid->sides[t.side].Plane().normal;
		scene::Displacement d;
		d.power = power;
		d.startPosition = start;
		const std::size_t verts = static_cast<std::size_t>( d.VertsPerRow() * d.VertsPerRow() );
		d.normals = std::vector<Vec3d>( verts, normal );
		d.distances = std::vector<double>( verts, 0.0 );
		d.offsets = std::vector<Vec3d>( verts );
		d.offsetNormals = std::vector<Vec3d>( verts, normal );
		d.alphas = std::vector<double>( verts, 0.0 );
		d.allowedVerts = std::vector<std::int64_t>( 10, -1 );
		solid->sides[t.side].displacement = std::move( d );
		RefreshTags( *solid, t.side );
	}
	return {};
}

EditResult DestroyDisplacement( scene::DocumentEdit &edit, const std::vector<FaceRef> &faces )
{
	std::vector<Target> targets;
	if ( EditResult r = Resolve( edit, faces, true, targets ); !r )
	{
		return r;
	}
	for ( const Target &t : targets )
	{
		edit.MutableSolid( t.face.solid )->sides[t.side].displacement.reset();
	}
	return {};
}

EditResult SetDisplacementPower(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, int power )
{
	if ( power < 2 || power > 4 )
	{
		return Reject( "displacement power is 2, 3 or 4" );
	}
	std::vector<Target> targets;
	if ( EditResult r = Resolve( edit, faces, true, targets ); !r )
	{
		return r;
	}
	bool changed = false;
	for ( const Target &t : targets )
	{
		scene::Solid *solid = edit.MutableSolid( t.face.solid );
		scene::Displacement &d = *solid->sides[t.side].displacement;
		if ( d.power == power )
		{
			continue;
		}
		const int n0 = d.VertsPerRow();
		const std::size_t count0 = static_cast<std::size_t>( n0 * n0 );
		std::vector<Vec3d> vectors( count0 );
		for ( std::size_t i = 0; i < count0; ++i )
		{
			vectors[i] = DisplacementVector( d, i );
		}
		const std::vector<Vec3d> offsets = d.offsets ? *d.offsets : std::vector<Vec3d>( count0 );
		const std::vector<Vec3d> offsetNormals =
		    d.offsetNormals ? *d.offsetNormals
		                    : std::vector<Vec3d>( count0, solid->sides[t.side].Plane().normal );
		const std::vector<double> alphas =
		    d.alphas ? *d.alphas : std::vector<double>( count0, 0.0 );

		scene::Displacement next = d;
		next.power = power;
		const int n1 = next.VertsPerRow();
		std::vector<Vec3d> nv, no, non;
		std::vector<double> na;
		for ( int r = 0; r < n1; ++r )
		{
			for ( int c = 0; c < n1; ++c )
			{
				const double sr = static_cast<double>( r ) * ( n0 - 1 ) / ( n1 - 1 );
				const double sc = static_cast<double>( c ) * ( n0 - 1 ) / ( n1 - 1 );
				nv.push_back( Sample( vectors, n0, sr, sc ) );
				no.push_back( Sample( offsets, n0, sr, sc ) );
				non.push_back( mapgeometry::Normalize( Sample( offsetNormals, n0, sr, sc ) ) );
				na.push_back( Sample( alphas, n0, sr, sc ) );
			}
		}
		next.normals.reset();
		StoreVectors( next, nv, solid->sides[t.side].Plane().normal );
		next.offsets = std::move( no );
		next.offsetNormals = std::move( non );
		next.alphas = std::move( na );
		next.triangleTags.reset();
		next.allowedVerts = std::vector<std::int64_t>( 10, -1 );
		d = std::move( next );
		RefreshTags( *solid, t.side );
		changed = true;
	}
	if ( !changed )
	{
		return NothingToDo( "the displacements already have that power" );
	}
	return {};
}

EditResult Sculpt(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, const SculptBrush &brush )
{
	if ( brush.radius <= 0.0 )
	{
		return Reject( "the brush radius must be positive" );
	}
	if ( brush.direction && mapgeometry::Length( *brush.direction ) < 1.0e-9 )
	{
		return Reject( "the paint direction has no length" );
	}
	std::vector<Target> targets;
	if ( EditResult r = Resolve( edit, faces, true, targets ); !r )
	{
		return r;
	}
	// Plan every face first: refuse malformed displacements before writing.
	struct Plan
	{
		Target target;
		std::vector<Vec3d> vectors;
		bool touched = false;
	};
	std::vector<Plan> plans;
	for ( const Target &t : targets )
	{
		const scene::Solid &solid = *edit.FindSolid( t.face.solid );
		const std::optional<mapgeometry::DisplacementSurface> surface =
		    scene::BuildDisplacement( solid, t.side );
		if ( !surface )
		{
			return Reject(
			    "face " + std::to_string( t.face.side ) + " has a malformed displacement" );
		}
		const scene::Displacement &d = *solid.sides[t.side].displacement;
		const Vec3d dir = mapgeometry::Normalize(
		    brush.direction ? *brush.direction : solid.sides[t.side].Plane().normal );
		const int n = d.VertsPerRow();
		Plan plan{ t, {}, false };
		plan.vectors.resize( surface->vertices.size() );
		for ( std::size_t i = 0; i < surface->vertices.size(); ++i )
		{
			plan.vectors[i] = DisplacementVector( d, i );
		}
		const std::vector<Vec3d> before = plan.vectors;
		for ( std::size_t i = 0; i < surface->vertices.size(); ++i )
		{
			const double w = Weight( mapgeometry::Length( surface->vertices[i] - brush.center ),
			    brush.radius, brush.falloff );
			if ( w <= 0.0 )
			{
				continue;
			}
			plan.touched = true;
			Vec3d &v = plan.vectors[i];
			switch ( brush.mode )
			{
			case SculptMode::Raise:
				v += dir * ( brush.amount * w );
				break;
			case SculptMode::Lower:
				v -= dir * ( brush.amount * w );
				break;
			case SculptMode::Set:
			{
				const double along = mapgeometry::Dot( v, dir );
				v += dir * ( ( brush.amount - along ) * w );
				break;
			}
			case SculptMode::Smooth:
			{
				const int r = static_cast<int>( i ) / n;
				const int c = static_cast<int>( i ) % n;
				Vec3d sum;
				int count = 0;
				for ( const auto &[dr, dc] : { std::pair{ -1, 0 }, std::pair{ 1, 0 },
				          std::pair{ 0, -1 }, std::pair{ 0, 1 } } )
				{
					if ( r + dr >= 0 && r + dr < n && c + dc >= 0 && c + dc < n )
					{
						sum += before[static_cast<std::size_t>( ( r + dr ) * n + c + dc )];
						++count;
					}
				}
				const double k = std::clamp( brush.amount, 0.0, 1.0 ) * w;
				v = v * ( 1.0 - k ) + ( sum / count ) * k;
				break;
			}
			}
		}
		plans.push_back( std::move( plan ) );
	}
	bool any = false;
	for ( Plan &plan : plans )
	{
		if ( !plan.touched )
		{
			continue;
		}
		scene::Solid *solid = edit.MutableSolid( plan.target.face.solid );
		StoreVectors( *solid->sides[plan.target.side].displacement, plan.vectors,
		    solid->sides[plan.target.side].Plane().normal );
		RefreshTags( *solid, plan.target.side );
		any = true;
	}
	if ( !any )
	{
		return NothingToDo( "the brush reaches no vertex" );
	}
	return {};
}

EditResult PaintAlpha( scene::DocumentEdit &edit, const std::vector<FaceRef> &faces,
    const Vec3d &center, double radius, double value, AlphaMode mode, bool falloff )
{
	if ( radius <= 0.0 )
	{
		return Reject( "the brush radius must be positive" );
	}
	std::vector<Target> targets;
	if ( EditResult r = Resolve( edit, faces, true, targets ); !r )
	{
		return r;
	}
	bool any = false;
	for ( const Target &t : targets )
	{
		const std::optional<mapgeometry::DisplacementSurface> surface =
		    scene::BuildDisplacement( *edit.FindSolid( t.face.solid ), t.side );
		if ( !surface )
		{
			return Reject(
			    "face " + std::to_string( t.face.side ) + " has a malformed displacement" );
		}
		std::vector<double> alphas = edit.FindSolid( t.face.solid )
		                                 ->sides[t.side]
		                                 .displacement->alphas.value_or(
		                                     std::vector<double>( surface->vertices.size(), 0.0 ) );
		bool touched = false;
		for ( std::size_t i = 0; i < surface->vertices.size(); ++i )
		{
			const double w =
			    Weight( mapgeometry::Length( surface->vertices[i] - center ), radius, falloff );
			if ( w <= 0.0 )
			{
				continue;
			}
			double next = alphas[i];
			switch ( mode )
			{
			case AlphaMode::Set:
				next = alphas[i] + ( value - alphas[i] ) * w;
				break;
			case AlphaMode::Raise:
				next = alphas[i] + value * w;
				break;
			case AlphaMode::Lower:
				next = alphas[i] - value * w;
				break;
			}
			next = std::clamp( next, 0.0, 255.0 );
			touched = touched || next != alphas[i];
			alphas[i] = next;
		}
		if ( touched )
		{
			edit.MutableSolid( t.face.solid )->sides[t.side].displacement->alphas =
			    std::move( alphas );
			any = true;
		}
	}
	if ( !any )
	{
		return NothingToDo( "the brush changes no alpha" );
	}
	return {};
}

EditResult SetDisplacementElevation(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, double elevation )
{
	std::vector<Target> targets;
	if ( EditResult r = Resolve( edit, faces, true, targets ); !r )
	{
		return r;
	}
	bool any = false;
	for ( const Target &t : targets )
	{
		if ( edit.FindSolid( t.face.solid )->sides[t.side].displacement->elevation == elevation )
		{
			continue;
		}
		scene::Solid *solid = edit.MutableSolid( t.face.solid );
		solid->sides[t.side].displacement->elevation = elevation;
		RefreshTags( *solid, t.side );
		any = true;
	}
	if ( !any )
	{
		return NothingToDo( "the displacements already have that elevation" );
	}
	return {};
}

EditResult SewDisplacements( scene::DocumentEdit &edit, const std::vector<FaceRef> &faces )
{
	std::vector<Target> targets;
	if ( EditResult r = Resolve( edit, faces, true, targets ); !r )
	{
		return r;
	}
	struct VertexRef
	{
		std::size_t target;
		std::size_t vertex;
	};
	std::vector<mapgeometry::DisplacementSurface> base;
	std::vector<mapgeometry::DisplacementSurface> displaced;
	for ( const Target &t : targets )
	{
		const scene::Solid &solid = *edit.FindSolid( t.face.solid );
		const auto b = scene::BuildDisplacement( solid, t.side, true );
		const auto d = scene::BuildDisplacement( solid, t.side );
		if ( !b || !d )
		{
			return Reject(
			    "face " + std::to_string( t.face.side ) + " has a malformed displacement" );
		}
		base.push_back( *b );
		displaced.push_back( *d );
	}
	// Cluster coincident base vertices by a rounded key.
	std::map<std::tuple<long long, long long, long long>, std::vector<VertexRef>> clusters;
	auto key = []( const Vec3d &p )
	{
		return std::make_tuple( std::llround( p.x / kSewEpsilon ),
		    std::llround( p.y / kSewEpsilon ), std::llround( p.z / kSewEpsilon ) );
	};
	for ( std::size_t t = 0; t < base.size(); ++t )
	{
		for ( std::size_t v = 0; v < base[t].vertices.size(); ++v )
		{
			clusters[key( base[t].vertices[v] )].push_back( { t, v } );
		}
	}
	std::vector<std::vector<Vec3d>> targetPositions;
	for ( const mapgeometry::DisplacementSurface &d : displaced )
	{
		targetPositions.push_back( d.vertices );
	}
	bool any = false;
	for ( const auto &[k, refs] : clusters )
	{
		bool mixed = false;
		for ( const VertexRef &r : refs )
		{
			mixed = mixed || r.target != refs.front().target;
		}
		if ( !mixed )
		{
			continue;
		}
		Vec3d sum;
		for ( const VertexRef &r : refs )
		{
			sum += displaced[r.target].vertices[r.vertex];
		}
		const Vec3d average = sum / static_cast<double>( refs.size() );
		for ( const VertexRef &r : refs )
		{
			if ( !mapgeometry::NearlyEqual( targetPositions[r.target][r.vertex], average, 1.0e-9 ) )
			{
				targetPositions[r.target][r.vertex] = average;
				any = true;
			}
		}
	}
	if ( !any )
	{
		return NothingToDo( "the displacements are already sewn" );
	}
	for ( std::size_t t = 0; t < targets.size(); ++t )
	{
		scene::Solid *solid = edit.MutableSolid( targets[t].face.solid );
		scene::Side &side = solid->sides[targets[t].side];
		scene::Displacement &d = *side.displacement;
		const Vec3d n = side.Plane().normal;
		std::vector<Vec3d> vectors( targetPositions[t].size() );
		for ( std::size_t v = 0; v < vectors.size(); ++v )
		{
			const Vec3d offset = d.offsets ? ( *d.offsets )[v] : Vec3d();
			vectors[v] = targetPositions[t][v] - base[t].vertices[v] - offset - n * d.elevation;
		}
		StoreVectors( d, vectors, n );
		RefreshTags( *solid, targets[t].side );
	}
	return {};
}

} // namespace hammer::app::ops
