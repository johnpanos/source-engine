//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/adapters/render/scene_geometry.h.
//
//=============================================================================//

#include "scene_geometry.h"

#include "mapgeometry/vec3.h"
#include "render/math/matrix.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>

namespace hammer::render_adapter
{

namespace
{

using ::render::pass::lines::LineList;
using ::render::pass::lines::LinesView;
using ::render::pass::lines::Rgba8;
using ::render::pass::lines::Space;
using ::render::pass::lines::Style;
using mapgeometry::Vec3d;

struct Color
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
};

constexpr Color kEdge = { 0.50f, 0.52f, 0.58f };
constexpr Color kSelectedFill = { 1.00f, 0.62f, 0.28f };
constexpr Color kSelectedEdge = { 1.00f, 0.58f, 0.15f };
constexpr Color kSelectedFace = { 1.00f, 0.35f, 0.35f };
constexpr Color kDisplacementEdge = { 0.35f, 0.55f, 0.40f };
constexpr Color kEntityDefault = { 0.85f, 0.35f, 0.85f };

Color SolidFill( int index )
{
	const std::uint32_t h = static_cast<std::uint32_t>( index ) * 2654435761u;
	return { 0.45f + 0.5f * ( ( h & 0xFF ) / 255.0f ),
	    0.45f + 0.5f * ( ( ( h >> 8 ) & 0xFF ) / 255.0f ),
	    0.45f + 0.5f * ( ( ( h >> 16 ) & 0xFF ) / 255.0f ) };
}

Color FromRgb( const scene::Rgb &c )
{
	return { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f };
}

std::uint8_t Byte( float v )
{
	return static_cast<std::uint8_t>( std::lround( std::clamp( v, 0.0f, 1.0f ) * 255.0f ) );
}

Rgba8 ToRgba( const Color &c )
{
	return { Byte( c.r ), Byte( c.g ), Byte( c.b ), 255 };
}

// The editor's fixed two-light shading (fullbright preview): an ambient term
// and two directional lights, as a function of the outward normal.
Color Shade( const Color &base, const Vec3d &normal )
{
	static const Vec3d l1 = mapgeometry::Normalize( Vec3d( 0.4, 0.6, 0.8 ) );
	static const Vec3d l2 = mapgeometry::Normalize( Vec3d( -0.5, -0.3, 0.4 ) );
	const double length = mapgeometry::Length( normal );
	const Vec3d n = length > 1.0e-12 ? normal / length : Vec3d( 0, 0, 1 );
	const double d = std::clamp( 0.35 + 0.55 * std::max( mapgeometry::Dot( n, l1 ), 0.0 ) +
	                                 0.25 * std::max( mapgeometry::Dot( n, l2 ), 0.0 ),
	    0.0, 1.0 );
	return { float( base.r * d ), float( base.g * d ), float( base.b * d ) };
}

::render::math::float3 F3( const Vec3d &p )
{
	return { float( p.x ), float( p.y ), float( p.z ) };
}

LineVertex Vertex( const Vec3d &p, Rgba8 color )
{
	LineVertex v;
	v.position[0] = float( p.x );
	v.position[1] = float( p.y );
	v.position[2] = float( p.z );
	v.color = color.Packed();
	return v;
}

UnlitVertex FaceVertex( const Vec3d &p, Rgba8 color, float u = 0.0f, float v = 0.0f )
{
	UnlitVertex vertex;
	vertex.position[0] = float( p.x );
	vertex.position[1] = float( p.y );
	vertex.position[2] = float( p.z );
	vertex.uv[0] = u;
	vertex.uv[1] = v;
	vertex.color[0] = color.r;
	vertex.color[1] = color.g;
	vertex.color[2] = color.b;
	vertex.color[3] = color.a;
	return vertex;
}

// A point's texture coordinate along one axis, in texture widths.
float TexCoord( const Vec3d &p, const scene::TextureAxis &axis, std::uint32_t size )
{
	const double scale = std::abs( axis.scale ) > 1.0e-9 ? axis.scale : 1.0;
	return float( ( mapgeometry::Dot( p, axis.axis ) / scale + axis.shift ) / double( size ) );
}

void PushEdge( std::vector<LineVertex> &out, const Vec3d &a, const Vec3d &b, Rgba8 color )
{
	out.push_back( Vertex( a, color ) );
	out.push_back( Vertex( b, color ) );
}

void PushBox( std::vector<UnlitVertex> &faces, std::vector<LineVertex> &edges, const Vec3d &lo,
    const Vec3d &hi, const Color &fill, Rgba8 edge )
{
	const std::array<Vec3d, 8> p = { Vec3d( lo.x, lo.y, lo.z ), Vec3d( hi.x, lo.y, lo.z ),
	    Vec3d( hi.x, hi.y, lo.z ), Vec3d( lo.x, hi.y, lo.z ), Vec3d( lo.x, lo.y, hi.z ),
	    Vec3d( hi.x, lo.y, hi.z ), Vec3d( hi.x, hi.y, hi.z ), Vec3d( lo.x, hi.y, hi.z ) };
	struct Quad
	{
		int a, b, c, d;
		Vec3d n;
	};
	const Quad quads[6] = { { 0, 3, 2, 1, Vec3d( 0, 0, -1 ) }, { 4, 5, 6, 7, Vec3d( 0, 0, 1 ) },
	    { 0, 1, 5, 4, Vec3d( 0, -1, 0 ) }, { 2, 3, 7, 6, Vec3d( 0, 1, 0 ) },
	    { 1, 2, 6, 5, Vec3d( 1, 0, 0 ) }, { 3, 0, 4, 7, Vec3d( -1, 0, 0 ) } };
	for ( const Quad &q : quads )
	{
		const Rgba8 shaded = ToRgba( Shade( fill, q.n ) );
		for ( int i : { q.a, q.b, q.c, q.a, q.c, q.d } )
		{
			faces.push_back( FaceVertex( p[i], shaded ) );
		}
	}
	static const int kEdges[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 },
	    { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
	for ( const auto &e : kEdges )
	{
		PushEdge( edges, p[e[0]], p[e[1]], edge );
	}
}

Color RoleColor( tools::OverlayRole role )
{
	switch ( role )
	{
	case tools::OverlayRole::Selection:
		return { 1.00f, 0.58f, 0.15f };
	case tools::OverlayRole::Pending:
		return { 1.00f, 0.88f, 0.30f };
	case tools::OverlayRole::Handle:
		return { 0.95f, 0.95f, 0.95f };
	case tools::OverlayRole::HandleHot:
		return { 1.00f, 1.00f, 0.20f };
	case tools::OverlayRole::Hover:
		return { 0.55f, 0.80f, 1.00f };
	case tools::OverlayRole::Clip:
		return { 0.35f, 1.00f, 0.45f };
	case tools::OverlayRole::Error:
	default:
		return { 1.00f, 0.25f, 0.25f };
	}
}

} // namespace

SceneGeometry BuildSceneGeometry(
    const viewport::RenderSnapshot &snapshot, const TextureSizes &sizes )
{
	SceneGeometry geometry;
	std::map<std::string, std::vector<UnlitVertex>> batches; // "" sorts first
	std::vector<UnlitVertex> &untextured = batches[std::string()];
	std::map<std::string, std::optional<TextureSize>> known;
	auto sizeOf = [&]( const std::string &material ) -> std::optional<TextureSize>
	{
		if ( !sizes || material.empty() )
			return std::nullopt;
		auto found = known.find( material );
		if ( found == known.end() )
		{
			std::optional<TextureSize> size = sizes( material );
			if ( size && ( size->width == 0 || size->height == 0 ) )
				size.reset();
			found = known.emplace( material, size ).first;
		}
		return found->second;
	};
	int solidIndex = 0;
	for ( const viewport::SolidDraw &solid : snapshot.solids )
	{
		const Color fill = solid.selected ? kSelectedFill : SolidFill( solidIndex );
		const Color edge = solid.selected          ? kSelectedEdge
		                   : solid.owner.IsValid() ? FromRgb( solid.color )
		                                           : kEdge;
		++solidIndex;
		for ( const viewport::FaceDraw &face : solid.faces )
		{
			const std::optional<TextureSize> size = sizeOf( face.material );
			std::vector<UnlitVertex> &out = size ? batches[face.material] : untextured;
			// Over a texture the color is the shading and the selection tint only.
			const Color white{};
			const Color tint = face.selected    ? kSelectedFace
			                   : solid.selected ? kSelectedFill
			                                    : white;
			auto textured = [&]( const Vec3d &p, const Color &color )
			{
				return size
				           ? FaceVertex( p, ToRgba( color ), TexCoord( p, face.uAxis, size->width ),
				                 TexCoord( p, face.vAxis, size->height ) )
				           : FaceVertex( p, ToRgba( color ) );
			};
			if ( face.displacement )
			{
				// Terrain: the displaced grid, blended grass -> dirt by vertex alpha.
				const mapgeometry::DisplacementSurface &disp = *face.displacement;
				const int n = static_cast<int>( disp.vertices.size() );
				const Rgba8 dispEdge = ToRgba( solid.selected ? edge : kDisplacementEdge );
				for ( const std::array<int, 3> &tri : disp.triangles )
				{
					if ( std::any_of( tri.begin(), tri.end(),
					         [n]( int i )
					         {
						         return i < 0 || i >= n;
					         } ) )
					{
						continue;
					}
					for ( int i : tri )
					{
						const double alpha = i < static_cast<int>( disp.vertexAlphas.size() )
						                         ? disp.vertexAlphas[i]
						                         : 0.0;
						const float t = static_cast<float>( alpha ) / 255.0f;
						const Color base =
						    size            ? tint
						    : face.selected ? kSelectedFace
						    : solid.selected
						        ? kSelectedFill
						        : Color{ 0.32f + t * 0.20f, 0.48f - t * 0.08f, 0.28f + t * 0.02f };
						const Vec3d normal = i < static_cast<int>( disp.vertexNormals.size() )
						                         ? disp.vertexNormals[i]
						                         : face.normal;
						out.push_back( textured( disp.vertices[i], Shade( base, normal ) ) );
					}
					++geometry.triangles;
					for ( int e = 0; e < 3; ++e )
					{
						PushEdge( geometry.edges, disp.vertices[tri[e]],
						    disp.vertices[tri[( e + 1 ) % 3]], dispEdge );
					}
				}
				continue;
			}
			if ( face.vertices.size() < 3 )
			{
				continue;
			}
			const Color shaded = Shade( size            ? tint
			                            : face.selected ? kSelectedFace
			                                            : fill,
			    face.normal );
			const Vec3d &v0 = face.vertices[0];
			for ( std::size_t i = 1; i + 1 < face.vertices.size(); ++i )
			{
				out.push_back( textured( v0, shaded ) );
				out.push_back( textured( face.vertices[i], shaded ) );
				out.push_back( textured( face.vertices[i + 1], shaded ) );
				++geometry.triangles;
			}
			const Rgba8 faceEdge = ToRgba( face.selected ? kSelectedFace : edge );
			for ( std::size_t i = 0; i < face.vertices.size(); ++i )
			{
				PushEdge( geometry.edges, face.vertices[i],
				    face.vertices[( i + 1 ) % face.vertices.size()], faceEdge );
			}
		}
	}
	for ( const viewport::EntityDraw &entity : snapshot.entities )
	{
		const scene::Rgb &rgb = entity.color;
		const Color color = entity.selected                              ? kSelectedFill
		                    : ( rgb.r == 0 && rgb.g == 0 && rgb.b == 0 ) ? kEntityDefault
		                                                                 : FromRgb( rgb );
		PushBox( untextured, geometry.edges, entity.mins, entity.maxs, color,
		    ToRgba( entity.selected ? kSelectedEdge : color ) );
	}
	for ( auto &[material, vertices] : batches )
	{
		if ( !vertices.empty() )
			geometry.faces.push_back( { material, std::move( vertices ) } );
	}
	return geometry;
}

void AppendOverlay( const tools::OverlayList &overlay, LineList &out )
{
	const Style world{ Space::kWorld, false };
	const Style screen{ Space::kScreen, false };
	for ( const tools::OverlayItem &item : overlay.items )
	{
		const Rgba8 color = ToRgba( RoleColor( item.role ) );
		switch ( item.kind )
		{
		case tools::OverlayKind::WorldLine:
			if ( item.world.size() >= 2 )
			{
				out.Line( world, F3( item.world[0] ), F3( item.world[1] ), color );
			}
			break;
		case tools::OverlayKind::WorldBox:
			if ( item.world.size() >= 2 )
			{
				out.Box( world, F3( item.world[0] ), F3( item.world[1] ), color );
			}
			break;
		case tools::OverlayKind::WorldPolygon:
		{
			std::vector<::render::math::float3> points;
			points.reserve( item.world.size() );
			for ( const Vec3d &p : item.world )
			{
				points.push_back( F3( p ) );
			}
			out.Polygon( world, points, color );
			break;
		}
		case tools::OverlayKind::ScreenRect:
		{
			const ::render::math::float3 corners[4] = {
			    { float( item.a.x ), float( item.a.y ), 0.0f },
			    { float( item.b.x ), float( item.a.y ), 0.0f },
			    { float( item.b.x ), float( item.b.y ), 0.0f },
			    { float( item.a.x ), float( item.b.y ), 0.0f } };
			out.Polygon( screen, corners, color );
			break;
		}
		case tools::OverlayKind::ScreenHandle:
			if ( item.shape == tools::HandleShape::Circle )
			{
				out.Disc( screen, float( item.a.x ), float( item.a.y ), float( item.size ), color );
			}
			else
			{
				out.Quad( screen, float( item.a.x - item.size ), float( item.a.y - item.size ),
				    float( item.a.x + item.size ), float( item.a.y + item.size ), color );
			}
			break;
		case tools::OverlayKind::ScreenLabel:
			break; // no text pass yet; the host's status bar carries tool status
		}
	}
}

void AppendGrid(
    std::span<const viewport::GridLine> grid, double width, double height, LineList &out )
{
	const Style screen{ Space::kScreen, false };
	for ( const viewport::GridLine &line : grid )
	{
		const bool vertical = line.orientation == viewport::GridLineOrientation::Vertical;
		Color c = { 0.13f, 0.13f, 0.15f };
		switch ( line.kind )
		{
		case viewport::GridLineKind::Major:
			c = { 0.22f, 0.22f, 0.24f };
			break;
		case viewport::GridLineKind::Block:
			c = { 0.26f, 0.26f, 0.36f };
			break;
		case viewport::GridLineKind::Axis:
			c = vertical ? Color{ 0.20f, 0.45f, 0.20f } : Color{ 0.45f, 0.20f, 0.20f };
			break;
		case viewport::GridLineKind::Minor:
			break;
		}
		// Pixel centers, so a one-pixel line covers the row or column it names.
		const float at = float( std::floor( line.screen ) + 0.5 );
		const ::render::math::float3 a = vertical ? ::render::math::float3{ at, 0.0f, 0.0f }
		                                          : ::render::math::float3{ 0.0f, at, 0.0f };
		const ::render::math::float3 b = vertical
		                                     ? ::render::math::float3{ at, float( height ), 0.0f }
		                                     : ::render::math::float3{ float( width ), at, 0.0f };
		out.Line( screen, a, b, ToRgba( c ) );
	}
}

LinesView ViewFor( const viewport::Camera2D &camera )
{
	// The camera's projection is affine, so it is read off WorldToScreen at
	// the origin and the three unit vectors rather than restating its axis
	// and sign conventions here, then carried from pixels to clip.
	const viewport::ScreenPoint s0 = camera.WorldToScreen( Vec3d( 0, 0, 0 ) );
	const Vec3d unit[3] = { Vec3d( 1, 0, 0 ), Vec3d( 0, 1, 0 ), Vec3d( 0, 0, 1 ) };
	::render::math::float4x4 toPixels;
	toPixels.rows[0] = { 0, 0, 0, float( s0.x ) };
	toPixels.rows[1] = { 0, 0, 0, float( s0.y ) };
	toPixels.rows[2] = { 0, 0, 0, 0 };
	toPixels.rows[3] = { 0, 0, 0, 1 };
	float *row0 = &toPixels.rows[0].x;
	float *row1 = &toPixels.rows[1].x;
	for ( int i = 0; i < 3; ++i )
	{
		const viewport::ScreenPoint s = camera.WorldToScreen( unit[i] );
		row0[i] = float( s.x - s0.x );
		row1[i] = float( s.y - s0.y );
	}
	LinesView view;
	view.width = static_cast<std::uint32_t>( std::max( camera.Width(), 1 ) );
	view.height = static_cast<std::uint32_t>( std::max( camera.Height(), 1 ) );
	view.worldToClip = ::render::math::Multiply(
	    ::render::math::PixelToClip( float( view.width ), float( view.height ) ), toPixels );
	return view;
}

LinesView ViewFor( const viewport::Camera3D &camera, const std::optional<scene::Box> &bounds )
{
	double zFar = 16384.0;
	if ( bounds )
	{
		const Vec3d lo = bounds->mins;
		const Vec3d hi = bounds->maxs;
		for ( int i = 0; i < 8; ++i )
		{
			const Vec3d corner( i & 1 ? hi.x : lo.x, i & 2 ? hi.y : lo.y, i & 4 ? hi.z : lo.z );
			zFar = std::max( zFar, mapgeometry::Length( corner - camera.Position() ) + 1024.0 );
		}
	}
	LinesView view;
	view.width = static_cast<std::uint32_t>( std::max( camera.Width(), 1 ) );
	view.height = static_cast<std::uint32_t>( std::max( camera.Height(), 1 ) );
	const double aspect = double( view.width ) / double( view.height );
	const ::render::math::float4x4 look = ::render::math::LookBasis(
	    F3( camera.Position() ), F3( camera.Forward() ), F3( camera.Right() ), F3( camera.Up() ) );
	const ::render::math::float4x4 projection =
	    ::render::math::Perspective( float( camera.Fov() * 3.14159265358979323846 / 180.0 ),
	        float( aspect ), 2.0f, float( zFar ) );
	view.worldToClip = ::render::math::Multiply( projection, look );
	// Edges drawn on their own faces win the depth test: a clip-space offset
	// of 0.2% of the near distance moves a line by about 0.2% of its own
	// distance, 0.02 units at 10 and 2 at 1,000.
	view.depthBias = 0.002f * 2.0f;
	return view;
}

} // namespace hammer::render_adapter
