//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 errors (RFC 0016). Each failure names the status
//			and the operation that failed; nativeCode carries the adapter's
//			own result code (a VkResult, a GL error) when it has one.
//
//=============================================================================//

#ifndef RENDER_DEVICE_ERRORS_H
#define RENDER_DEVICE_ERRORS_H

#include "foundation/error.h"

#include <cstdint>

namespace render::device
{

enum class DeviceStatus : std::uint32_t
{
	kInvalidDescription = 1, // a description the port's rules reject
	kUnsupported,            // valid, but a capability or format this device lacks
	kOutOfMemory,
	kDeviceLost,
	kStaleEpoch,        // a token or handle from before a device loss
	kTooManyBindGroups, // more than kMaxBindGroups layouts
	kLayoutMismatch,    // an artifact's reflected bindings are not in the layouts
	kInvalidHandle,     // unknown, released or foreign resource
	kInvalidState,      // an encoder used after submission, rendering not ended
	kUnavailable,       // no adapter or driver could be opened
	kInternal
};

enum class DeviceOperation : std::uint32_t
{
	kCreateDevice = 1,
	kCreateBuffer,
	kCreateTexture,
	kCreateSampler,
	kCreateBindGroupLayout,
	kCreateBindGroup,
	kCreatePipeline,
	kBeginEncoder,
	kSubmit,
	kReadBuffer,
	kRelease
};

using DeviceError = foundation::Error<DeviceStatus, DeviceOperation>;

const char *DescribeStatus( DeviceStatus status );
const char *DescribeOperation( DeviceOperation operation );

} // namespace render::device

#endif // RENDER_DEVICE_ERRORS_H
