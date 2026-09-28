//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 completion tokens (RFC 0016, RFC 0006 GPU
//			completion). A token names one submission: its queue, a value that
//			increases by one per submission on that queue, and the device epoch.
//
//			- Submissions on one queue complete in order: when a token is
//			  complete, every earlier token of that queue and epoch is too.
//			- A device loss starts a new epoch. Tokens of an old epoch are
//			  complete (nothing of that epoch is still in flight), but work may
//			  no longer wait on them: Submit rejects them with kStaleEpoch.
//			- The default token (value 0) names no submission and is complete.
//
//			How an adapter implements tokens (timeline semaphores, fence sync
//			objects) is private to it.
//
//=============================================================================//

#ifndef RENDER_DEVICE_COMPLETION_H
#define RENDER_DEVICE_COMPLETION_H

#include <cstdint>
#include <span>

namespace render::device
{

enum class QueueKind : std::uint8_t
{
	kGraphics,
	kCompute,  // optional capability; the graph falls back to graphics
	kTransfer, // optional capability
	kCount
};

struct CompletionToken
{
	QueueKind queue = QueueKind::kGraphics;
	std::uint32_t epoch = 0;
	std::uint64_t value = 0;

	constexpr bool NamesSubmission() const { return value != 0; }

	friend constexpr bool operator==( const CompletionToken &, const CompletionToken & ) = default;
};

// Work that must complete before a submission starts, on any queue.
struct SubmitWaits
{
	std::span<const CompletionToken> tokens;
};

} // namespace render::device

#endif // RENDER_DEVICE_COMPLETION_H
