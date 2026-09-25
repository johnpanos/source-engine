//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.gpu-compute.v1 (RFC 0011 G5/G6): the renderer's compute
//          service for engine-side GPU producers. Host-visible storage
//          buffers, the renderer's built-in compute programs by name (the
//          renderer owns shader binaries), dispatches queued into the next
//          submission, and completion serials: a buffer the GPU writes may be
//          read once CompletedSerial() reaches the serial its dispatch ran in,
//          and a retired resource is freed only after the serial it names.
//          No call waits for the GPU. No native types.
//
//===========================================================================//

#ifndef RENDER_GPU_COMPUTE_H
#define RENDER_GPU_COMPUTE_H

#include <cstddef>
#include <cstdint>

namespace gpu_compute
{
static const char *const kGpuComputeInterface = "RenderGpuCompute001";

struct Caps
{
	bool compute = false;
	bool storageImages = false;
	bool rayQuery = false;
};

class IGpuCompute
{
public:
	virtual ~IGpuCompute() = default;
	[[nodiscard]] virtual Caps Capabilities() const = 0;
	// A host-visible, coherent storage buffer; 0 on failure.
	[[nodiscard]] virtual uint32_t CreateBuffer( size_t bytes ) = 0;
	[[nodiscard]] virtual void *Map( uint32_t buffer ) = 0;
	// A built-in program (e.g. "sdf-probe-trace") over `buffers` storage
	// buffers (set 0, bindings 0..n-1) and `pushBytes` of push constants.
	[[nodiscard]] virtual uint32_t CreateProgram(
	    const char *name, uint32_t buffers, uint32_t pushBytes ) = 0;
	// Queues a dispatch into the next submission and returns its serial (0 on
	// failure). Dispatches run in queue order; each one's writes are visible
	// to the next.
	[[nodiscard]] virtual uint64_t QueueDispatch( uint32_t program, const uint32_t *buffers,
	    uint32_t count, const void *push, uint32_t pushBytes, uint32_t groupsX, uint32_t groupsY,
	    uint32_t groupsZ ) = 0;
	[[nodiscard]] virtual uint64_t CompletedSerial() const = 0;
	// Frees `resource` once CompletedSerial() reaches `afterSerial`.
	virtual void Retire( uint32_t resource, uint64_t afterSerial ) = 0;
};

} // namespace gpu_compute

#endif // RENDER_GPU_COMPUTE_H
