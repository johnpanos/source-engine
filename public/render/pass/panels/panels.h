//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.panels (RFC 0016 layer 6 feature; RFC 0010's
//			in-world panel capability): a panel placed in the world (a
//			vgui_screen: Portal 2's chamber sign, a monitor's text) drawn by
//			the core as a first-class emissive surface.
//
//			A panel is a rectangle in the world whose image is a draw list
//			(render.world-panel.v1, render/world_panel.h, which defines the
//			image, its resolution and what it emits and casts), as the UI
//			toolkit painted it for one frame. The pass rasterizes the list
//			into the panel's image at the resolution the caller chose for the
//			panel's footprint on screen (world_panel::ChooseResolution) as
//			two images: the emission (the emissive quads under the coatings'
//			transmittance) and the face's albedo (the coatings), each with a
//			mip chain built in linear light. It draws the rectangle as a
//			PBRMetalRough material (the surface program's pbr point, through
//			the one resolver): the albedo image as its base, a dielectric
//			MRAO, the emission image at the panel's emission scale (the
//			contract's emission, which its area lights share), lit by the
//			scene's ambient cube at the panel (Panel::ambientCube).
//
//			One frame, one image. A panel is submitted at most once per host
//			frame (Submit refuses a second list for the same frame), and
//			every view of that frame draws the image rasterized from that one
//			list, at the first slot of the frame that draws the panel. A view
//			of a frame whose panel was never submitted fails by name; nothing
//			draws a previous frame's image.
//
//			Textures of a list are keys the slot's IPanelTextures resolves
//			(the material system's texture handles in the product), sampled
//			as they are stored (gamma values, as the legacy 2D path samples
//			them), with the provider's sampler.
//
//			Threads: Submit, QueueView, Stats on the main thread; Record and
//			ReleaseDevice on the render sequence. The queue between them is
//			locked.
//
//=============================================================================//

#ifndef RENDER_PASS_PANELS_PANELS_H
#define RENDER_PASS_PANELS_PANELS_H

#include "render/device/device.h"
#include "render/frame/debug_controls.h"
#include "render/material/program_resolver.h"
#include "render/world_panel.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace render::pass::panels
{

// The panel's image, placement and resolution are render.world-panel.v1's
// (render/world_panel.h, one definition for the core, the engine, the client
// and the UI surface).
using world_panel::Placement;
using world_panel::Quad;
using world_panel::Resolution;

struct Panel
{
	std::uint64_t id = 0; // the caller's, stable across frames (the image is kept per id)
	Placement placement = {};
	float unitsWide = 0.0f;
	float unitsTall = 0.0f;
	std::vector<Quad> quads;    // in paint order
	std::vector<int> textures;  // keys IPanelTextures resolves
	Resolution resolution = {}; // the list was painted for it (world_panel::ChooseResolution)
	float emissionScale = 1.0f; // scene radiance per decoded image value
	// The scene's light at the panel, which its coatings' albedo reflects:
	// Source's ambient cube (+x, -x, +y, -y, +z, -z, linear), as a model at
	// the panel is lit (the surface's indirect diffuse where no probe volume
	// covers it). Black: the coatings show no reflected light.
	float ambientCube[6][3] = {};
};

// The backend's textures a slot's lists name (the product: the material
// system's handles, render/legacy/core_passes.h ICoreTextures).
class IPanelTextures
{
public:
	// Invalid when the key names no resident texture.
	virtual device::TextureId Import( int key ) = 0;
	virtual device::SamplerDesc Sampler( int key ) = 0;

protected:
	~IPanelTextures() = default;
};

struct PanelTarget
{
	// The device the slot records on; the pass's objects live on it.
	device::IRenderDevice2 *device = nullptr;
	// The color target in kColorAttachment (its sRGB view, or with
	// terms.encodeOutput its unorm view), and depth in kDepthWrite.
	device::TextureId color;
	device::Format colorFormat = device::Format::kUnknown;
	device::TextureId depth;
	device::Format depthFormat = device::Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t samples = 1;
	IPanelTextures *textures = nullptr;
	// No earlier than every submission so far, and the serial of the frame
	// being recorded (0 unknown: retired objects wait for ReleaseDevice).
	device::CompletionToken submitted;
	std::uint64_t frame = 0;
	// The frame's terms at the slot (output scale, fog, eye, area lights):
	// the same terms the world's surfaces read.
	material::FrameTerms terms;
};

struct PanelView
{
	std::uint64_t hostFrame = 0;       // the frame whose submissions the view draws
	std::vector<std::uint64_t> panels; // their ids
	float toClip[16] = {};             // world to clip, as the port draws it
	device::Viewport viewport;
	frame::DebugControls debug; // the frame's debug controls (RFC 0014)
};

struct PanelStats
{
	std::uint64_t submitted = 0;  // lists accepted
	std::uint64_t refused = 0;    // second lists for a panel's frame, refused
	std::uint64_t rasterized = 0; // images rasterized (one per panel and frame drawn)
	std::uint64_t viewsQueued = 0;
	std::uint64_t viewsDrawn = 0;
	std::uint64_t viewsFailed = 0; // claimed panels not drawn: never legacy's
	std::uint64_t panelsDrawn = 0;
	std::uint64_t textureBytes = 0; // the resident images, mips included
	Resolution lastResolution = {}; // of the last image rasterized
	std::string lastFailure;
};

// A panel tag: the forwarded bit and the panel bit, then the view's serial
// in the low 28 bits (render/legacy/core_passes.h reserves bits 30 and 29).
inline constexpr std::uint32_t kPanelTag = 0x90000000u;
inline constexpr std::uint32_t kPanelSerialMask = 0x0fffffffu;
inline bool IsPanelTag( std::uint32_t tag )
{
	return ( tag & ~kPanelSerialMask ) == kPanelTag;
}

class PanelPass
{
public:
	// mipsModule: a suite's seeded mip kernel (SPIR-V words); empty for the
	// core one.
	explicit PanelPass( std::span<const std::uint32_t> mipsModule = {} );
	~PanelPass();
	PanelPass( const PanelPass & ) = delete;
	PanelPass &operator=( const PanelPass & ) = delete;

	// Main thread. The panel's list for a host frame; false (and counted)
	// when the panel already has one for that frame, or the list or its
	// resolution is invalid (lastFailure says which).
	bool Submit( std::uint64_t hostFrame, Panel panel );
	// Whether the panel has a list for the frame.
	bool Submitted( std::uint64_t hostFrame, std::uint64_t id ) const;
	// The tag of the slot to mark for the view; 0 when it names no panel.
	std::uint32_t QueueView( PanelView view );
	// A panel no longer exists: its image is released behind the frames
	// that used it.
	void Remove( std::uint64_t id );
	PanelStats Stats() const;
	std::uint64_t Failures() const;

	// Render sequence: draws the view a slot's tag names (rasterizing each
	// of its panels' images first when this frame has not yet).
	void Record( std::uint32_t tag, device::CommandEncoder &encoder, const PanelTarget &target );
	// The device is about to go (after an idle wait): its objects are
	// released. A pass destroyed without it drops its handles.
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace render::pass::panels

#endif // RENDER_PASS_PANELS_PANELS_H
