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
#include "render/device/resources.h"

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
	kRGBA8,  // 4 bytes per texel
	kETC1,   // render.device.v2's kETC1Rgb
	kETC1A4, // render.device.v2's kETC1A4
	kRGBA4   // render.device.v2's kRGBA4Unorm: a 16-bit word per texel (D42)
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
	// A render target (RGBA8, one level, sides powers of two from 8 to 512):
	// SetTarget draws into it, later draws sample what it holds. Replaces
	// any previous image.
	bool CreateTarget( int width, int height );
	void Release();
	bool Valid() const { return m_texture != 0; }
	bool IsTarget() const { return m_target; }
	// Whether a draw may sample it: a target only once something was drawn
	// into it (its contents are undefined before) and while it is not the
	// target being drawn.
	bool Sampleable() const;
	// Repeat on both axes, else clamped (the port's sampler has one mode).
	void SetWrap( bool wrapS, bool wrapT );
	int Width() const { return m_width; }
	int Height() const { return m_height; }
	std::size_t Bytes() const { return m_bytes; }
	std::uint32_t Group(); // the material bind group drawing it (renderer use)
	std::uint32_t Id() const { return m_texture; } // the device's TextureId value

private:
	friend struct TargetAccess;
	std::uint32_t m_texture; // TextureId
	std::uint32_t m_group;   // BindGroupId, made on first draw
	bool m_target = false;
	// A target's colour usage (render::device::ResourceUsage): the renderer
	// keeps it current across recordings.
	std::uint8_t m_usage = 0;
	std::uint8_t m_wrap; // bit 0 repeats u, bit 1 repeats v
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
	std::uint32_t targetSwitches = 0;    // SetTarget changes of the drawn target
	std::uint32_t feedbackRefusals = 0;  // draws that sampled the target they draw into
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

// The target the frame draws into: a render target (Texture::CreateTarget),
// or null for the screen. Ends the current pass; the next pass loads what
// the target holds. Clears, viewports, draws and core sections follow it.
// The frame ends on the screen (EndFrame switches back).
void SetTarget( Texture *target );

void Clear( bool color, bool depth, std::uint32_t rgba );
// Target-space viewport (D3D sense: origin top-left of the target).
void SetViewport( int x, int y, int width, int height );

// Transient memory for one draw of this frame.
void *AllocTransient( Memory kind, std::size_t bytes, std::size_t align = 16 );
// Memory a mesh keeps. inPlace: its one copy is a buffer of the device the
// CPU writes in place, which legacy draws bind and the render core reads
// where it is (DeviceBufferOf, CoreMeshStreams); else CPU memory copied to
// the device when a legacy draw first reads it (and never, when the core
// draws the mesh from its own data). In-place memory falls back to the
// copied kind when the device has none left.
void *AllocLinear( Memory kind, std::size_t bytes, bool inPlace = false );
void FreeLinear( void *ptr );
// Before rewriting memory a recorded draw may read: submits the frame so far.
void PrepareWrite( const void *ptr );
void FlushLinear( const void *ptr, std::size_t bytes );
// The device buffer and byte offset of in-place memory (AllocLinear), for a
// draw the current recording makes: the memory then counts as read by it
// (PrepareWrite submits before rewriting it). False for other memory.
bool DeviceBufferOf( const void *ptr, render::device::BufferId &buffer, std::uint64_t &offset );

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
// Submits the frame's recording so far and continues it (the next recording
// loads what this one drew): what the device holds for the recorded draws is
// released behind it (a budget on per-frame geometry).
void FlushRecording();
void EndCoreSection();

// Writes the last presented frame as a binary PPM (400x240, RGB).
bool CaptureTopScreen( const char *path );

const Stats &FrameStats();

} // namespace pica

#endif // PICA_RENDERER_H
