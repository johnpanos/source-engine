//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.luminance (RFC 0016 layer 6 feature, K8 post cohort):
//			the luminance counts behind the client's auto exposure, on the
//			core. A query counts the texels of a texture rectangle whose
//			scaled luminance lies in a range: the one definition of the
//			legacy luminance_compare_ps2x test the client's histogram
//			(viewpostprocess.cpp) ran through an occlusion query over
//			_rt_FullFrameFB:
//
//			  L = dot( texel.rgb * scale, ( 0.2125, 0.7154, 0.0721 ) )
//			  counted when minimum <= L <= maximum
//
//			with the texel read as stored (no sRGB decode) at integer
//			coordinates x0..x1, y0..y1 inclusive (the rectangle the legacy
//			screen-space draw covered one texel per pixel).
//
//			A query is queued on the main thread, in frame order (its slot
//			reads the texture as the frame's stream left it there), and its
//			count is read on the main thread once the frame that recorded it
//			has completed on the GPU, a frame or more later, as the occlusion
//			query's was. Nothing is read before its completion token allows.
//
//			Threads: Queue and Result on the main thread; Record,
//			FrameSubmitted and ReleaseDevice on the render sequence. The
//			state between them is locked.
//
//=============================================================================//

#ifndef RENDER_PASS_LUMINANCE_LUMINANCE_H
#define RENDER_PASS_LUMINANCE_LUMINANCE_H

#include "render/device/device.h"
#include "render/device/encoder.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>

namespace render::pass::luminance
{

struct Query
{
	int texture = 0; // a key ILuminanceTextures resolves (a material system handle)
	int x0 = 0, y0 = 0, x1 = -1, y1 = -1; // inclusive texel rectangle
	float minimum = 0.0f;
	float maximum = 0.0f;
	float scale = 1.0f;
};

// The legacy test, for oracles and the CPU: whether a texel counts.
inline bool Counts( const float rgb[3], float minimum, float maximum, float scale )
{
	const float l = ( rgb[0] * 0.2125f + rgb[1] * 0.7154f + rgb[2] * 0.0721f ) * scale;
	return l >= minimum && l <= maximum;
}

class ILuminanceTextures
{
public:
	// Invalid when the key names no resident texture.
	virtual device::TextureId Import( int key ) = 0;

protected:
	~ILuminanceTextures() = default;
};

struct LuminanceTarget
{
	device::IRenderDevice2 *device = nullptr;
	ILuminanceTextures *textures = nullptr;
	// No earlier than every submission made before this frame's.
	device::CompletionToken submitted;
	std::uint64_t frame = 0; // the frame being recorded (0 unknown)
};

struct LuminanceStats
{
	std::uint64_t queued = 0;
	std::uint64_t refused = 0;  // malformed queries
	std::uint64_t recorded = 0; // dispatches recorded
	std::uint64_t resolved = 0; // counts read back
	std::uint64_t failed = 0;   // queued queries that will never resolve
	std::string lastFailure;
};

// A luminance tag: the forwarded bit with high byte 0x8e, then the query's
// serial in the low 24 bits (clear of the world's, the panels' 0x90,
// temporal's 0x88 and post's 0x8c tags, and of bits 30 and 29).
inline constexpr std::uint32_t kLuminanceTag = 0x8e000000u;
inline constexpr std::uint32_t kLuminanceSerialMask = 0x00ffffffu;
inline bool IsLuminanceTag( std::uint32_t tag )
{
	return ( tag & ~kLuminanceSerialMask ) == kLuminanceTag;
}

class LuminanceCounter
{
public:
	// countModule: a suite's seeded kernel (SPIR-V words); empty for the core's.
	explicit LuminanceCounter( std::span<const std::uint32_t> countModule = {} );
	~LuminanceCounter();
	LuminanceCounter( const LuminanceCounter & ) = delete;
	LuminanceCounter &operator=( const LuminanceCounter & ) = delete;

	// Main thread: the tag of the slot that counts; 0 for a malformed query.
	std::uint32_t Queue( const Query &query );
	// Main thread: the count once read back; nullopt while pending; -1 when
	// the query failed (its slot never recorded, or its dispatch was refused).
	std::optional<std::int64_t> Result( std::uint32_t tag );
	LuminanceStats Stats() const;

	// Render sequence: the query's dispatch (a slot recorded again for a
	// capture records nothing more).
	void Record(
	    std::uint32_t tag, device::CommandEncoder &encoder, const LuminanceTarget &target );
	// Render sequence: the frame's submission completed by `token`; reads
	// back every query whose submission has completed.
	void FrameSubmitted( device::IRenderDevice2 &device, device::CompletionToken token );
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace render::pass::luminance

#endif // RENDER_PASS_LUMINANCE_LUMINANCE_H
