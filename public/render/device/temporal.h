//========= Copyright Valve Corporation, All rights reserved. ============//
// RFC 0019: provider-neutral temporal reconstruction recording contract.
#ifndef RENDER_DEVICE_TEMPORAL_H
#define RENDER_DEVICE_TEMPORAL_H
#include "render/device/device.h"
#include <string>
namespace render::device
{
struct TemporalExtent
{
	std::uint32_t width = 0, height = 0;
	friend bool operator==( TemporalExtent, TemporalExtent ) = default;
};
struct TemporalImages
{
	TextureId color, depth, motion, output;
};
struct TemporalDispatch
{
	TemporalImages images;
	TemporalExtent render, output;
	float jitterX = 0, jitterY = 0;
	float preExposure = 1;
	float deltaMilliseconds = 16.666667f;
	bool reset = true;
};
// One instance owns one view's history. It borrows its device. Record is on
// the device sequence, outside rendering; inputs are sampled, output storage
// write. The caller declares these accesses in the graph. A failed Record
// invalidates the encoder. Failed submission invalidates provider history.
// The provider retains all private resources through actual GPU completion.
class ITemporalUpscaler
{
public:
	virtual ~ITemporalUpscaler() = default;
	virtual DeviceResult<void> Record( CommandEncoder &encoder, const TemporalDispatch &frame ) = 0;
	virtual std::string Description() const = 0;
};
}
#endif
