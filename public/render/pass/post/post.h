//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.post (RFC 0016 K8 "Post and screen effects"): the
//			engine's bloom, ported from the native backend's
//			screenspace_post.frag (Downsample_nohdr_ps2x, BlurFilter_ps2x).
//			From the linear scene it builds a quarter-size bloom image in the
//			display-referred (sRGB-encoded) values the legacy chain worked on:
//
//			1. downsample: each tap is the scene times the exposure, tone
//			   mapped at SDR (the clip) and sRGB encoded, then shaped by its
//			   luminance against the bloom tint and raised to the tint's
//			   exponent; a texel averages four bilinear taps, the 4x4 source
//			   block (Downsample_nohdr's offsets of 0.5 and 2.5 texels);
//			2. blur x: the 13-tap separable Gaussian (weights 0.2013, 0.2185,
//			   0.0821, 0.0461, 0.0262, 0.0162, 0.0102 at 0, 1.3366, 3.4295,
//			   5.4264, 7.4359, 9.4436 and 11.4401 texels);
//			3. blur y: the same taps scaled by the bloom amount. Legacy
//			   BlurFilterY takes its vertical step from the source's width
//			   (1 / width, not 1 / height); the port keeps that quirk.
//
//			Each step writes an 8-bit UNORM image, as the legacy _rt_SmallFB0
//			and _rt_SmallFB1 did, so values clip at 1 where they clipped.
//			render.pass.output adds the result to the encoded frame
//			(engine_post's fb + bloom, Portal 2's bloomadd).
//
//			Claims: the legacy chain's materials are consumed by name
//			(ClaimPostDraw): their draws describe the frame's bloom and the
//			pass draws it once. Anything else is refused by name.
//
//=============================================================================//

#ifndef RENDER_PASS_POST_POST_H
#define RENDER_PASS_POST_POST_H

#include "foundation/expected.h"
#include "render/device/device.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>
#include <vector>

namespace render::pass::post
{

// The 13-tap kernel, shared with the suites' CPU oracle.
inline constexpr float kBlurWeights[7] = {
    0.2013f, 0.2185f, 0.0821f, 0.0461f, 0.0262f, 0.0162f, 0.0102f };
inline constexpr float kBlurOffsets[7] = {
    0.0f, 1.3366f, 3.4295f, 5.4264f, 7.4359f, 9.4436f, 11.4401f };

enum class PostStatus : std::uint8_t
{
	kInvalidTarget, // the scene or its extent cannot be read
	kInvalidParams, // non-finite or negative parameters
	kDevice,        // the device refused an image, layout or pipeline
};

// A legacy draw's role in the bloom chain.
enum class PostRole : std::uint8_t
{
	kDownsample, // Downsample_nohdr
	kBlurX,      // BlurFilterX
	kBlurY,      // BlurFilterY: carries $bloomamount
	kAdd,        // Engine_Post (bloom add) or screenspace_general's bloomadd
};

// One legacy draw's material, as the mesh handoff passes it.
struct PostDrawVariable
{
	std::string_view key;
	std::string_view value;
};

struct PostClaim
{
	PostRole role = PostRole::kDownsample;
	float bloomAmount = 0.0f; // kBlurY: $bloomamount
	// kDownsample: r_bloomtint{r,g,b,exponent}, which the frontend passes as
	// "$bloomtint" "[r g b e]"; $bloomtintenable 0 gives 1/3 grey, exponent 1.
	float tint[4] = { 0.3f, 0.59f, 0.11f, 2.2f };
	bool bloomEnabled = true; // kAdd: Engine_Post's $bloomenable
};

// Claims a legacy draw by its shader and variables, or names why not.
foundation::Expected<PostClaim, std::string_view> ClaimPostDraw(
    std::string_view shader, std::span<const PostDrawVariable> variables );

struct BloomParams
{
	float exposure = 1.0f; // the frame's exposure, as render.pass.output applies it
	float tint[4] = { 0.3f, 0.59f, 0.11f, 2.2f }; // r_bloomtint{r,g,b,exponent}
	float bloomAmount = 1.0f;                     // BlurFilterY's $bloomamount
};

struct BloomSource
{
	device::TextureId scene; // linear float color
	device::ResourceUsage sceneUsage = device::ResourceUsage::kSampled;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

class BloomRenderer
{
public:
	static foundation::Expected<std::unique_ptr<BloomRenderer>, PostStatus> Create(
	    device::IRenderDevice2 &device );
	// Test endpoint: another fragment program with the same interface.
	static foundation::Expected<std::unique_ptr<BloomRenderer>, PostStatus> CreateWithFragment(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> fragmentSpirv );
	~BloomRenderer();

	BloomRenderer( const BloomRenderer & ) = delete;
	BloomRenderer &operator=( const BloomRenderer & ) = delete;

	// Records the three steps into `encoder`, outside rendering. The scene is
	// in sourceUsage before and after. Returns the bloom image (in kSampled),
	// valid until the next Record; its extent is the scene's / 4 (at least 1).
	foundation::Expected<device::TextureId, PostStatus> Record(
	    device::CommandEncoder &encoder, const BloomSource &source, const BloomParams &params );

	std::uint32_t Width() const { return m_Width; }
	std::uint32_t Height() const { return m_Height; }

	// Releases what executions up to `token` used once it completes.
	void Collect( device::CompletionToken token );
	std::uint32_t RecordFailures() const;

private:
	explicit BloomRenderer( device::IRenderDevice2 &device ) : m_Device( device ) {}
	bool EnsureImages( std::uint32_t width, std::uint32_t height );
	bool Step( device::CommandEncoder &encoder, std::uint32_t mode, device::TextureId source,
	    device::TextureId target, std::uint32_t sourceWidth, std::uint32_t sourceHeight,
	    const BloomParams &params );

	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_Layout;
	device::SamplerId m_Sampler; // bilinear, clamped
	device::PipelineId m_Pipeline;
	device::TextureId m_Images[2];
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
	mutable std::mutex m_PendingLock;
	std::vector<device::BindGroupId> m_Pending;
	std::vector<device::TextureId> m_Retired;
	std::uint32_t m_RecordFailures = 0;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::post

#endif // RENDER_PASS_POST_POST_H
