//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Present-mode selection for a swapchain (render.present-policy.v1).
//
//          Vsync waits for vertical blank (FIFO, which every Vulkan surface
//          supports). Without vsync the presenter does not wait, like D3D9's
//          D3DPRESENT_INTERVAL_IMMEDIATE: immediate when the surface offers it,
//          then mailbox (no tearing, still no wait), then FIFO as the only
//          guaranteed mode.
//
//===========================================================================//

#ifndef RENDER_PRESENT_POLICY_H
#define RENDER_PRESENT_POLICY_H

#include <cstdint>

namespace render
{
enum class PresentModeKind : uint8_t
{
	Fifo,
	Mailbox,
	Immediate,
};

struct PresentModeSupport
{
	bool fifo = true;
	bool mailbox = false;
	bool immediate = false;
};

[[nodiscard]] constexpr PresentModeKind SelectPresentMode(
    bool vsync, const PresentModeSupport &supported )
{
	if ( vsync )
		return PresentModeKind::Fifo;
	if ( supported.immediate )
		return PresentModeKind::Immediate;
	if ( supported.mailbox )
		return PresentModeKind::Mailbox;
	return PresentModeKind::Fifo;
}

[[nodiscard]] constexpr const char *PresentModeName( PresentModeKind mode )
{
	switch ( mode )
	{
	case PresentModeKind::Fifo:
		return "fifo";
	case PresentModeKind::Mailbox:
		return "mailbox";
	case PresentModeKind::Immediate:
		return "immediate";
	}
	return "unknown";
}
} // namespace render

#endif // RENDER_PRESENT_POLICY_H
