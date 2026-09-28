//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the GTK Hammer viewport renderer (see renderer.h).
//			Uses libepoxy for GL entry points. No GTK/GDK includes: the caller owns
//			context/current-ness and framebuffer binding. The view transforms are
//			derived from the viewport cameras' own projections (Camera2D::
//			WorldToScreen, Camera3D's basis and field of view), so a pixel drawn
//			here and a pixel the tools pick agree.
//
//=============================================================================//

#include "renderer.h"

#include "mapgeometry/vec3.h"

#include <epoxy/gl.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace hammergtk
{

namespace
{

using hammer::tools::OverlayItem;
using hammer::tools::OverlayKind;
using hammer::tools::OverlayRole;
using hammer::viewport::Camera2D;
using hammer::viewport::Camera3D;
using hammer::viewport::GridLine;
using hammer::viewport::GridLineKind;
using hammer::viewport::GridLineOrientation;
using mapgeometry::Vec3d;

constexpr double kPi = 3.14159265358979323846;

// ---- Minimal column-major mat4 helpers (GL order) --------------------------

struct Mat4
{
	float m[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
};

Mat4 Multiply( const Mat4 &a, const Mat4 &b )
{
	Mat4 r;
	for ( int col = 0; col < 4; ++col )
	{
		for ( int row = 0; row < 4; ++row )
		{
			float sum = 0.0f;
			for ( int k = 0; k < 4; ++k )
			{
				sum += a.m[k * 4 + row] * b.m[col * 4 + k];
			}
			r.m[col * 4 + row] = sum;
		}
	}
	return r;
}

Mat4 Perspective( double fovyRad, double aspect, double zNear, double zFar )
{
	Mat4 r;
	for ( float &v : r.m )
	{
		v = 0.0f;
	}
	const double f = 1.0 / std::tan( fovyRad * 0.5 );
	r.m[0] = static_cast<float>( f / aspect );
	r.m[5] = static_cast<float>( f );
	r.m[10] = static_cast<float>( ( zFar + zNear ) / ( zNear - zFar ) );
	r.m[11] = -1.0f;
	r.m[14] = static_cast<float>( ( 2.0 * zFar * zNear ) / ( zNear - zFar ) );
	return r;
}

// The view matrix of a Camera3D: rows Right, Up and -Forward about the eye.
Mat4 ViewOf( const Camera3D &camera )
{
	const Vec3d r = camera.Right();
	const Vec3d u = camera.Up();
	const Vec3d f = camera.Forward();
	const Vec3d &eye = camera.Position();
	Mat4 m;
	m.m[0] = static_cast<float>( r.x );
	m.m[4] = static_cast<float>( r.y );
	m.m[8] = static_cast<float>( r.z );
	m.m[12] = static_cast<float>( -mapgeometry::Dot( r, eye ) );
	m.m[1] = static_cast<float>( u.x );
	m.m[5] = static_cast<float>( u.y );
	m.m[9] = static_cast<float>( u.z );
	m.m[13] = static_cast<float>( -mapgeometry::Dot( u, eye ) );
	m.m[2] = static_cast<float>( -f.x );
	m.m[6] = static_cast<float>( -f.y );
	m.m[10] = static_cast<float>( -f.z );
	m.m[14] = static_cast<float>( mapgeometry::Dot( f, eye ) );
	m.m[3] = 0.0f;
	m.m[7] = 0.0f;
	m.m[11] = 0.0f;
	m.m[15] = 1.0f;
	return m;
}

// World -> clip for a Camera2D. The camera's projection is affine, so it is
// read off Camera2D::WorldToScreen at the origin and the three unit vectors
// rather than restating its axis and sign conventions here.
Mat4 OrthoOf( const Camera2D &camera )
{
	const double w = camera.Width();
	const double h = camera.Height();
	const hammer::viewport::ScreenPoint s0 = camera.WorldToScreen( Vec3d( 0, 0, 0 ) );
	const Vec3d unit[3] = { Vec3d( 1, 0, 0 ), Vec3d( 0, 1, 0 ), Vec3d( 0, 0, 1 ) };
	Mat4 m;
	for ( float &v : m.m )
	{
		v = 0.0f;
	}
	for ( int i = 0; i < 3; ++i )
	{
		const hammer::viewport::ScreenPoint s = camera.WorldToScreen( unit[i] );
		m.m[i * 4 + 0] = static_cast<float>( 2.0 / w * ( s.x - s0.x ) );
		m.m[i * 4 + 1] = static_cast<float>( -2.0 / h * ( s.y - s0.y ) );
	}
	m.m[12] = static_cast<float>( 2.0 / w * s0.x - 1.0 );
	m.m[13] = static_cast<float>( 1.0 - 2.0 / h * s0.y );
	m.m[15] = 1.0f;
	return m;
}

// Logical pixels (y down) -> clip.
Mat4 ScreenOf( int width, int height )
{
	Mat4 m;
	for ( float &v : m.m )
	{
		v = 0.0f;
	}
	m.m[0] = 2.0f / static_cast<float>( width );
	m.m[5] = -2.0f / static_cast<float>( height );
	m.m[12] = -1.0f;
	m.m[13] = 1.0f;
	m.m[15] = 1.0f;
	return m;
}

// ---- Shaders ---------------------------------------------------------------

const char *kVertexSrc = "#version 330 core\n"
                         "layout(location=0) in vec3 aPos;\n"
                         "layout(location=1) in vec3 aNormal;\n"
                         "layout(location=2) in vec3 aColor;\n"
                         "layout(location=3) in vec2 aTexCoord;\n"
                         "uniform mat4 uMVP;\n"
                         "out vec3 vNormal;\n"
                         "out vec3 vColor;\n"
                         "out vec2 vUV;\n"
                         "void main(){\n"
                         "  vNormal = aNormal;\n"
                         "  vColor = aColor;\n"
                         "  vUV = aTexCoord;\n"
                         "  gl_Position = uMVP * vec4(aPos, 1.0);\n"
                         "}\n";

const char *kFragmentSrc = "#version 330 core\n"
                           "in vec3 vNormal;\n"
                           "in vec3 vColor;\n"
                           "in vec2 vUV;\n"
                           "uniform int uWire;\n"
                           "uniform int uUseTexture;\n"
                           "uniform sampler2D uTex;\n"
                           "out vec4 fragColor;\n"
                           "void main(){\n"
                           "  if (uWire == 1) {\n"
                           "    fragColor = vec4(vColor, 1.0);\n"
                           "    return;\n"
                           "  }\n"
                           "  vec3 base = (uUseTexture == 1) ? texture(uTex, vUV).rgb : vColor;\n"
                           "  vec3 n = normalize(vNormal);\n"
                           "  vec3 l1 = normalize(vec3(0.4, 0.6, 0.8));\n"
                           "  vec3 l2 = normalize(vec3(-0.5, -0.3, 0.4));\n"
                           "  float d = 0.35 + 0.55*max(dot(n,l1),0.0) + 0.25*max(dot(n,l2),0.0);\n"
                           "  fragColor = vec4(base * clamp(d,0.0,1.0), 1.0);\n"
                           "}\n";

bool CompileShader( GLenum type, const char *src, GLuint &out, std::string &error )
{
	const GLuint sh = glCreateShader( type );
	glShaderSource( sh, 1, &src, nullptr );
	glCompileShader( sh );
	GLint ok = 0;
	glGetShaderiv( sh, GL_COMPILE_STATUS, &ok );
	if ( !ok )
	{
		char log[1024] = { 0 };
		glGetShaderInfoLog( sh, sizeof( log ) - 1, nullptr, log );
		error = std::string( "shader compile failed: " ) + log;
		glDeleteShader( sh );
		return false;
	}
	out = sh;
	return true;
}

struct Color
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
};

Color SolidFill( int index )
{
	const std::uint32_t h = static_cast<std::uint32_t>( index ) * 2654435761u;
	return { 0.45f + 0.5f * ( ( h & 0xFF ) / 255.0f ),
	    0.45f + 0.5f * ( ( ( h >> 8 ) & 0xFF ) / 255.0f ),
	    0.45f + 0.5f * ( ( ( h >> 16 ) & 0xFF ) / 255.0f ) };
}

Color FromRgb( const hammer::scene::Rgb &c )
{
	return { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f };
}

constexpr Color kEdge = { 0.50f, 0.52f, 0.58f };
constexpr Color kSelectedFill = { 1.00f, 0.62f, 0.28f };
constexpr Color kSelectedEdge = { 1.00f, 0.58f, 0.15f };
constexpr Color kSelectedFace = { 1.00f, 0.35f, 0.35f };
constexpr Color kDisplacementEdge = { 0.35f, 0.55f, 0.40f };
constexpr Color kEntityDefault = { 0.85f, 0.35f, 0.85f };

Color RoleColor( OverlayRole role )
{
	switch ( role )
	{
	case OverlayRole::Selection:
		return { 1.00f, 0.58f, 0.15f };
	case OverlayRole::Pending:
		return { 1.00f, 0.88f, 0.30f };
	case OverlayRole::Handle:
		return { 0.95f, 0.95f, 0.95f };
	case OverlayRole::HandleHot:
		return { 1.00f, 1.00f, 0.20f };
	case OverlayRole::Hover:
		return { 0.55f, 0.80f, 1.00f };
	case OverlayRole::Clip:
		return { 0.35f, 1.00f, 0.45f };
	case OverlayRole::Error:
	default:
		return { 1.00f, 0.25f, 0.25f };
	}
}

// One interleaved vertex is 11 floats: pos(3) normal(3) colour(3) texcoord(2).
// Every buffer carries the same layout, so SetupAttribs is uniform.
constexpr int kVertexFloats = 11;

void PushVertex( std::vector<float> &out, const Vec3d &p, const Vec3d &n, const Color &c,
    float u = 0.0f, float v = 0.0f )
{
	out.insert( out.end(),
	    { static_cast<float>( p.x ), static_cast<float>( p.y ), static_cast<float>( p.z ),
	        static_cast<float>( n.x ), static_cast<float>( n.y ), static_cast<float>( n.z ), c.r,
	        c.g, c.b, u, v } );
}

void PushLine( std::vector<float> &out, const Vec3d &a, const Vec3d &b, const Color &c )
{
	PushVertex( out, a, Vec3d( 0, 0, 1 ), c );
	PushVertex( out, b, Vec3d( 0, 0, 1 ), c );
}

std::array<Vec3d, 8> BoxCorners( const Vec3d &lo, const Vec3d &hi )
{
	return { Vec3d( lo.x, lo.y, lo.z ), Vec3d( hi.x, lo.y, lo.z ), Vec3d( hi.x, hi.y, lo.z ),
	    Vec3d( lo.x, hi.y, lo.z ), Vec3d( lo.x, lo.y, hi.z ), Vec3d( hi.x, lo.y, hi.z ),
	    Vec3d( hi.x, hi.y, hi.z ), Vec3d( lo.x, hi.y, hi.z ) };
}

void PushBoxEdges( std::vector<float> &out, const Vec3d &lo, const Vec3d &hi, const Color &c )
{
	const std::array<Vec3d, 8> p = BoxCorners( lo, hi );
	static const int kEdges[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 },
	    { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
	for ( const auto &e : kEdges )
	{
		PushLine( out, p[e[0]], p[e[1]], c );
	}
}

// The six faces of a box as triangles, each with its outward normal.
int PushBoxFaces( std::vector<float> &out, const Vec3d &lo, const Vec3d &hi, const Color &c )
{
	const std::array<Vec3d, 8> p = BoxCorners( lo, hi );
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
		for ( int i : { q.a, q.b, q.c, q.a, q.c, q.d } )
		{
			PushVertex( out, p[i], q.n, c );
		}
	}
	return 12;
}

void SetupAttribs()
{
	const GLsizei stride = kVertexFloats * sizeof( float );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>( 0 ) );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(
	    1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer(
	    2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>( 6 * sizeof( float ) ) );
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer(
	    3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>( 9 * sizeof( float ) ) );
}

void Upload( GLuint vao, GLuint vbo, const std::vector<float> &data, GLenum usage )
{
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, static_cast<GLsizeiptr>( data.size() * sizeof( float ) ),
	    data.empty() ? nullptr : data.data(), usage );
	SetupAttribs();
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

} // namespace

Renderer::~Renderer()
{
	ReleaseGl();
}

void Renderer::ReleaseGl()
{
	if ( !m_initialized )
	{
		return;
	}
	const GLuint buffers[] = { m_meshVbo, m_lineVbo, m_dynVbo };
	glDeleteBuffers( 3, buffers );
	const GLuint arrays[] = { m_meshVao, m_lineVao, m_dynVao };
	glDeleteVertexArrays( 3, arrays );
	for ( const auto &kv : m_textures )
	{
		if ( kv.second )
		{
			GLuint t = kv.second;
			glDeleteTextures( 1, &t );
		}
	}
	m_textures.clear();
	if ( m_program )
	{
		glDeleteProgram( m_program );
	}
	m_meshVbo = m_lineVbo = m_dynVbo = 0;
	m_meshVao = m_lineVao = m_dynVao = m_program = 0;
	m_initialized = false;
}

unsigned int Renderer::TextureFor( const std::string &material )
{
	if ( !m_catalog || material.empty() )
	{
		return 0;
	}
	auto it = m_textures.find( material );
	if ( it != m_textures.end() )
	{
		return it->second;
	}

	GLuint tex = 0;
	const hammer::formats::VtfImage *image = m_catalog->BaseTextureImage( material );
	if ( image && image->width > 0 && image->height > 0 )
	{
		glGenTextures( 1, &tex );
		glBindTexture( GL_TEXTURE_2D, tex );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, image->width, image->height, 0, GL_RGBA,
		    GL_UNSIGNED_BYTE, image->rgba.data() );
		glGenerateMipmap( GL_TEXTURE_2D );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	m_textures.emplace( material, tex );
	return tex;
}

bool Renderer::Init( std::string &error )
{
	GLuint vs = 0;
	GLuint fs = 0;
	if ( !CompileShader( GL_VERTEX_SHADER, kVertexSrc, vs, error ) )
	{
		return false;
	}
	if ( !CompileShader( GL_FRAGMENT_SHADER, kFragmentSrc, fs, error ) )
	{
		glDeleteShader( vs );
		return false;
	}
	m_program = glCreateProgram();
	glAttachShader( m_program, vs );
	glAttachShader( m_program, fs );
	glLinkProgram( m_program );
	glDeleteShader( vs );
	glDeleteShader( fs );

	GLint linked = 0;
	glGetProgramiv( m_program, GL_LINK_STATUS, &linked );
	if ( !linked )
	{
		char log[1024] = { 0 };
		glGetProgramInfoLog( m_program, sizeof( log ) - 1, nullptr, log );
		error = std::string( "program link failed: " ) + log;
		glDeleteProgram( m_program );
		m_program = 0;
		return false;
	}

	glGenVertexArrays( 1, &m_meshVao );
	glGenBuffers( 1, &m_meshVbo );
	glGenVertexArrays( 1, &m_lineVao );
	glGenBuffers( 1, &m_lineVbo );
	glGenVertexArrays( 1, &m_dynVao );
	glGenBuffers( 1, &m_dynVbo );

	m_initialized = true;
	return true;
}

void Renderer::SetSnapshot( const hammer::viewport::RenderSnapshot &snapshot )
{
	if ( !m_initialized )
	{
		return;
	}

	std::vector<float> mesh;
	std::vector<float> lines;
	m_triCount = 0;
	m_solidCount = static_cast<int>( snapshot.solids.size() );
	m_entityCount = static_cast<int>( snapshot.entities.size() );
	m_meshRanges.clear();
	m_bounds = snapshot.bounds;

	// Appends a per-texture draw run, merging with the previous run when the
	// texture matches so a wall of same-material faces is one draw call.
	auto addRange = [&]( unsigned int texture, int firstVertex, int vertexCount )
	{
		if ( vertexCount <= 0 )
			return;
		if ( !m_meshRanges.empty() && m_meshRanges.back().texture == texture &&
		     m_meshRanges.back().firstVertex + m_meshRanges.back().vertexCount == firstVertex )
		{
			m_meshRanges.back().vertexCount += vertexCount;
		}
		else
		{
			m_meshRanges.push_back( { texture, firstVertex, vertexCount } );
		}
	};
	auto vertexCount = [&]()
	{
		return static_cast<int>( mesh.size() / kVertexFloats );
	};

	int solidIndex = 0;
	for ( const hammer::viewport::SolidDraw &solid : snapshot.solids )
	{
		Color fill = solid.selected ? kSelectedFill : SolidFill( solidIndex );
		Color edge = solid.selected          ? kSelectedEdge
		             : solid.owner.IsValid() ? FromRgb( solid.color )
		                                     : kEdge;
		++solidIndex;

		for ( const hammer::viewport::FaceDraw &face : solid.faces )
		{
			const int first = vertexCount();
			if ( face.displacement )
			{
				// Terrain: the displaced grid, blended grass -> dirt by vertex alpha.
				const mapgeometry::DisplacementSurface &disp = *face.displacement;
				const int n = static_cast<int>( disp.vertices.size() );
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
						const Color c =
						    face.selected || solid.selected
						        ? ( face.selected ? kSelectedFace : kSelectedFill )
						        : Color{ 0.32f + t * 0.20f, 0.48f - t * 0.08f, 0.28f + t * 0.02f };
						const Vec3d normal = i < static_cast<int>( disp.vertexNormals.size() )
						                         ? disp.vertexNormals[i]
						                         : face.normal;
						PushVertex( mesh, disp.vertices[i], normal, c );
					}
					++m_triCount;
					for ( int e = 0; e < 3; ++e )
					{
						PushLine( lines, disp.vertices[tri[e]], disp.vertices[tri[( e + 1 ) % 3]],
						    solid.selected ? edge : kDisplacementEdge );
					}
				}
				addRange( 0, first, vertexCount() - first );
				continue;
			}
			if ( face.vertices.size() < 3 )
			{
				continue;
			}

			// Selected solids and faces stay flat-shaded so the highlight reads;
			// ordinary faces are textured when the catalog resolves the material.
			const bool highlighted = solid.selected || face.selected;
			const unsigned int texture = highlighted ? 0 : TextureFor( face.material );

			// World-planar UVs: project onto the two world axes least aligned with
			// the face normal, at Source's default 0.25 units per texel.
			const double ax = std::fabs( face.normal.x );
			const double ay = std::fabs( face.normal.y );
			const double az = std::fabs( face.normal.z );
			const int aU = ( az >= ax && az >= ay ) ? 0 : ( ax >= ay ? 1 : 0 );
			const int aV = ( az >= ax && az >= ay ) ? 1 : 2;
			double worldU = 128.0;
			double worldV = 128.0;
			if ( texture )
			{
				if ( const hammer::formats::VtfImage *img =
				         m_catalog->BaseTextureImage( face.material ) )
				{
					worldU = ( img->width > 0 ? img->width : 512 ) * 0.25;
					worldV = ( img->height > 0 ? img->height : 512 ) * 0.25;
				}
			}
			auto uv = [&]( const Vec3d &p, int axis, double size )
			{
				return static_cast<float>( mapgeometry::Component( p, axis ) / size );
			};

			const Color faceFill = face.selected ? kSelectedFace : fill;
			const Vec3d &v0 = face.vertices[0];
			for ( std::size_t i = 1; i + 1 < face.vertices.size(); ++i )
			{
				for ( const Vec3d *p : { &v0, &face.vertices[i], &face.vertices[i + 1] } )
				{
					PushVertex( mesh, *p, face.normal, faceFill, uv( *p, aU, worldU ),
					    uv( *p, aV, worldV ) );
				}
				++m_triCount;
			}
			addRange( texture, first, vertexCount() - first );

			const Color faceEdge = face.selected ? kSelectedFace : edge;
			for ( std::size_t i = 0; i < face.vertices.size(); ++i )
			{
				PushLine( lines, face.vertices[i], face.vertices[( i + 1 ) % face.vertices.size()],
				    faceEdge );
			}
		}
	}

	// Point entities: their marker boxes, filled in 3D and outlined everywhere.
	for ( const hammer::viewport::EntityDraw &entity : snapshot.entities )
	{
		const hammer::scene::Rgb &rgb = entity.color;
		const Color color = entity.selected                              ? kSelectedFill
		                    : ( rgb.r == 0 && rgb.g == 0 && rgb.b == 0 ) ? kEntityDefault
		                                                                 : FromRgb( rgb );
		const int first = vertexCount();
		PushBoxFaces( mesh, entity.mins, entity.maxs, color );
		addRange( 0, first, vertexCount() - first );
		PushBoxEdges( lines, entity.mins, entity.maxs, entity.selected ? kSelectedEdge : color );
	}

	m_meshVertexCount = vertexCount();
	m_lineVertexCount = static_cast<int>( lines.size() / kVertexFloats );
	Upload( m_meshVao, m_meshVbo, mesh, GL_STATIC_DRAW );
	Upload( m_lineVao, m_lineVbo, lines, GL_STATIC_DRAW );
}

void Renderer::DrawDynamic(
    const std::vector<float> &vertices, unsigned int mode, const float mvp[16] )
{
	if ( vertices.empty() )
	{
		return;
	}
	Upload( m_dynVao, m_dynVbo, vertices, GL_DYNAMIC_DRAW );
	glUniformMatrix4fv( glGetUniformLocation( m_program, "uMVP" ), 1, GL_FALSE, mvp );
	glBindVertexArray( m_dynVao );
	glDrawArrays( mode, 0, static_cast<GLsizei>( vertices.size() / kVertexFloats ) );
	glBindVertexArray( 0 );
}

void Renderer::DrawOverlay( const hammer::tools::OverlayList &overlay, const float worldMvp[16],
    const float screenMvp[16], bool depthTest )
{
	std::vector<float> worldLines;
	std::vector<float> screenLines;
	std::vector<float> screenFill;
	auto screen = []( double x, double y )
	{
		return Vec3d( x, y, 0.0 );
	};
	for ( const OverlayItem &item : overlay.items )
	{
		const Color c = RoleColor( item.role );
		switch ( item.kind )
		{
		case OverlayKind::WorldLine:
			if ( item.world.size() >= 2 )
			{
				PushLine( worldLines, item.world[0], item.world[1], c );
			}
			break;
		case OverlayKind::WorldBox:
			if ( item.world.size() >= 2 )
			{
				PushBoxEdges( worldLines, item.world[0], item.world[1], c );
			}
			break;
		case OverlayKind::WorldPolygon:
			for ( std::size_t i = 0; i < item.world.size(); ++i )
			{
				PushLine( worldLines, item.world[i], item.world[( i + 1 ) % item.world.size()], c );
			}
			break;
		case OverlayKind::ScreenRect:
		{
			const Vec3d p[4] = { screen( item.a.x, item.a.y ), screen( item.b.x, item.a.y ),
			    screen( item.b.x, item.b.y ), screen( item.a.x, item.b.y ) };
			for ( int i = 0; i < 4; ++i )
			{
				PushLine( screenLines, p[i], p[( i + 1 ) % 4], c );
			}
			break;
		}
		case OverlayKind::ScreenHandle:
		{
			// A filled square, or an octagon for round handles.
			const int sides = item.shape == hammer::tools::HandleShape::Circle ? 8 : 4;
			const double start = sides == 4 ? kPi / 4.0 : 0.0;
			const double radius = sides == 4 ? item.size * std::sqrt( 2.0 ) : item.size;
			const Vec3d center = screen( item.a.x, item.a.y );
			for ( int i = 0; i < sides; ++i )
			{
				const double a0 = start + 2.0 * kPi * i / sides;
				const double a1 = start + 2.0 * kPi * ( i + 1 ) / sides;
				PushVertex( screenFill, center, Vec3d( 0, 0, 1 ), c );
				PushVertex( screenFill,
				    center + Vec3d( radius * std::cos( a0 ), radius * std::sin( a0 ), 0 ),
				    Vec3d( 0, 0, 1 ), c );
				PushVertex( screenFill,
				    center + Vec3d( radius * std::cos( a1 ), radius * std::sin( a1 ), 0 ),
				    Vec3d( 0, 0, 1 ), c );
			}
			break;
		}
		case OverlayKind::ScreenLabel:
			break; // no GL text; the host's status bar carries tool status
		}
	}
	glUniform1i( glGetUniformLocation( m_program, "uWire" ), 1 );
	glUniform1i( glGetUniformLocation( m_program, "uUseTexture" ), 0 );
	if ( depthTest )
	{
		glEnable( GL_DEPTH_TEST );
	}
	DrawDynamic( worldLines, GL_LINES, worldMvp );
	glDisable( GL_DEPTH_TEST );
	DrawDynamic( screenLines, GL_LINES, screenMvp );
	DrawDynamic( screenFill, GL_TRIANGLES, screenMvp );
}

void Renderer::Render3D(
    const Camera3D &camera, const hammer::tools::OverlayList &overlay, int fbWidth, int fbHeight )
{
	if ( !m_initialized || fbWidth <= 0 || fbHeight <= 0 || !camera.HasArea() )
	{
		return;
	}
	glViewport( 0, 0, fbWidth, fbHeight );
	glClearColor( 0.13f, 0.14f, 0.17f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	// Depth range: past the farthest corner of the scene from the eye.
	double zFar = 16384.0;
	if ( m_bounds )
	{
		for ( const Vec3d &corner : BoxCorners( m_bounds->mins, m_bounds->maxs ) )
		{
			zFar = std::max( zFar, mapgeometry::Length( corner - camera.Position() ) + 1024.0 );
		}
	}
	const double aspect =
	    static_cast<double>( camera.Width() ) / static_cast<double>( camera.Height() );
	const Mat4 mvp =
	    Multiply( Perspective( camera.Fov() * kPi / 180.0, aspect, 2.0, zFar ), ViewOf( camera ) );

	glUseProgram( m_program );
	glUniformMatrix4fv( glGetUniformLocation( m_program, "uMVP" ), 1, GL_FALSE, mvp.m );
	glUniform1i( glGetUniformLocation( m_program, "uWire" ), 0 );
	glUniform1i( glGetUniformLocation( m_program, "uTex" ), 0 ); // sampler on unit 0
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 1.0f, 1.0f );
	glBindVertexArray( m_meshVao );
	const GLint useTexLoc = glGetUniformLocation( m_program, "uUseTexture" );
	glActiveTexture( GL_TEXTURE0 );
	for ( const MeshRange &range : m_meshRanges )
	{
		glUniform1i( useTexLoc, range.texture ? 1 : 0 );
		if ( range.texture )
		{
			glBindTexture( GL_TEXTURE_2D, range.texture );
		}
		glDrawArrays( GL_TRIANGLES, range.firstVertex, range.vertexCount );
	}
	glBindTexture( GL_TEXTURE_2D, 0 );
	glDisable( GL_POLYGON_OFFSET_FILL );

	glUniform1i( glGetUniformLocation( m_program, "uWire" ), 1 );
	glBindVertexArray( m_lineVao );
	glDrawArrays( GL_LINES, 0, m_lineVertexCount );

	const Mat4 screenMvp = ScreenOf( camera.Width(), camera.Height() );
	DrawOverlay( overlay, mvp.m, screenMvp.m, false );
	glBindVertexArray( 0 );
	glUseProgram( 0 );
}

void Renderer::Render2D( const Camera2D &camera, const std::vector<GridLine> &grid,
    const hammer::tools::OverlayList &overlay, int fbWidth, int fbHeight )
{
	if ( !m_initialized || fbWidth <= 0 || fbHeight <= 0 || !camera.HasArea() )
	{
		return;
	}
	glViewport( 0, 0, fbWidth, fbHeight );
	glClearColor( 0.03f, 0.03f, 0.04f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	glUseProgram( m_program );
	glUniform1i( glGetUniformLocation( m_program, "uWire" ), 1 );
	glUniform1i( glGetUniformLocation( m_program, "uUseTexture" ), 0 );

	// Grid lines arrive in screen space (the workspace computed them from the
	// same camera and the grid policy).
	const Mat4 screenMvp = ScreenOf( camera.Width(), camera.Height() );
	std::vector<float> gridLines;
	for ( const GridLine &line : grid )
	{
		const bool vertical = line.orientation == GridLineOrientation::Vertical;
		Color c = { 0.13f, 0.13f, 0.15f };
		switch ( line.kind )
		{
		case GridLineKind::Major:
			c = { 0.22f, 0.22f, 0.24f };
			break;
		case GridLineKind::Block:
			c = { 0.26f, 0.26f, 0.36f };
			break;
		case GridLineKind::Axis:
			c = vertical ? Color{ 0.20f, 0.45f, 0.20f } : Color{ 0.45f, 0.20f, 0.20f };
			break;
		case GridLineKind::Minor:
			break;
		}
		const Vec3d a = vertical ? Vec3d( line.screen, 0, 0 ) : Vec3d( 0, line.screen, 0 );
		const Vec3d b = vertical ? Vec3d( line.screen, camera.Height(), 0 )
		                         : Vec3d( camera.Width(), line.screen, 0 );
		PushLine( gridLines, a, b, c );
	}
	DrawDynamic( gridLines, GL_LINES, screenMvp.m );

	const Mat4 mvp = OrthoOf( camera );
	glUniformMatrix4fv( glGetUniformLocation( m_program, "uMVP" ), 1, GL_FALSE, mvp.m );
	glBindVertexArray( m_lineVao );
	glDrawArrays( GL_LINES, 0, m_lineVertexCount );

	DrawOverlay( overlay, mvp.m, screenMvp.m, false );
	glBindVertexArray( 0 );
	glUseProgram( 0 );
}

} // namespace hammergtk
