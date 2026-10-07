// pica_lab: proves the 3DS fullbright renderer's conventions before the shader
// API uses them. It draws a fixed scene through pica_renderer with D3D-style
// matrices, writes the top screen to sdmc:/pica_lab.ppm and exits; the host
// check (tools/n3ds/pica_lab_check.py) judges the image.
//
// Scene, with a D3D perspective camera at the origin looking down +z:
//  * a textured quad filling the left half of the view: a 2x2 checker of
//    red (top-left), green (top-right), blue (bottom-left), white
//    (bottom-right), so orientation and texture tiling show;
//  * on the right, a yellow quad at depth 5 drawn first, then a magenta quad
//    at depth 10 drawn second that overlaps it: with the depth test the
//    yellow must stay in front;
//  * a back-facing (clockwise in D3D terms reversed) quad that culling must
//    remove, drawn at the bottom right in cyan;
//  * a 100x60 viewport at the top-left screen corner cleared... drawn with a
//    full-viewport grey quad: the corner must be grey.
#include <3ds.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "pica_renderer.h"
#include "pica_texture.h"

using namespace pica;

static void Identity( float m[16] )
{
	std::memset( m, 0, 16 * sizeof( float ) );
	m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// D3D left-handed perspective (row-vector), z in [0, 1].
static void Perspective( float m[16], float fovY, float aspect, float zn, float zf )
{
	std::memset( m, 0, 16 * sizeof( float ) );
	const float yScale = 1.0f / std::tan( fovY * 0.5f );
	m[0] = yScale / aspect;
	m[5] = yScale;
	m[10] = zf / ( zf - zn );
	m[11] = 1.0f;
	m[14] = -zn * zf / ( zf - zn );
}

static Vertex V( float x, float y, float z, std::uint32_t bgra, float u, float v )
{
	Vertex out;
	out.pos[0] = x, out.pos[1] = y, out.pos[2] = z;
	std::memcpy( out.color, &bgra, 4 );
	out.uv[0] = u, out.uv[1] = v;
	return out;
}

// A quad (two triangles, clockwise as seen by a D3D camera = front facing).
static void Quad( std::vector<Vertex> &out, float x0, float y0, float x1, float y1, float z,
	std::uint32_t bgra, bool front = true )
{
	Vertex a = V( x0, y1, z, bgra, 0, 0 ), b = V( x1, y1, z, bgra, 1, 0 );
	Vertex c = V( x1, y0, z, bgra, 1, 1 ), d = V( x0, y0, z, bgra, 0, 1 );
	if ( front )
		out.insert( out.end(), { a, b, c, a, c, d } );
	else
		out.insert( out.end(), { a, c, b, a, d, c } );
}

static void DrawList( const float mvp[16], const DrawState &state, Texture *tex,
	const std::vector<Vertex> &verts )
{
	Vertex *gpu = static_cast<Vertex *>( AllocTransient( verts.size() * sizeof( Vertex ) ) );
	std::memcpy( gpu, verts.data(), verts.size() * sizeof( Vertex ) );
	FlushLinear( gpu, verts.size() * sizeof( Vertex ) );
	Draw( mvp, state, tex, gpu, int( verts.size() ), nullptr, 0, Primitive::kTriangles );
}

int main()
{
	gfxInitDefault();
	consoleInit( GFX_BOTTOM, nullptr );
	if ( !Init() )
	{
		std::printf( "pica::Init failed\n" );
		return 1;
	}

	// 16x16 checker texture through the RGBA8 -> ETC1 path the shader API uses.
	std::vector<std::uint8_t> rgba( 16 * 16 * 4 );
	for ( int y = 0; y < 16; ++y )
		for ( int x = 0; x < 16; ++x )
		{
			std::uint8_t *p = &rgba[( y * 16 + x ) * 4];
			const bool right = x >= 8, bottom = y >= 8;
			const std::uint8_t colors[4][3] = { { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 }, { 255, 255, 255 } };
			std::memcpy( p, colors[( bottom ? 2 : 0 ) + ( right ? 1 : 0 )], 3 );
			p[3] = 255;
		}
	std::vector<std::uint8_t> level0, level1;
	EncodeLevel( TexFormat::kRGBA8, rgba.data(), 16, 16, level0 );
	std::vector<std::uint8_t> half( 8 * 8 * 4 );
	Resample( rgba.data(), 16, 16, half.data(), 8, 8 );
	EncodeLevel( TexFormat::kRGBA8, half.data(), 8, 8, level1 );
	const std::uint8_t *levels[2] = { level0.data(), level1.data() };
	Texture checker;
	checker.Upload( TexFormat::kRGBA8, 16, 16, 2, levels );

	float proj[16];
	Perspective( proj, 1.0f, 400.0f / 240.0f, 1.0f, 100.0f );

	for ( int frame = 0; frame < 4; ++frame )
	{
		BeginFrame();
		Clear( true, true, 0x000000FF );
		DrawState state;
		const std::uint32_t white = 0xFFFFFFFF;

		// Left half: textured checker at depth 4 (view half-width at z=4 is
		// tan(0.5)*4*aspect ~ 3.64).
		std::vector<Vertex> verts;
		Quad( verts, -3.6f, -2.0f, -0.2f, 2.0f, 4.0f, white );
		DrawList( proj, state, &checker, verts );

		// Right: yellow at depth 5, then magenta behind it at depth 10.
		verts.clear();
		Quad( verts, 0.5f, 0.0f, 3.0f, 2.0f, 5.0f, 0xFFFFFF00 ); // yellow (D3DCOLOR 0xAARRGGBB)
		DrawList( proj, state, nullptr, verts );
		verts.clear();
		Quad( verts, 2.0f, 0.4f, 8.0f, 6.0f, 10.0f, 0xFFFF00FF ); // magenta
		DrawList( proj, state, nullptr, verts );

		// Bottom right: a back-facing cyan quad; culling removes it.
		verts.clear();
		Quad( verts, 0.5f, -2.0f, 3.0f, -0.5f, 5.0f, 0xFF00FFFF, false ); // cyan
		DrawList( proj, state, nullptr, verts );

		// Top-left 100x60 viewport: a full-viewport grey quad (identity
		// matrix, D3D clip space, depth 0.5).
		SetViewport( 0, 0, 100, 60 );
		float identity[16];
		Identity( identity );
		verts.clear();
		Quad( verts, -1.0f, -1.0f, 1.0f, 1.0f, 0.5f, 0xFF808080 );
		DrawState flat;
		flat.depthTest = false;
		flat.depthWrite = false;
		DrawList( identity, flat, nullptr, verts );
		SetViewport( 0, 0, kScreenWidth, kScreenHeight );

		EndFrame();
	}
	gspWaitForVBlank();
	gspWaitForVBlank();
	const bool captured = CaptureTopScreen( "sdmc:/pica_lab.ppm" );
	std::printf( "pica_lab: capture %s, draws %lu\n", captured ? "ok" : "FAILED",
		(unsigned long)FrameStats().draws );
	FILE *done = std::fopen( "sdmc:/pica_lab.done", "w" );
	if ( done )
	{
		std::fprintf( done, "%s\n", captured ? "ok" : "failed" );
		std::fclose( done );
	}
	checker.Release();
	Shutdown();
	gfxExit();
	return 0;
}
