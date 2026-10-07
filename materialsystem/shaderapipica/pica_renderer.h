//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The 3DS fullbright renderer: draws textured, vertex-colored geometry on the
// PICA200 through citro3d. It owns the GPU context, the top-screen render
// target, a per-frame linear-memory ring for transient vertex and index data,
// and the conversion from D3D clip space (what the material system's
// matrices produce) to PICA clip space.
//
// Conventions (each proven by tools/n3ds/pica_lab before the shader API used
// them):
//  * the top screen is 400x240, scanned out from a 240x400 framebuffer
//    rotated a quarter turn: clip x' = y, y' = -x;
//  * PICA clip depth is [-w, 0] with the near plane at -w, depth written as
//    -z/w (C3D_DepthMap(true, -1, 0)), so D3D's z' = z - w and every D3D
//    depth comparison is reversed;
//  * vertex colors are D3DCOLOR bytes (B G R A); the shader swizzles them.
//
// Only the 3DS build compiles this file (it needs libctru and citro3d).
//
//=============================================================================//

#ifndef PICA_RENDERER_H
#define PICA_RENDERER_H

#include "pica_texture.h"

#include <cstddef>
#include <cstdint>

namespace pica
{

// The vertex record the GPU reads (24 bytes).
struct Vertex
{
	float pos[3];
	std::uint8_t color[4]; // B G R A
	float uv[2];
};
static_assert( sizeof( Vertex ) == 24, "PICA vertex record" );

enum class Compare : std::uint8_t
{
	kNever,
	kLess,
	kEqual,
	kLessEqual,
	kGreater,
	kNotEqual,
	kGreaterEqual,
	kAlways
};

// D3D blend factors (ShaderBlendFactor_t order is mapped by the caller).
enum class Blend : std::uint8_t
{
	kZero,
	kOne,
	kSrcColor,
	kOneMinusSrcColor,
	kDstColor,
	kOneMinusDstColor,
	kSrcAlpha,
	kOneMinusSrcAlpha,
	kDstAlpha,
	kOneMinusDstAlpha,
	kSrcAlphaSaturate
};

enum class Primitive : std::uint8_t
{
	kTriangles,
	kTriangleStrip,
	kTriangleFan
};

struct DrawState
{
	bool depthTest = true;
	bool depthWrite = true;
	Compare depthFunc = Compare::kLessEqual; // D3D sense
	bool blend = false;
	Blend src = Blend::kOne;
	Blend dst = Blend::kZero;
	bool alphaTest = false;
	Compare alphaFunc = Compare::kGreaterEqual;
	std::uint8_t alphaRef = 0;
	bool cull = true;
	bool colorWrite = true;
	bool alphaWrite = true;
	float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
};

class Texture
{
public:
	Texture();
	~Texture();
	Texture( const Texture & ) = delete;
	Texture &operator=( const Texture & ) = delete;

	// levels[i] holds level i encoded in format (level 0 is width x height,
	// each next level half each side). Replaces any previous image.
	bool Upload( TexFormat format, int width, int height, int levelCount,
		const std::uint8_t *const *levels );
	void Release();
	bool Valid() const { return m_valid; }
	void SetWrap( bool wrapS, bool wrapT );
	int Width() const { return m_width; }
	int Height() const { return m_height; }
	std::size_t Bytes() const { return m_bytes; }
	void *Native() { return m_native; }

private:
	void *m_native; // C3D_Tex
	bool m_valid;
	int m_width;
	int m_height;
	std::size_t m_bytes;
};

struct Stats
{
	std::uint32_t draws = 0;
	std::uint32_t triangles = 0;
	std::uint32_t ringOverflows = 0;
	std::size_t textureBytes = 0;
};

// Screen: the top screen, 400x240.
constexpr int kScreenWidth = 400;
constexpr int kScreenHeight = 240;

bool Init();
void Shutdown();
bool Initialized();

void BeginFrame();
void EndFrame(); // presents; waits for vblank when vsync is on
bool InFrame();

void Clear( bool color, bool depth, std::uint32_t rgba );
// Screen-space viewport (D3D sense: origin top-left of the 400x240 screen).
void SetViewport( int x, int y, int width, int height );

// Transient linear memory valid until the frame after next starts.
void *AllocTransient( std::size_t bytes, std::size_t align = 16 );
// True when ptr is in GPU-visible linear memory (static buffers must be).
bool IsLinear( const void *ptr );
void *AllocLinear( std::size_t bytes );
void FreeLinear( void *ptr );
void FlushLinear( const void *ptr, std::size_t bytes );

// clipFromObject is the D3D row-vector matrix model * view * projection.
// vertices and indices are in linear memory.
void Draw( const float clipFromObject[16], const DrawState &state, Texture *texture,
	const Vertex *vertices, int vertexCount, const std::uint16_t *indices, int indexCount,
	Primitive primitive );

// Writes the last presented top screen as a binary PPM (400x240, RGB).
bool CaptureTopScreen( const char *path );

const Stats &FrameStats();

// The D3D -> PICA clip conversion, exposed for the host oracle.
void ConvertClip( const float d3dRowMajor[16], float picaRows[16] );

} // namespace pica

#endif // PICA_RENDERER_H
