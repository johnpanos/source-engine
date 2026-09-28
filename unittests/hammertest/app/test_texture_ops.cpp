//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app texture operations (RFC 0002, R08 domain logic):
//			texture lock is exact under translation, rotation, non-uniform scale
//			and mirroring (every point keeps its texel); legacy justify and fit
//			(single face and treated as one); world and face alignment; rotation
//			about the texture normal; shift normalization; material replacement
//			and apply modes. Negative checks: unknown faces refuse with nothing
//			staged, zero scales, materials without a size, empty find strings.
//
//=============================================================================//

#include "hammer/app/ops/texture_ops.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <cmath>
#include <map>

using namespace hammer;
using namespace hammer::app::ops;
using scene::FaceRef;
using mapgeometry::Vec3d;

namespace
{

class Materials final : public ports::IMaterialInfo
{
public:
	bool Exists( std::string_view m ) const override
	{
		return m_sizes.count( std::string( m ) ) > 0;
	}
	std::optional<ports::MaterialSize> Size( std::string_view m ) const override
	{
		const auto it = m_sizes.find( std::string( m ) );
		if ( it == m_sizes.end() )
			return std::nullopt;
		return it->second;
	}
	std::vector<std::string> Names() const override
	{
		std::vector<std::string> out;
		for ( const auto &e : m_sizes )
			out.push_back( e.first );
		return out;
	}
	std::map<std::string, ports::MaterialSize> m_sizes;
};

bool SameTexels( const scene::FaceTexture &before, const scene::FaceTexture &after,
    const mapgeometry::Affine &xf, const std::vector<Vec3d> &points )
{
	for ( const Vec3d &p : points )
	{
		const Vec3d q = xf.Point( p );
		if ( std::fabs( TexelU( before, p ) - TexelU( after, q ) ) > 1e-6 ||
		     std::fabs( TexelV( before, p ) - TexelV( after, q ) ) > 1e-6 )
			return false;
	}
	return true;
}

} // namespace

int main()
{
	testing::Checks checks;

	// Texture lock is exact for any invertible map.
	{
		scene::FaceTexture t;
		t.material = "BRICK/BRICKWALL001";
		t.u = { Vec3d( 1, 0, 0 ), 7, 0.25 };
		t.v = { Vec3d( 0, -1, 0 ), 3, 0.5 };
		const std::vector<Vec3d> pts = {
		    Vec3d( 0, 0, 0 ), Vec3d( 64, 0, 0 ), Vec3d( 13, 77, -5 ), Vec3d( -40, 8, 16 ) };
		const mapgeometry::Affine cases[] = {
		    mapgeometry::Affine::Translation( Vec3d( 16, -8, 4 ) ),
		    mapgeometry::Affine::About(
		        mapgeometry::Mat3::AxisRotation( 2, 90 ), Vec3d( 32, 32, 0 ) ),
		    mapgeometry::Affine::About( mapgeometry::AngleMatrix( 10, 33, 5 ), Vec3d( 1, 2, 3 ) ),
		    mapgeometry::Affine::About( mapgeometry::Mat3::Scale( Vec3d( 2, 0.5, 1 ) ), Vec3d() ),
		    mapgeometry::Affine::About( mapgeometry::Mat3::Mirror( 0 ), Vec3d( 10, 0, 0 ) ),
		};
		int exact = 0;
		for ( const mapgeometry::Affine &xf : cases )
			exact += SameTexels( t, LockTexture( t, xf ), xf, pts ) ? 1 : 0;
		checks.Equal( exact, 5, "texture lock keeps every texel under 5 maps" );
		const scene::FaceTexture moved = LockTexture( t, cases[0] );
		checks.That( moved.u.axis == t.u.axis && moved.u.scale == t.u.scale,
		    "translation keeps axes and scale" );
		checks.Near( moved.u.shift, 7 - 16 / 0.25, 1e-9,
		    "translation shifts by -d/scale (legacy OffsetTexture)" );
		const scene::FaceTexture singular = LockTexture( t,
		    mapgeometry::Affine::About( mapgeometry::Mat3::Scale( Vec3d( 0, 1, 1 ) ), Vec3d() ) );
		checks.That( singular == t, "a singular map leaves the texture (negative)" );
	}

	// Alignment and rotation.
	{
		scene::FaceTexture t;
		t.u.scale = 0.25;
		t.v.scale = 0.25;
		t.u.shift = 5;
		const Vec3d slope = mapgeometry::Normalize( Vec3d( 0, 1, 1 ) );
		const scene::FaceTexture world = AlignedTexture( t, slope, TextureAlignment::World );
		checks.That(
		    world.u.shift == 0 && world.u.scale == 0.25, "alignment resets shifts, keeps scales" );
		const scene::FaceTexture face = AlignedTexture( t, slope, TextureAlignment::Face );
		checks.Near(
		    mapgeometry::Dot( face.u.axis, slope ), 0, 1e-12, "face-aligned u lies in the face" );
		checks.Near(
		    mapgeometry::Dot( face.v.axis, slope ), 0, 1e-12, "face-aligned v lies in the face" );
		checks.Near( mapgeometry::Length( face.v.axis ), 1, 1e-12, "face-aligned axes are unit" );
		const scene::FaceTexture floor =
		    AlignedTexture( t, Vec3d( 0, 0, 1 ), TextureAlignment::World );
		const scene::FaceTexture r = RotatedTexture( floor, 90 );
		checks.That( mapgeometry::NearlyEqual( r.u.axis, Vec3d( 0, -1, 0 ), 1e-12 ) ||
		                 mapgeometry::NearlyEqual( r.u.axis, Vec3d( 0, 1, 0 ), 1e-12 ),
		    "rotation turns the u axis in the floor plane" );
		checks.Near( r.rotation, 90, 1e-12, "rotation field accumulates" );
		checks.That(
		    mapgeometry::NearlyEqual( RotatedTexture( r, -90 ).u.axis, floor.u.axis, 1e-12 ),
		    "rotating back restores the axes" );
		scene::FaceTexture wrap = floor;
		wrap.u.shift = 300;
		wrap.v.shift = -0.0;
		const scene::FaceTexture n = NormalizedShifts( wrap, { 256, 128 } );
		checks.That(
		    n.u.shift == 44 && !std::signbit( n.v.shift ), "shifts wrap by the texture size" );
	}

	// Operations on a document.
	scene::MapDocument doc;
	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::ObjectId box;
	{
		scene::DocumentEdit edit( doc );
		box = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 128, 64, 64 ) }, tex ) );
		scene::CommitEdit( doc, edit );
	}
	const scene::Solid &solid = *doc.FindSolid( box );
	const FaceRef top{ box, solid.sides[0].vmfId };  // +Z
	const FaceRef west{ box, solid.sides[2].vmfId }; // -X
	FaceRef bogus{ box, 999999 };

	Materials mats;
	mats.m_sizes["DEV/DEV_MEASUREGENERIC01B"] = { 128, 128 };
	mats.m_sizes["BRICK/BRICKWALL001"] = { 256, 128 };

	{
		scene::DocumentEdit edit( doc );
		checks.That( ApplyMaterial( edit, { top, west }, "BRICK/BRICKWALL001" ).HasValue(),
		    "apply material" );
		checks.That(
		    FindFace( edit, top )->texture.material == "BRICK/BRICKWALL001", "material set" );
		checks.That(
		    FindFace( doc, top )->texture.material == tex.material, "base untouched until commit" );
		checks.That( ApplyMaterialToObjects( edit, { box }, "TOOLS/TOOLSNODRAW" ).HasValue(),
		    "apply to objects" );
		int all = 0;
		for ( const scene::Side &s : edit.FindSolid( box )->sides )
			all += s.texture.material == "TOOLS/TOOLSNODRAW";
		checks.Equal( all, 6, "every face of the solid" );
	}
	{
		scene::DocumentEdit edit( doc );
		const auto bad = ApplyMaterial( edit, { top, bogus }, "BRICK/BRICKWALL001" );
		checks.That( !bad && edit.Finish().Empty(),
		    "an unknown face refuses with nothing staged (negative)" );
		checks.That( !ApplyMaterial( edit, {}, "X" ), "no faces is nothing to do (negative)" );
		TextureValues zero;
		zero.scaleU = 0.0;
		checks.That( !SetTextureValues( edit, { top }, zero ), "zero scale refused (negative)" );
		TextureValues lm;
		lm.lightmapScale = 0;
		checks.That(
		    !SetTextureValues( edit, { top }, lm ), "zero lightmap scale refused (negative)" );
	}

	// Values, shift and justify.
	{
		scene::DocumentEdit edit( doc );
		TextureValues v;
		v.scaleU = 0.5;
		v.rotation = 90;
		v.lightmapScale = 32;
		checks.That( SetTextureValues( edit, { top }, v ).HasValue(), "set values" );
		const scene::FaceTexture &t = FindFace( edit, top )->texture;
		checks.That(
		    t.u.scale == 0.5 && t.rotation == 90 && t.lightmapScale == 32, "values applied" );
		checks.That( ShiftTexture( edit, { top }, 4, -2 ).HasValue(), "nudge" );
		checks.That( FindFace( edit, top )->texture.v.shift == -2, "nudge adds" );
	}
	{
		// The top face spans x 0..128, y 0..64. World axes u=(1 0 0) v=(0 -1 0), scale 0.25:
		// u in [0, 512], v in [-256, 0].
		scene::DocumentEdit edit( doc );
		checks.That(
		    JustifyTexture( edit, { top }, Justification::Left, mats ).HasValue(), "justify left" );
		checks.Near( FindFace( edit, top )->texture.u.shift, 0, 1e-9, "left edge at texel 0" );
		checks.That(
		    JustifyTexture( edit, { top }, Justification::Top, mats ).HasValue(), "justify top" );
		checks.Near(
		    FindFace( edit, top )->texture.v.shift, 0, 1e-9, "top: -minV = 256 wraps to 0 on 128" );
		checks.That( JustifyTexture( edit, { top }, Justification::Fit, mats ).HasValue(), "fit" );
		const scene::FaceTexture &f = FindFace( edit, top )->texture;
		checks.Near( f.u.scale, 1.0, 1e-9, "fit: 128 units over 128 texels" );
		checks.Near( f.v.scale, 0.5, 1e-9, "fit: 64 units over 128 texels" );
		const std::vector<Vec3d> corners = { Vec3d( 0, 0, 64 ), Vec3d( 128, 64, 64 ) };
		double lo = 1e9, hi = -1e9;
		for ( const Vec3d &c : corners )
		{
			lo = std::min( lo, TexelU( f, c ) );
			hi = std::max( hi, TexelU( f, c ) );
		}
		checks.That( std::fabs( lo ) < 1e-6 && std::fabs( hi - 128 ) < 1e-6,
		    "fit spans exactly one texture" );
		checks.That(
		    JustifyTexture( edit, { top }, Justification::Fit, mats, false, 2, 1 ).HasValue(),
		    "fit x2" );
		checks.Near( FindFace( edit, top )->texture.u.scale, 0.5, 1e-9, "fit twice across" );
		checks.That( !JustifyTexture( edit, { top }, Justification::Fit, mats, false, 0, 1 ),
		    "fit count 0 refused (negative)" );
		Materials none;
		checks.That( !JustifyTexture( edit, { top }, Justification::Left, none ),
		    "a material without a size refuses (negative)" );
	}
	{
		// Treat as one: two faces justified over their union.
		scene::DocumentEdit edit( doc );
		scene::ObjectId second;
		second =
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 128, 0, 0 ), Vec3d( 256, 64, 64 ) }, tex ) );
		const FaceRef top2{ second, edit.FindSolid( second )->sides[0].vmfId };
		checks.That(
		    JustifyTexture( edit, { top, top2 }, Justification::Right, mats, true ).HasValue(),
		    "justify as one" );
		checks.Near( FindFace( edit, top )->texture.u.shift,
		    FindFace( edit, top2 )->texture.u.shift, 1e-9,
		    "faces justified as one share the shift" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    AlignTexture( edit, { west }, TextureAlignment::Face ).HasValue(), "align to face" );
		const scene::FaceTexture &w = FindFace( edit, west )->texture;
		checks.Near(
		    mapgeometry::Dot( w.u.axis, Vec3d( -1, 0, 0 ) ), 0, 1e-12, "wall axes in the wall" );
	}
	{
		scene::DocumentEdit edit( doc );
		int count = 0;
		checks.That( ReplaceMaterial(
		                 edit, {}, "dev/dev_measuregeneric01b", "BRICK/BRICKWALL001", false, count )
		                     .HasValue() &&
		                 count == 6,
		    "replace whole names, case-insensitively" );
		checks.That(
		    ReplaceMaterial( edit, { box }, "BRICKWALL", "CONCRETEWALL", true, count ).HasValue() &&
		        FindFace( edit, top )->texture.material == "BRICK/CONCRETEWALL001",
		    "replace substrings" );
		checks.That( !ReplaceMaterial( edit, {}, "NOPE", "X", false, count ) && count == 0,
		    "no match is nothing to do (negative)" );
		checks.That(
		    !ReplaceMaterial( edit, {}, "", "X", false, count ), "empty find refused (negative)" );
	}
	{
		scene::DocumentEdit edit( doc );
		scene::FaceTexture source = tex;
		source.material = "METAL/METALWALL001";
		source.u.shift = 12;
		source.u.axis = Vec3d( 0, 0, 1 );
		checks.That(
		    ApplyTextureFrom( edit, source, { top }, ApplyTextureMode::MaterialOnly ).HasValue(),
		    "lift: material only" );
		checks.That( FindFace( edit, top )->texture.u.shift == 0 &&
		                 FindFace( edit, top )->texture.material == source.material,
		    "material only keeps alignment" );
		checks.That(
		    ApplyTextureFrom( edit, source, { top }, ApplyTextureMode::MaterialValues ).HasValue(),
		    "lift: values" );
		checks.That( FindFace( edit, top )->texture.u.shift == 12 &&
		                 FindFace( edit, top )->texture.u.axis == Vec3d( 1, 0, 0 ),
		    "values keep the target's axes" );
		checks.That(
		    ApplyTextureFrom( edit, source, { top }, ApplyTextureMode::Projected ).HasValue() &&
		        FindFace( edit, top )->texture == source,
		    "projected copies everything" );
		scene::FaceTexture empty;
		checks.That( !ApplyTextureFrom( edit, empty, { top }, ApplyTextureMode::Projected ),
		    "a source without a material refuses (negative)" );
	}

	// Smoothing groups.
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    SetSmoothingGroup( edit, { top, west }, 3, true ).HasValue(), "set smoothing group 3" );
		checks.That( FindFace( edit, top )->texture.smoothingGroups == 4u, "bit 3 is 4" );
		checks.That( SetSmoothingGroup( edit, { top }, 32, true ).HasValue() &&
		                 FindFace( edit, top )->texture.smoothingGroups == ( 4u | 0x80000000u ),
		    "group 32 is the top bit" );
		checks.That( SetSmoothingGroup( edit, { top }, 3, false ).HasValue() &&
		                 FindFace( edit, top )->texture.smoothingGroups == 0x80000000u,
		    "clear a group" );
		checks.That( SetSmoothingGroup( edit, { west }, 3, true ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "unchanged is nothing" );
		checks.That( !SetSmoothingGroup( edit, { top }, 0, true ) &&
		                 !SetSmoothingGroup( edit, { top }, 33, true ),
		    "groups outside 1..32 refused (negative)" );
	}

	return checks.Report();
}
