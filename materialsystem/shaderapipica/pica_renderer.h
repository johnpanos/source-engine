//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The 3DS shader API's frame on render.device.pica, the render core's
// PICA200 adapter (RFC 0026); nothing here reaches citro3d. The render core
// draws everything: this borrows the core's device (BindDevice) and owns the
// frame (a 400x240 colour and depth target in screen orientation, render
// targets, clears, the core sections the core's passes record into), the
// shader API's textures and the in-place device buffers behind its model
// meshes (CoreMeshStreams).
//
// Conventions:
//  * clip space is the port's: D3D's (y up, depth 0 to 1, row 0 at the top);
//    the device's presenter turns the target onto the top screen;
//  * mesh vertex colours are D3DCOLOR bytes (B G R A);
//  * textures are uploaded in the port's raster layout (row 0 at the top).
//
// A frame is recorded into one encoder and submitted at EndFrame (or when a
// mesh's memory is about to be rewritten while a recorded draw reads it:
// PrepareWrite).
//
//=============================================================================//

#ifndef PICA_RENDERER_H
#define PICA_RENDERER_H

#include "pica_texture.h"
#include "render/device/completion.h"
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
	kRGBA4,  // render.device.v2's kRGBA4Unorm: a 16-bit word per texel (D42)
	kRGBA8Srgb, // kRGBA8 decoded from sRGB when sampled (off the 3DS: D3D9's
	            // per-sampler sRGB read, which the core asks for per import)
	kRGBA16,    // RGBA16161616 texels (8 bytes) stored as render.device.v2's
	            // kRGBA16Float, which WebGPU and Vulkan both sample (off the
	            // 3DS: integer-HDR lightmap pages, the same values as
	            // shaderapivulkan's 16-bit unorm pages)
	kRGBA16F    // half floats (8 bytes a texel) as they are, kRGBA16Float (off
	            // the 3DS: HDR images, IMAGE_FORMAT_RGBA16161616F)
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
	// A cube map (RGBA8, one level, size x size a face): faces[i] is face i
	// in the port's order (+X, -X, +Y, -Y, +Z, -Z). Replaces any previous image.
	// srgb: an sRGB image, read as linear values (an env map off the 3DS).
	bool UploadCube( int size, const std::uint8_t *const *faces, bool srgb = false, bool half = false );
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
	std::uint64_t Id() const { return m_texture; } // the device's TextureId value
	// The colour usage the renderer keeps current across recordings
	// (render::device::ResourceUsage), for work recorded in a core section
	// (core_copies.cpp): read before transitioning, set to where it is left.
	std::uint8_t Usage() const { return m_usage; }
	void SetUsage( std::uint8_t usage ) { m_usage = usage; }

private:
	friend struct TargetAccess;
	std::uint64_t m_texture; // TextureId (the device's tagged 64-bit value)
	bool m_target = false;
	// A target's colour usage (render::device::ResourceUsage): the renderer
	// keeps it current across recordings.
	std::uint8_t m_usage = 0;
	std::uint8_t m_wrap; // bit 0 repeats u, bit 1 repeats v
	int m_width;
	int m_height;
	std::size_t m_bytes;
	// What Upload made (off the 3DS a new image of the same shape refills it,
	// so the id the core holds stays valid).
	std::uint8_t m_format = 0;
	int m_levels = 0;
};

struct Stats
{
	std::uint32_t submits = 0;
	std::uint32_t submitFailures = 0; // frames the device refused (not presented)
	std::uint32_t targetSwitches = 0;    // SetTarget changes of the drawn target
	std::size_t textureBytes = 0;
	std::size_t meshBytes = 0; // linear memory of the meshes (AllocLinear)
};

// The depth buffers' format: the PICA200's D24S8; elsewhere D32 float with
// an 8-bit stencil (portals and stencil clears need one, as the native
// backend's depth has; WebGPU's depth24plus is no copyable 24-bit unorm).
#if defined( PLATFORM_3DS )
constexpr render::device::Format kDepthFormat = render::device::Format::kD24UnormS8;
#else
constexpr render::device::Format kDepthFormat = render::device::Format::kD32FloatS8;
#endif

#if defined( PLATFORM_3DS )
// Screen: the top screen, 400x240.
constexpr int kScreenWidth = 400;
constexpr int kScreenHeight = 240;
#else
// Screen: the window's back buffer (RFC 0029: the browser's canvas), set
// before Init by SetScreenSize.
inline int kScreenWidth = 1280;
inline int kScreenHeight = 720;
void SetScreenSize( int width, int height );
// The engine's video mode changed: the screen's targets are made again at
// the new size between frames (false while a frame is open or on failure).
bool ResizeScreen( int width, int height );

// Shows the frame's colour target (RGBA8, width x height) on the screen, after
// the frame's work on the same queue; the composition root binds the one its
// device's presentation needs (the WebGPU adapter's canvas). Without one the
// frame is drawn and not shown (headless).
using Presenter = bool ( * )( void *context, render::device::IRenderDevice2 &device,
    std::uint64_t color, std::uint32_t width, std::uint32_t height );
void BindPresenter( Presenter presenter, void *context );
#endif

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
// The frame's last submission (EndFrame's): its completion token, and
// whether the device took it.
render::device::CompletionToken LastSubmission( bool *submitted );
bool InFrame();

// The target the frame draws into: a render target (Texture::CreateTarget),
// or null for the screen. Ends the current pass; the next pass loads what
// the target holds. Clears, viewports, draws and core sections follow it.
// The frame ends on the screen (EndFrame switches back).
void SetTarget( Texture *target );

void Clear( bool color, bool depth, std::uint32_t rgba );
// Target-space viewport (D3D sense: origin top-left of the target).
void SetViewport( int x, int y, int width, int height );

// Memory a mesh keeps. inPlace: its one copy is a buffer of the render
// core's device the CPU writes in place, which the core reads where it is
// (DeviceBufferOf, CoreMeshStreams); else CPU memory (the world meshes, which
// the core draws from the map's own data). In-place memory falls back to CPU
// memory when the device has none left.
void *AllocLinear( Memory kind, std::size_t bytes, bool inPlace = false );
void FreeLinear( void *ptr );
// Before rewriting memory a recorded draw may read: submits the frame so far.
void PrepareWrite( const void *ptr );
void FlushLinear( const void *ptr, std::size_t bytes );
// The device buffer and byte offset of in-place memory (AllocLinear), for a
// draw the current recording makes: the memory then counts as read by it
// (PrepareWrite submits before rewriting it). False for other memory.
bool DeviceBufferOf( const void *ptr, render::device::BufferId &buffer, std::uint64_t &offset );

// A section of the frame for a render core pass (RFC 0026 P3, the core's
// passes at slots of this stream): ends this renderer's pass and returns the
// frame's encoder with the target in its home usages (color
// kColorAttachment, depth kDepthWrite, the frame's top-left 400x240); null
// outside a frame. EndCoreSection reopens the pass, loading what the section
// drew, with the current viewport.
struct CoreSectionTarget
{
	render::device::IRenderDevice2 *device = nullptr;
	std::uint64_t color = 0; // TextureId values
	std::uint64_t depth = 0;
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

// Reads a region of what the frame has drawn into the current target (the
// screen, or the render target bound), as RGBA8 rows top first: the engine's
// screenshots and ReadPixels. Submits the recording so far and waits for it.
bool ReadCurrentTarget( int x, int y, int width, int height, std::uint8_t *rgba );
// Writes the last presented frame as a binary PPM (400x240, RGB).
bool CaptureTopScreen( const char *path );

const Stats &FrameStats();

} // namespace pica

#endif // PICA_RENDERER_H
