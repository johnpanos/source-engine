//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_PASS_TEMPORAL_H
#define RENDER_PASS_TEMPORAL_H
#include "render/device/temporal.h"
#include "render/graph/graph_builder.h"
#include <optional>
namespace render::pass::temporal
{
// Producer-owned identity. chain identifies the full portal/mirror path,
// never just its depth. A zero scene or generator is invalid.
struct ViewKey
{
	std::uint64_t scene = 0, generator = 0, chain = 0;
	std::uint32_t eye = 0, destination = 0;
	friend bool operator==( ViewKey, ViewKey ) = default;
};
struct Frame
{
	ViewKey view;
	std::uint64_t sequence = 0;
	device::TemporalExtent render, output;
	float jitterX = 0, jitterY = 0, preExposure = 1, deltaMilliseconds = 0;
	bool discontinuity = false;
	bool motionComplete = false;
};
struct Inputs
{
	graph::ResourceRef color, depth, motion, output;
};
// One object per view. Metadata commits only after successful submission;
// abort discards the candidate. Skipped sequences reset, stale ones fail.
class History
{
public:
	device::DeviceResult<device::TemporalDispatch> Prepare(
	    const Frame &frame, std::uint32_t deviceEpoch );
	void Commit();
	void Abort();

private:
	std::optional<Frame> m_Previous, m_Pending;
	std::uint32_t m_Epoch = 0, m_PendingEpoch = 0;
	bool m_Invalid = true;
};
// Validates shapes/formats before adding a pass. Provider and history outlive
// graph execution. Caller Commit/Abort follows the executor's submission result.
device::DeviceResult<void> AddPass( graph::GraphBuilder &graph, History &history,
    device::ITemporalUpscaler &provider, const Frame &frame, std::uint32_t deviceEpoch,
    const Inputs &inputs );
}
#endif
