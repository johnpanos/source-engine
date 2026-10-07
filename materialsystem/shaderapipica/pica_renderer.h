//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The 3DS shader API's renderer: draws textured, vertex-colored geometry
// through render.device.pica, the render core's PICA200 adapter (RFC 0026
// P5); nothing here reaches citro3d. It borrows the render core's device
// (BindDevice) and owns a 400x240 colour and
// depth target in screen orientation, per-frame rings of transient vertex
// and index data, and the device buffers behind the shader API's meshes.
//
// Conventions:
//  * clip space is the port's: D3D's (y up, depth 0 to 1, row 0 at the top),
//    so the material system's matrices are used as they are; the device's
//    presenter turns the target onto the top screen;
//  * vertex colors are D3DCOLOR bytes (B G R A); the vertex program swizzles;
//  * textures are uploaded in the port's raster layout (row 0 at the top).
//
// A frame is recorded into one encoder and submitted at EndFrame (or when a
// mesh's memory is about to be rewritten while a recorded draw reads it:
// PrepareWrite). Linear memory is a device upload buffer written in place.
//
//=============================================================================//

#ifndef PICA_RENDERER_H
#define PICA_RENDERER_H

#include "pica_texture.h"

#include <cstddef>
#include <cstdint>

namespace render::device
{
class IRenderDevice2;
class CommandEncoder;
}

namespace pica
{

// The vertex record the GPU reads (24 bytes).
struct Vertex
{
	float pos[3];
	std::uint8_t color[4]; // B G R A
	float uv[2];
};

// What linear memory holds: a device buffer is one or the other.
enum class Memory : std::uint8_t
{
	kVertices,
	kIndices
};

// The texel formats Texture::Upload takes, in the port's raster layout.
enum class UploadFormat : std::uint8_t
{
	kRGBA8, // 4 bytes per texel
	kETC1,  // render.device.v2's kETC1Rgb
	kETC1A4 // render.device.v2's kETC1A4
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
	kTriangleStrip
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

	// levels[i] holds level i in format (level 0 is width x height, each
	// next level half each side). Replaces any previous image.
	bool Upload( UploadFormat format, int width, int height, int levelCount,
	    const std::uint8_t *const *levels );
	void Release();
	bool Valid() const { return m_texture != 0; }
	// Repeat on both axes, else clamped (the port's sampler has one mode).
	void SetWrap( bool wrapS, bool wrapT );
	int Width() const { return m_width; }
	int Height() const { return m_height; }
	std::size_t Bytes() const { return m_bytes; }
	std::uint32_t Group(); // the material bind group drawing it (renderer use)
	std::uint32_t Id() const { return m_texture; } // the device's TextureId value
	bool Repeat() const { return m_repeat; }

private:
	std::uint32_t m_texture; // TextureId
	std::uint32_t m_group;   // BindGroupId, made on first draw
	bool m_repeat;
	int m_width;
	int m_height;
	std::size_t m_bytes;
};

struct Stats
{
	std::uint32_t draws = 0;
	std::uint32_t triangles = 0;
	std::uint32_t ringOverflows = 0;
	std::uint32_t blendRefusals = 0; // D3D factor pairs no port blend mode draws
	std::uint32_t submits = 0;
	std::uint32_t submitFailures = 0; // frames the device refused (not presented)
	std::uint32_t residencyRefusals = 0; // draws whose mesh found no linear memory
	std::size_t textureBytes = 0;
	std::size_t meshBytes = 0; // linear memory of the meshes (AllocLinear)
	std::size_t ringPeak[2] = {}; // the most of each transient ring a frame used
};

// Screen: the top screen, 400x240.
constexpr int kScreenWidth = 400;
constexpr int kScreenHeight = 240;

// The render core's device, which the launcher's composition owns (RFC 0026:
// one device on the 3DS). Bound before Init; it outlives Shutdown.
void BindDevice( render::device::IRenderDevice2 *device );

bool Init();
// Test fixture (-pica_linear_reserve): holds `bytes` of linear memory in a
// device buffer until Shutdown, so exhaustion paths run on demand.
bool ReserveLinear( std::size_t bytes );
void Shutdown();
bool Initialized();

void BeginFrame();
void EndFrame(); // presents; waits for vblank when vsync is on
bool InFrame();

void Clear( bool color, bool depth, std::uint32_t rgba );
// Screen-space viewport (D3D sense: origin top-left of the 400x240 screen).
void SetViewport( int x, int y, int width, int height );

// Transient memory for one draw of this frame.
void *AllocTransient( Memory kind, std::size_t bytes, std::size_t align = 16 );
// GPU-visible memory a mesh keeps (a device buffer written in place).
void *AllocLinear( Memory kind, std::size_t bytes );
void FreeLinear( void *ptr );
// Before rewriting memory a recorded draw may read: submits the frame so far.
void PrepareWrite( const void *ptr );
void FlushLinear( const void *ptr, std::size_t bytes );

// clipFromObject is the D3D row-vector matrix model * view * projection.
// vertices and indices are in memory from AllocLinear or AllocTransient.
void Draw( const float clipFromObject[16], const DrawState &state, Texture *texture,
    const Vertex *vertices, int vertexCount, const std::uint16_t *indices, int indexCount,
    Primitive primitive );

// A section of the frame for a render core pass (RFC 0026 P3, the core's
// passes at slots of this stream): ends this renderer's pass and returns the
// frame's encoder with the target in its home usages (color
// kColorAttachment, depth kDepthWrite, the frame's top-left 400x240); null
// outside a frame. EndCoreSection reopens the pass, loading what the section
// drew, with the current viewport.
struct CoreSectionTarget
{
	render::device::IRenderDevice2 *device = nullptr;
	std::uint32_t color = 0; // TextureId values
	std::uint32_t depth = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	// The recording's serial: it rises at every submission, so what the core
	// kept of earlier recordings can be released behind them.
	std::uint64_t serial = 0;
	// The last submission's token (CompletionToken's epoch and value).
	std::uint32_t submittedEpoch = 0;
	std::uint64_t submittedValue = 0;
};
render::device::CommandEncoder *BeginCoreSection( CoreSectionTarget &target );
void EndCoreSection();

// Writes the last presented frame as a binary PPM (400x240, RGB).
bool CaptureTopScreen( const char *path );

const Stats &FrameStats();

} // namespace pica

#endif // PICA_RENDERER_H
