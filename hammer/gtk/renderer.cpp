//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the GTK Hammer viewport renderer (see renderer.h).
//			Uses libepoxy for GL entry points. No GTK/GDK includes: the caller owns
//			context/current-ness and framebuffer binding. Renders either a shaded
//			perspective camera view or a 2D orthographic wireframe-with-grid view.
//
//=============================================================================//

#include "renderer.h"

#include <epoxy/gl.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace hammergtk
{

namespace
{

constexpr float kPi = 3.14159265358979323846f;

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

Mat4 Perspective( float fovyRad, float aspect, float zNear, float zFar )
{
	Mat4 r;
	for ( float &v : r.m )
	{
		v = 0.0f;
	}
	const float f = 1.0f / std::tan( fovyRad * 0.5f );
	r.m[0] = f / aspect;
	r.m[5] = f;
	r.m[10] = ( zFar + zNear ) / ( zNear - zFar );
	r.m[11] = -1.0f;
	r.m[14] = ( 2.0f * zFar * zNear ) / ( zNear - zFar );
	return r;
}

void Normalize3( float v[3] )
{
	const float len = std::sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
	if ( len > 1.0e-8f )
	{
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
	}
}

void Cross3( const float a[3], const float b[3], float out[3] )
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

Mat4 LookAt( const float eye[3], const float center[3], const float up[3] )
{
	float f[3] = { center[0] - eye[0], center[1] - eye[1], center[2] - eye[2] };
	Normalize3( f );
	float s[3];
	Cross3( f, up, s );
	Normalize3( s );
	float u[3];
	Cross3( s, f, u );

	Mat4 r;
	r.m[0] = s[0];
	r.m[4] = s[1];
	r.m[8] = s[2];
	r.m[1] = u[0];
	r.m[5] = u[1];
	r.m[9] = u[2];
	r.m[2] = -f[0];
	r.m[6] = -f[1];
	r.m[10] = -f[2];
	r.m[12] = -( s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2] );
	r.m[13] = -( u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2] );
	r.m[14] = f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2];
	r.m[15] = 1.0f;
	return r;
}

// ---- Shaders ---------------------------------------------------------------

const char *kVertexSrc = "#version 330 core\n"
                         "layout(location=0) in vec3 aPos;\n"
                         "layout(location=1) in vec3 aNormal;\n"
                         "layout(location=2) in vec3 aColor;\n"
                         "uniform mat4 uMVP;\n"
                         "out vec3 vNormal;\n"
                         "out vec3 vColor;\n"
                         "void main(){\n"
                         "  vNormal = aNormal;\n"
                         "  vColor = aColor;\n"
                         "  gl_Position = uMVP * vec4(aPos, 1.0);\n"
                         "}\n";

const char *kFragmentSrc = "#version 330 core\n"
                           "in vec3 vNormal;\n"
                           "in vec3 vColor;\n"
                           "uniform int uWire;\n"
                           "uniform int uOverride;\n"
                           "uniform vec3 uOverrideColor;\n"
                           "out vec4 fragColor;\n"
                           "void main(){\n"
                           "  if (uWire == 1) {\n"
                           "    fragColor = vec4(uOverride == 1 ? uOverrideColor : vColor, 1.0);\n"
                           "    return;\n"
                           "  }\n"
                           "  vec3 n = normalize(vNormal);\n"
                           "  vec3 l1 = normalize(vec3(0.4, 0.6, 0.8));\n"
                           "  vec3 l2 = normalize(vec3(-0.5, -0.3, 0.4));\n"
                           "  float d = 0.35 + 0.55*max(dot(n,l1),0.0) + 0.25*max(dot(n,l2),0.0);\n"
                           "  fragColor = vec4(vColor * clamp(d,0.0,1.0), 1.0);\n"
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

void SolidColor( int index, float out[3] )
{
	const std::uint32_t h = static_cast<std::uint32_t>( index ) * 2654435761u;
	out[0] = 0.45f + 0.5f * ( ( h & 0xFF ) / 255.0f );
	out[1] = 0.45f + 0.5f * ( ( ( h >> 8 ) & 0xFF ) / 255.0f );
	out[2] = 0.45f + 0.5f * ( ( ( h >> 16 ) & 0xFF ) / 255.0f );
}

// Pushes one interleaved vertex (pos, normal, colour) onto a buffer.
void PushVertex( std::vector<float> &out, float x, float y, float z, float nx, float ny, float nz,
    float r, float g, float b )
{
	out.push_back( x );
	out.push_back( y );
	out.push_back( z );
	out.push_back( nx );
	out.push_back( ny );
	out.push_back( nz );
	out.push_back( r );
	out.push_back( g );
	out.push_back( b );
}

void SetupAttribs()
{
	const GLsizei stride = 9 * sizeof( float );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>( 0 ) );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(
	    1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer(
	    2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>( 6 * sizeof( float ) ) );
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
	const GLuint buffers[] = { m_meshVbo, m_lineVbo, m_gridVbo };
	glDeleteBuffers( 3, buffers );
	const GLuint arrays[] = { m_meshVao, m_lineVao, m_gridVao };
	glDeleteVertexArrays( 3, arrays );
	if ( m_program )
	{
		glDeleteProgram( m_program );
	}
	m_meshVbo = m_lineVbo = m_gridVbo = 0;
	m_meshVao = m_lineVao = m_gridVao = m_program = 0;
	m_initialized = false;
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
	glGenVertexArrays( 1, &m_gridVao );
	glGenBuffers( 1, &m_gridVbo );

	m_initialized = true;
	return true;
}

void Renderer::SetScene( const hammer::geometry::WorldScene &scene )
{
	if ( !m_initialized )
	{
		return;
	}

	std::vector<float> mesh;
	std::vector<float> lines;
	m_triCount = 0;
	m_solidCount = static_cast<int>( scene.solids.size() );

	int solidIndex = 0;
	for ( const hammer::geometry::BrushSolid &solid : scene.solids )
	{
		// A negative id is the in-progress "pending" box; an id matching the
		// highlight is the selected brush. Both get a distinct fill/edge colour.
		const bool pending = solid.id < 0;
		const bool selected = solid.id == m_highlightId;

		float color[3];
		SolidColor( solidIndex++, color );
		float edge[3] = { 0.50f, 0.52f, 0.58f };
		if ( pending )
		{
			color[0] = 0.90f;
			color[1] = 0.80f;
			color[2] = 0.30f;
			edge[0] = 1.00f;
			edge[1] = 0.88f;
			edge[2] = 0.30f;
		}
		else if ( selected )
		{
			color[0] = 1.00f;
			color[1] = 0.62f;
			color[2] = 0.28f;
			edge[0] = 1.00f;
			edge[1] = 0.58f;
			edge[2] = 0.15f;
		}

		for ( const hammer::geometry::BrushFace &face : solid.faces )
		{
			if ( face.vertices.size() < 3 )
			{
				continue;
			}
			const float n[3] = { static_cast<float>( face.plane.normal.x ),
			    static_cast<float>( face.plane.normal.y ),
			    static_cast<float>( face.plane.normal.z ) };
			const hammer::geometry::Vec3d &v0 = face.vertices[0];
			for ( std::size_t i = 1; i + 1 < face.vertices.size(); ++i )
			{
				const hammer::geometry::Vec3d tri[3] = {
				    v0, face.vertices[i], face.vertices[i + 1] };
				for ( const hammer::geometry::Vec3d &p : tri )
				{
					PushVertex( mesh, static_cast<float>( p.x ), static_cast<float>( p.y ),
					    static_cast<float>( p.z ), n[0], n[1], n[2], color[0], color[1], color[2] );
				}
				++m_triCount;
			}

			for ( std::size_t i = 0; i < face.vertices.size(); ++i )
			{
				const hammer::geometry::Vec3d &a = face.vertices[i];
				const hammer::geometry::Vec3d &b = face.vertices[( i + 1 ) % face.vertices.size()];
				PushVertex( lines, static_cast<float>( a.x ), static_cast<float>( a.y ),
				    static_cast<float>( a.z ), 0, 0, 1, edge[0], edge[1], edge[2] );
				PushVertex( lines, static_cast<float>( b.x ), static_cast<float>( b.y ),
				    static_cast<float>( b.z ), 0, 0, 1, edge[0], edge[1], edge[2] );
			}
		}
	}

	m_meshVertexCount = static_cast<int>( mesh.size() / 9 );
	m_lineVertexCount = static_cast<int>( lines.size() / 9 );

	glBindVertexArray( m_meshVao );
	glBindBuffer( GL_ARRAY_BUFFER, m_meshVbo );
	glBufferData( GL_ARRAY_BUFFER, static_cast<GLsizeiptr>( mesh.size() * sizeof( float ) ),
	    mesh.empty() ? nullptr : mesh.data(), GL_STATIC_DRAW );
	SetupAttribs();

	glBindVertexArray( m_lineVao );
	glBindBuffer( GL_ARRAY_BUFFER, m_lineVbo );
	glBufferData( GL_ARRAY_BUFFER, static_cast<GLsizeiptr>( lines.size() * sizeof( float ) ),
	    lines.empty() ? nullptr : lines.data(), GL_STATIC_DRAW );
	SetupAttribs();

	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	if ( scene.bounded )
	{
		m_sceneCenter[0] = static_cast<float>( ( scene.mins.x + scene.maxs.x ) * 0.5 );
		m_sceneCenter[1] = static_cast<float>( ( scene.mins.y + scene.maxs.y ) * 0.5 );
		m_sceneCenter[2] = static_cast<float>( ( scene.mins.z + scene.maxs.z ) * 0.5 );
		const float dx = static_cast<float>( scene.maxs.x - scene.mins.x );
		const float dy = static_cast<float>( scene.maxs.y - scene.mins.y );
		const float dz = static_cast<float>( scene.maxs.z - scene.mins.z );
		m_sceneSize[0] = dx;
		m_sceneSize[1] = dy;
		m_sceneSize[2] = dz;
		m_sceneRadius = 0.5f * std::sqrt( dx * dx + dy * dy + dz * dz );
		if ( m_sceneRadius < 1.0f )
		{
			m_sceneRadius = 1.0f;
		}
		m_haveScene = true;
	}
	else
	{
		m_haveScene = false;
	}
	// Note: SetScene does NOT move the camera, so live edits keep the current
	// view. Callers frame explicitly via FrameScene() after a load/new/reset.
}

void Renderer::AxisIndices( int &uAxis, int &vAxis ) const
{
	switch ( m_mode )
	{
	case ViewMode::Top: // X / Y
		uAxis = 0;
		vAxis = 1;
		break;
	case ViewMode::Front: // X / Z
		uAxis = 0;
		vAxis = 2;
		break;
	case ViewMode::Side: // Y / Z
		uAxis = 1;
		vAxis = 2;
		break;
	default:
		uAxis = 0;
		vAxis = 1;
		break;
	}
}

void Renderer::FrameScene()
{
	m_camera.target[0] = m_sceneCenter[0];
	m_camera.target[1] = m_sceneCenter[1];
	m_camera.target[2] = m_sceneCenter[2];
	m_camera.distance = m_sceneRadius * 2.6f + 64.0f;
	m_camera.yawDeg = 45.0f;
	m_camera.pitchDeg = 28.0f;

	int uAxis = 0;
	int vAxis = 1;
	AxisIndices( uAxis, vAxis );
	m_ortho.panU = m_sceneCenter[uAxis];
	m_ortho.panV = m_sceneCenter[vAxis];
	// Fit this view's own 2D extent with a margin, relative to a nominal viewport.
	const float fit = std::max( m_sceneSize[uAxis], m_sceneSize[vAxis] ) + 128.0f;
	m_ortho.pixelsPerUnit = ( 0.85f * 620.0f ) / fit;
}

void Renderer::DragBy( float dxPixels, float dyPixels )
{
	if ( m_mode == ViewMode::Perspective )
	{
		m_camera.yawDeg += -dxPixels * 0.4f;
		m_camera.pitchDeg += dyPixels * 0.4f;
		if ( m_camera.pitchDeg > 89.0f )
		{
			m_camera.pitchDeg = 89.0f;
		}
		if ( m_camera.pitchDeg < -89.0f )
		{
			m_camera.pitchDeg = -89.0f;
		}
	}
	else
	{
		// Pan: screen +y is down, world V axis is up, so invert dy.
		m_ortho.panU -= dxPixels / m_ortho.pixelsPerUnit;
		m_ortho.panV += dyPixels / m_ortho.pixelsPerUnit;
	}
}

void Renderer::ZoomBy( float factor )
{
	if ( m_mode == ViewMode::Perspective )
	{
		m_camera.distance *= factor;
		if ( m_camera.distance < 8.0f )
		{
			m_camera.distance = 8.0f;
		}
		if ( m_camera.distance > 1.0e6f )
		{
			m_camera.distance = 1.0e6f;
		}
	}
	else
	{
		m_ortho.pixelsPerUnit /= factor;
		if ( m_ortho.pixelsPerUnit < 1.0e-4f )
		{
			m_ortho.pixelsPerUnit = 1.0e-4f;
		}
		if ( m_ortho.pixelsPerUnit > 64.0f )
		{
			m_ortho.pixelsPerUnit = 64.0f;
		}
	}
}

void Renderer::PanScroll( float dxUnits, float dyUnits )
{
	if ( m_mode == ViewMode::Perspective )
	{
		// Two-finger scroll orbits the camera.
		DragBy( -dxUnits, -dyUnits );
		return;
	}
	// Natural pan: the canvas follows the fingers.
	m_ortho.panU += dxUnits / m_ortho.pixelsPerUnit;
	m_ortho.panV -= dyUnits / m_ortho.pixelsPerUnit;
}

void Renderer::ZoomAtPixel( float factor, float px, float py, int widthPx, int heightPx )
{
	if ( m_mode == ViewMode::Perspective )
	{
		ZoomBy( 1.0f / factor ); // factor>1 zooms in; dolly closer
		return;
	}
	float wu = 0.0f;
	float wv = 0.0f;
	PixelToWorld( px, py, widthPx, heightPx, wu, wv );
	m_ortho.pixelsPerUnit *= factor;
	if ( m_ortho.pixelsPerUnit < 1.0e-4f )
	{
		m_ortho.pixelsPerUnit = 1.0e-4f;
	}
	if ( m_ortho.pixelsPerUnit > 64.0f )
	{
		m_ortho.pixelsPerUnit = 64.0f;
	}
	// Re-anchor so the same world point stays under the cursor.
	m_ortho.panU = wu - ( px - widthPx * 0.5f ) / m_ortho.pixelsPerUnit;
	m_ortho.panV = wv + ( py - heightPx * 0.5f ) / m_ortho.pixelsPerUnit;
}

void Renderer::PixelToWorld(
    float px, float py, int widthPx, int heightPx, float &outU, float &outV ) const
{
	outU = m_ortho.panU + ( px - widthPx * 0.5f ) / m_ortho.pixelsPerUnit;
	outV = m_ortho.panV - ( py - heightPx * 0.5f ) / m_ortho.pixelsPerUnit;
}

void Renderer::RenderPerspective( int widthPx, int heightPx )
{
	glClearColor( 0.13f, 0.14f, 0.17f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	const float yaw = m_camera.yawDeg * kPi / 180.0f;
	const float pitch = m_camera.pitchDeg * kPi / 180.0f;
	const float eye[3] = {
	    m_camera.target[0] + m_camera.distance * std::cos( pitch ) * std::cos( yaw ),
	    m_camera.target[1] + m_camera.distance * std::cos( pitch ) * std::sin( yaw ),
	    m_camera.target[2] + m_camera.distance * std::sin( pitch ),
	};
	const float up[3] = { 0.0f, 0.0f, 1.0f };
	const float aspect = static_cast<float>( widthPx ) / static_cast<float>( heightPx );
	const float zFar = m_camera.distance + m_sceneRadius * 4.0f + 1024.0f;
	const Mat4 mvp = Multiply( Perspective( 60.0f * kPi / 180.0f, aspect, 4.0f, zFar ),
	    LookAt( eye, m_camera.target, up ) );

	glUseProgram( m_program );
	glUniformMatrix4fv( glGetUniformLocation( m_program, "uMVP" ), 1, GL_FALSE, mvp.m );
	glUniform1i( glGetUniformLocation( m_program, "uOverride" ), 0 );

	glUniform1i( glGetUniformLocation( m_program, "uWire" ), 0 );
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 1.0f, 1.0f );
	glBindVertexArray( m_meshVao );
	glDrawArrays( GL_TRIANGLES, 0, m_meshVertexCount );
	glDisable( GL_POLYGON_OFFSET_FILL );

	glUniform1i( glGetUniformLocation( m_program, "uWire" ), 1 );
	glBindVertexArray( m_lineVao );
	glDrawArrays( GL_LINES, 0, m_lineVertexCount );
}

void Renderer::BuildGrid( int widthPx, int heightPx )
{
	int uAxis = 0;
	int vAxis = 1;
	AxisIndices( uAxis, vAxis );

	const float halfU = ( widthPx * 0.5f ) / m_ortho.pixelsPerUnit;
	const float halfV = ( heightPx * 0.5f ) / m_ortho.pixelsPerUnit;
	const float minU = m_ortho.panU - halfU;
	const float maxU = m_ortho.panU + halfU;
	const float minV = m_ortho.panV - halfV;
	const float maxV = m_ortho.panV + halfV;

	// Choose a grid step that keeps lines at least ~7px apart (Hammer powers of 2).
	float step = 64.0f;
	while ( step * m_ortho.pixelsPerUnit < 7.0f )
	{
		step *= 2.0f;
	}

	std::vector<float> grid;
	auto pushLine = [&]( float u0, float v0, float u1, float v1, float r, float g, float b )
	{
		float p0[3] = { 0, 0, 0 };
		float p1[3] = { 0, 0, 0 };
		p0[uAxis] = u0;
		p0[vAxis] = v0;
		p1[uAxis] = u1;
		p1[vAxis] = v1;
		PushVertex( grid, p0[0], p0[1], p0[2], 0, 0, 1, r, g, b );
		PushVertex( grid, p1[0], p1[1], p1[2], 0, 0, 1, r, g, b );
	};

	const float startU = std::ceil( minU / step ) * step;
	for ( float u = startU; u <= maxU; u += step )
	{
		const bool major = std::fabs( std::fmod( u, step * 8.0f ) ) < 0.5f;
		const float c = major ? 0.22f : 0.13f;
		pushLine( u, minV, u, maxV, c, c, c + 0.02f );
	}
	const float startV = std::ceil( minV / step ) * step;
	for ( float v = startV; v <= maxV; v += step )
	{
		const bool major = std::fabs( std::fmod( v, step * 8.0f ) ) < 0.5f;
		const float c = major ? 0.22f : 0.13f;
		pushLine( minU, v, maxU, v, c, c, c + 0.02f );
	}

	// Origin axes (Hammer draws the world axes brighter).
	if ( minU <= 0.0f && maxU >= 0.0f )
	{
		pushLine( 0.0f, minV, 0.0f, maxV, 0.20f, 0.45f, 0.20f );
	}
	if ( minV <= 0.0f && maxV >= 0.0f )
	{
		pushLine( minU, 0.0f, maxU, 0.0f, 0.45f, 0.20f, 0.20f );
	}

	m_gridVertexCount = static_cast<int>( grid.size() / 9 );
	glBindVertexArray( m_gridVao );
	glBindBuffer( GL_ARRAY_BUFFER, m_gridVbo );
	glBufferData( GL_ARRAY_BUFFER, static_cast<GLsizeiptr>( grid.size() * sizeof( float ) ),
	    grid.empty() ? nullptr : grid.data(), GL_DYNAMIC_DRAW );
	SetupAttribs();
	glBindVertexArray( 0 );
}

void Renderer::RenderOrtho( int widthPx, int heightPx )
{
	glClearColor( 0.03f, 0.03f, 0.04f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	int uAxis = 0;
	int vAxis = 1;
	AxisIndices( uAxis, vAxis );

	const float halfU = ( widthPx * 0.5f ) / m_ortho.pixelsPerUnit;
	const float halfV = ( heightPx * 0.5f ) / m_ortho.pixelsPerUnit;

	// Build an MVP that maps the two selected world axes to NDC.
	Mat4 mvp;
	for ( float &v : mvp.m )
	{
		v = 0.0f;
	}
	mvp.m[uAxis * 4 + 0] = 1.0f / halfU; // world[uAxis] -> ndc.x
	mvp.m[vAxis * 4 + 1] = 1.0f / halfV; // world[vAxis] -> ndc.y
	mvp.m[12] = -m_ortho.panU / halfU;   // translate x
	mvp.m[13] = -m_ortho.panV / halfV;   // translate y
	mvp.m[15] = 1.0f;

	glUseProgram( m_program );
	glUniformMatrix4fv( glGetUniformLocation( m_program, "uMVP" ), 1, GL_FALSE, mvp.m );
	glUniform1i( glGetUniformLocation( m_program, "uWire" ), 1 );

	// Grid (per-vertex colours).
	BuildGrid( widthPx, heightPx );
	glUniform1i( glGetUniformLocation( m_program, "uOverride" ), 0 );
	glBindVertexArray( m_gridVao );
	glDrawArrays( GL_LINES, 0, m_gridVertexCount );

	// Scene edges in their per-brush colours (normal grey, selected orange,
	// pending yellow) -- Hammer's 2D wireframe with selection feedback.
	glUniform1i( glGetUniformLocation( m_program, "uOverride" ), 0 );
	glBindVertexArray( m_lineVao );
	glDrawArrays( GL_LINES, 0, m_lineVertexCount );
}

void Renderer::Render( int widthPx, int heightPx )
{
	if ( !m_initialized || heightPx <= 0 || widthPx <= 0 )
	{
		return;
	}
	glViewport( 0, 0, widthPx, heightPx );
	if ( m_mode == ViewMode::Perspective )
	{
		RenderPerspective( widthPx, heightPx );
	}
	else
	{
		RenderOrtho( widthPx, heightPx );
	}
	glBindVertexArray( 0 );
	glUseProgram( 0 );
}

} // namespace hammergtk
