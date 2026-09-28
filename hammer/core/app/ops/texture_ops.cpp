//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/texture_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/texture_ops.h"

#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/texture_axes.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace hammer::app::ops
{

using mapgeometry::Vec3d;
using scene::FaceRef;
using scene::FaceTexture;

namespace
{

constexpr double kPi = 3.14159265358979323846;

Vec3d RotateAbout( const Vec3d &v, const Vec3d &axisRaw, double degrees )
{
	const Vec3d k = mapgeometry::Normalize( axisRaw );
	const double r = degrees * kPi / 180.0;
	const double c = std::cos( r );
	const double s = std::sin( r );
	// Rodrigues' rotation formula.
	return v * c + mapgeometry::Cross( k, v ) * s + k * ( mapgeometry::Dot( k, v ) * ( 1.0 - c ) );
}

double RoundNear( double v, double epsilon )
{
	const double r = std::round( v );
	return std::fabs( v - r ) < epsilon ? r : v;
}

std::string Lower( std::string s )
{
	for ( char &c : s )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	return s;
}

scene::Side *MutableFace( scene::DocumentEdit &edit, const FaceRef &face )
{
	scene::Solid *solid = edit.MutableSolid( face.solid );
	return solid ? solid->FindSide( face.side ) : nullptr;
}

// Validates every reference before any write, so a bad reference refuses the
// whole operation with nothing staged.
EditResult CheckFaces( const scene::DocumentEdit &edit, const std::vector<FaceRef> &faces )
{
	if ( faces.empty() )
	{
		return NothingToDo( "no faces selected" );
	}
	for ( const FaceRef &f : faces )
	{
		if ( !FindFace( edit, f ) )
		{
			return Reject( "unknown face " + std::to_string( f.side ) );
		}
	}
	return {};
}

// The face polygon (texture math needs its vertices).
std::vector<Vec3d> FaceVertices( const scene::Solid &solid, std::uint32_t sideId )
{
	const mapgeometry::BrushSolid geometry = scene::BuildGeometry( solid );
	for ( const mapgeometry::BrushFace &f : geometry.faces )
	{
		if ( solid.sides[static_cast<std::size_t>( f.sourcePlane )].vmfId == sideId )
		{
			return f.vertices;
		}
	}
	return {};
}

struct Extent
{
	double minU = 0.0;
	double minV = 0.0;
	double maxU = 0.0;
	double maxV = 0.0;
	bool valid = false;

	void Add( double u, double v )
	{
		if ( !valid )
		{
			minU = maxU = u;
			minV = maxV = v;
			valid = true;
			return;
		}
		minU = std::min( minU, u );
		maxU = std::max( maxU, u );
		minV = std::min( minV, v );
		maxV = std::max( maxV, v );
	}
};

// Texture-space extent of 'vertices' without the shift (legacy
// GetTextureExtents).
Extent TextureExtent( const FaceTexture &t, const std::vector<Vec3d> &vertices )
{
	Extent e;
	for ( const Vec3d &p : vertices )
	{
		e.Add( mapgeometry::Dot( p, t.u.axis ) / t.u.scale,
		    mapgeometry::Dot( p, t.v.axis ) / t.v.scale );
	}
	return e;
}

} // namespace

double TexelU( const FaceTexture &t, const Vec3d &p )
{
	return mapgeometry::Dot( p, t.u.axis ) / t.u.scale + t.u.shift;
}

double TexelV( const FaceTexture &t, const Vec3d &p )
{
	return mapgeometry::Dot( p, t.v.axis ) / t.v.scale + t.v.shift;
}

FaceTexture LockTexture( const FaceTexture &texture, const mapgeometry::Affine &xf )
{
	// u(p) = dot( p, w ) + shift with w = axis / scale. For p' = M p + t we need
	// w' = M^-T w and shift' = shift - dot( t, w' ).
	const std::optional<mapgeometry::Affine> inverse = mapgeometry::Inverse( xf );
	if ( !inverse )
	{
		return texture;
	}
	const mapgeometry::Mat3 invT = mapgeometry::Transpose( inverse->linear );
	FaceTexture out = texture;
	auto lockAxis = [&]( const scene::TextureAxis &axis, scene::TextureAxis &result )
	{
		const Vec3d w = mapgeometry::Apply( invT, axis.axis / axis.scale );
		const double len = mapgeometry::Length( w );
		if ( len < 1.0e-12 )
		{
			return;
		}
		// Keep the scale's sign convention: axis direction absorbs orientation.
		const double scaleMagnitude = 1.0 / len;
		const double sign = axis.scale < 0.0 ? -1.0 : 1.0;
		result.scale = scaleMagnitude * sign;
		result.axis = w * ( result.scale );
		result.shift = axis.shift - mapgeometry::Dot( xf.translation, w );
	};
	lockAxis( texture.u, out.u );
	lockAxis( texture.v, out.v );
	return out;
}

FaceTexture AlignedTexture(
    const FaceTexture &texture, const Vec3d &normal, TextureAlignment alignment )
{
	FaceTexture out = texture;
	const mapgeometry::TextureAxes world = mapgeometry::WorldAlignedTextureAxes( normal );
	out.u.axis = world.u;
	out.v.axis = world.v;
	if ( alignment == TextureAlignment::Face )
	{
		const Vec3d n = mapgeometry::Normalize( normal );
		const Vec3d u = mapgeometry::Normalize( mapgeometry::Cross( n, world.v ) );
		if ( mapgeometry::Length( u ) > 0.5 )
		{
			out.u.axis = u;
			out.v.axis = mapgeometry::Normalize( mapgeometry::Cross( u, n ) );
		}
	}
	out.u.shift = 0.0;
	out.v.shift = 0.0;
	out.rotation = 0.0;
	return out;
}

FaceTexture RotatedTexture( const FaceTexture &texture, double degrees )
{
	FaceTexture out = texture;
	const Vec3d normal = mapgeometry::Cross( texture.v.axis, texture.u.axis );
	if ( mapgeometry::Length( normal ) < 1.0e-12 )
	{
		return out;
	}
	out.u.axis = RotateAbout( texture.u.axis, normal, degrees );
	out.v.axis = RotateAbout( texture.v.axis, normal, degrees );
	out.rotation = texture.rotation + degrees;
	return out;
}

FaceTexture NormalizedShifts( const FaceTexture &texture, const ports::MaterialSize &size )
{
	FaceTexture out = texture;
	for ( scene::TextureAxis *axis : { &out.u, &out.v } )
	{
		axis->axis = Vec3d( RoundNear( axis->axis.x, 0.001 ), RoundNear( axis->axis.y, 0.001 ),
		    RoundNear( axis->axis.z, 0.001 ) );
		axis->shift = RoundNear( axis->shift, 0.001 );
	}
	if ( size.width > 0 )
	{
		out.u.shift = std::fmod( out.u.shift, static_cast<double>( size.width ) );
	}
	if ( size.height > 0 )
	{
		out.v.shift = std::fmod( out.v.shift, static_cast<double>( size.height ) );
	}
	if ( out.u.shift == 0.0 )
	{
		out.u.shift = 0.0; // fold negative zero
	}
	if ( out.v.shift == 0.0 )
	{
		out.v.shift = 0.0;
	}
	return out;
}

const scene::Side *FindFace( const scene::DocumentReader &doc, const FaceRef &face )
{
	const scene::Solid *solid = doc.FindSolid( face.solid );
	return solid ? solid->FindSide( face.side ) : nullptr;
}

EditResult ApplyMaterial(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, const std::string &material )
{
	if ( material.empty() )
	{
		return Reject( "no material" );
	}
	if ( EditResult ok = CheckFaces( edit, faces ); !ok )
	{
		return ok;
	}
	for ( const FaceRef &f : faces )
	{
		MutableFace( edit, f )->texture.material = material;
	}
	return {};
}

EditResult ApplyMaterialToObjects( scene::DocumentEdit &edit,
    const std::vector<scene::ObjectId> &ids, const std::string &material )
{
	if ( material.empty() )
	{
		return Reject( "no material" );
	}
	bool any = false;
	for ( scene::ObjectId id : scene::ExpandToLeaves( edit, ids ) )
	{
		if ( scene::Solid *s = edit.MutableSolid( id ) )
		{
			for ( scene::Side &side : s->sides )
			{
				side.texture.material = material;
			}
			any = true;
		}
	}
	if ( !any )
	{
		return NothingToDo( "no solids selected" );
	}
	return {};
}

EditResult SetTextureValues(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, const TextureValues &values )
{
	if ( ( values.scaleU && *values.scaleU == 0.0 ) || ( values.scaleV && *values.scaleV == 0.0 ) )
	{
		return Reject( "texture scale cannot be zero" );
	}
	if ( values.lightmapScale && *values.lightmapScale <= 0.0 )
	{
		return Reject( "lightmap scale must be positive" );
	}
	if ( EditResult ok = CheckFaces( edit, faces ); !ok )
	{
		return ok;
	}
	for ( const FaceRef &f : faces )
	{
		FaceTexture &t = MutableFace( edit, f )->texture;
		if ( values.rotation )
		{
			t = RotatedTexture( t, *values.rotation - t.rotation );
			t.rotation = *values.rotation;
		}
		if ( values.shiftU )
		{
			t.u.shift = *values.shiftU;
		}
		if ( values.shiftV )
		{
			t.v.shift = *values.shiftV;
		}
		if ( values.scaleU )
		{
			t.u.scale = *values.scaleU;
		}
		if ( values.scaleV )
		{
			t.v.scale = *values.scaleV;
		}
		if ( values.lightmapScale )
		{
			t.lightmapScale = *values.lightmapScale;
		}
	}
	return {};
}

EditResult ShiftTexture(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, double deltaU, double deltaV )
{
	if ( EditResult ok = CheckFaces( edit, faces ); !ok )
	{
		return ok;
	}
	for ( const FaceRef &f : faces )
	{
		FaceTexture &t = MutableFace( edit, f )->texture;
		t.u.shift += deltaU;
		t.v.shift += deltaV;
	}
	return {};
}

EditResult SetSmoothingGroup(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, int group, bool on )
{
	if ( group < 1 || group > 32 )
	{
		return Reject( "smoothing groups are numbered 1 to 32" );
	}
	if ( EditResult ok = CheckFaces( edit, faces ); !ok )
	{
		return ok;
	}
	const std::uint32_t bit = 1u << ( group - 1 );
	bool changed = false;
	for ( const FaceRef &f : faces )
	{
		const std::uint32_t current = FindFace( edit, f )->texture.smoothingGroups;
		const std::uint32_t next = on ? ( current | bit ) : ( current & ~bit );
		if ( next != current )
		{
			MutableFace( edit, f )->texture.smoothingGroups = next;
			changed = true;
		}
	}
	if ( !changed )
	{
		return NothingToDo( "the faces already have that smoothing group setting" );
	}
	return {};
}

EditResult JustifyTexture( scene::DocumentEdit &edit, const std::vector<FaceRef> &faces,
    Justification justification, const ports::IMaterialInfo &materials, bool treatAsOne, int fitU,
    int fitV )
{
	if ( fitU < 1 || fitV < 1 )
	{
		return Reject( "fit repeat counts must be at least 1" );
	}
	if ( EditResult ok = CheckFaces( edit, faces ); !ok )
	{
		return ok;
	}
	// Gather sizes and vertices first; refuse before any write.
	struct Work
	{
		FaceRef face;
		ports::MaterialSize size;
		std::vector<Vec3d> vertices;
	};
	std::vector<Work> work;
	std::vector<Vec3d> allVertices;
	for ( const FaceRef &f : faces )
	{
		const scene::Side *side = FindFace( edit, f );
		const std::optional<ports::MaterialSize> size = materials.Size( side->texture.material );
		if ( !size || size->width <= 0 || size->height <= 0 )
		{
			return Reject( "material '" + side->texture.material + "' has no known size" );
		}
		Work w{ f, *size, FaceVertices( *edit.FindSolid( f.solid ), f.side ) };
		if ( w.vertices.empty() )
		{
			return Reject( "face " + std::to_string( f.side ) + " has no polygon" );
		}
		allVertices.insert( allVertices.end(), w.vertices.begin(), w.vertices.end() );
		work.push_back( std::move( w ) );
	}

	for ( const Work &w : work )
	{
		FaceTexture &t = MutableFace( edit, w.face )->texture;
		const std::vector<Vec3d> &extentOf = treatAsOne ? allVertices : w.vertices;
		const double width = w.size.width;
		const double height = w.size.height;
		if ( justification == Justification::Fit )
		{
			FaceTexture unit = t;
			unit.u.scale = 1.0;
			unit.v.scale = 1.0;
			const Extent e = TextureExtent( unit, extentOf );
			const double spanU = e.maxU - e.minU;
			const double spanV = e.maxV - e.minV;
			if ( spanU <= 0.0 || spanV <= 0.0 )
			{
				continue; // a degenerate projection keeps its values
			}
			t.u.scale = spanU / ( width * fitU );
			t.v.scale = spanV / ( height * fitV );
			const Extent fitted = TextureExtent( t, extentOf );
			t.u.shift = -fitted.minU;
			t.v.shift = -fitted.minV;
		}
		else
		{
			const Extent e = TextureExtent( t, extentOf );
			switch ( justification )
			{
			case Justification::Left:
				t.u.shift = -e.minU;
				break;
			case Justification::Right:
				t.u.shift = -e.maxU + width;
				break;
			case Justification::Top:
				t.v.shift = -e.minV;
				break;
			case Justification::Bottom:
				t.v.shift = -e.maxV + height;
				break;
			case Justification::Center:
				t.u.shift = -( e.minU + e.maxU ) / 2.0 + width / 2.0;
				t.v.shift = -( e.minV + e.maxV ) / 2.0 + height / 2.0;
				break;
			case Justification::Fit:
				break;
			}
		}
		t = NormalizedShifts( t, w.size );
	}
	return {};
}

EditResult AlignTexture(
    scene::DocumentEdit &edit, const std::vector<FaceRef> &faces, TextureAlignment alignment )
{
	if ( EditResult ok = CheckFaces( edit, faces ); !ok )
	{
		return ok;
	}
	for ( const FaceRef &f : faces )
	{
		scene::Side *side = MutableFace( edit, f );
		const double rotation = side->texture.rotation;
		side->texture = AlignedTexture( side->texture, side->Plane().normal, alignment );
		if ( rotation != 0.0 )
		{
			side->texture = RotatedTexture( side->texture, rotation );
		}
	}
	return {};
}

EditResult ReplaceMaterial( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const std::string &find, const std::string &replace, bool substring, int &replacedCount )
{
	replacedCount = 0;
	if ( find.empty() || replace.empty() )
	{
		return Reject( "find and replace must both be named" );
	}
	const std::string needle = Lower( find );
	const std::vector<scene::ObjectId> scope =
	    ids.empty() ? edit.SolidIds() : scene::ExpandToLeaves( edit, ids );
	for ( scene::ObjectId id : scope )
	{
		const scene::Solid *current = edit.FindSolid( id );
		if ( !current )
		{
			continue;
		}
		// Decide before touching, so untouched solids are not copied.
		bool matches = false;
		for ( const scene::Side &side : current->sides )
		{
			const std::string mat = Lower( side.texture.material );
			matches =
			    matches || ( substring ? mat.find( needle ) != std::string::npos : mat == needle );
		}
		if ( !matches )
		{
			continue;
		}
		for ( scene::Side &side : edit.MutableSolid( id )->sides )
		{
			const std::string mat = Lower( side.texture.material );
			if ( substring )
			{
				const std::size_t at = mat.find( needle );
				if ( at != std::string::npos )
				{
					side.texture.material.replace( at, find.size(), replace );
					++replacedCount;
				}
			}
			else if ( mat == needle )
			{
				side.texture.material = replace;
				++replacedCount;
			}
		}
	}
	if ( replacedCount == 0 )
	{
		return NothingToDo( "no face uses '" + find + "'" );
	}
	return {};
}

EditResult ApplyTextureFrom( scene::DocumentEdit &edit, const FaceTexture &source,
    const std::vector<FaceRef> &targets, ApplyTextureMode mode )
{
	if ( source.material.empty() )
	{
		return Reject( "the source face has no material" );
	}
	if ( EditResult ok = CheckFaces( edit, targets ); !ok )
	{
		return ok;
	}
	for ( const FaceRef &f : targets )
	{
		FaceTexture &t = MutableFace( edit, f )->texture;
		switch ( mode )
		{
		case ApplyTextureMode::MaterialOnly:
			t.material = source.material;
			break;
		case ApplyTextureMode::MaterialValues:
		{
			const Vec3d uAxis = t.u.axis;
			const Vec3d vAxis = t.v.axis;
			const double rotation = t.rotation;
			t = source;
			t.u.axis = uAxis;
			t.v.axis = vAxis;
			t.rotation = rotation;
			if ( source.rotation != rotation )
			{
				t = RotatedTexture( t, source.rotation - rotation );
			}
			break;
		}
		case ApplyTextureMode::Projected:
			t = source;
			break;
		}
	}
	return {};
}

} // namespace hammer::app::ops
