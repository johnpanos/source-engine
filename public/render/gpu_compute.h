//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.gpu-compute.v1 (RFC 0011 G5/G6): the renderer's compute
//          service for engine-side GPU producers. Host-visible storage
//          buffers, the renderer's built-in compute programs by name (the
//          renderer owns shader binaries), dispatches queued into the next
//          submission, and completion serials: a buffer the GPU writes may be
//          read once CompletedSerial() reaches the serial its dispatch ran in,
//          and a retired resource is freed only after the serial it names.
//          With ray query (Caps::rayQuery, G7): triangle geometry and scenes
//          of its instances for programs to trace against. No call waits for
//          the GPU. No native types.
//
//===========================================================================//

#ifndef RENDER_GPU_COMPUTE_H
#define RENDER_GPU_COMPUTE_H

#include <cstddef>
#include <cstdint>

namespace gpu_compute
{
static const char *const kGpuComputeInterface = "RenderGpuCompute001";

// Where a buffer lives: Upload (the CPU writes, the GPU reads: device-local
// where the device has host-visible local memory) or Readback (the GPU
// writes, the CPU reads: host-cached memory, so reads are not uncached).
enum class BufferUse
{
	Upload,
	Readback,
};

// A program's binding, in order (set 0, bindings 0..n-1).
enum class Binding
{
	Buffer, // a storage buffer
	Scene,  // a ray-query scene (Caps::rayQuery)
};

// One instance of a scene: geometry placed by a row-major 3 x 4 transform;
// programs read `customIndex` (24 bits) on a hit, and a ray's cull mask
// selects instances by `mask` (8 bits).
struct SceneInstance
{
	uint32_t geometry = 0;
	float transform[12] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 };
	uint32_t customIndex = 0;
	uint32_t mask = 0xFF;
};

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
	[[nodiscard]] virtual uint32_t CreateBuffer( size_t bytes, BufferUse use ) = 0;
	[[nodiscard]] virtual void *Map( uint32_t buffer ) = 0;
	// A built-in program (e.g. "sdf-probe-trace") over `bindings` and
	// `pushBytes` of push constants.
	[[nodiscard]] virtual uint32_t CreateProgram(
	    const char *name, const Binding *bindings, uint32_t count, uint32_t pushBytes ) = 0;
	// Opaque triangles (xyz positions, 32-bit indices), copied; 0 without ray
	// query or on failure. Built in queue order, ahead of later dispatches.
	[[nodiscard]] virtual uint32_t CreateGeometry( const float *positions, uint32_t vertexCount,
	    const uint32_t *indices, uint32_t indexCount ) = 0;
	// A scene of geometry instances, built in queue order; its geometry must
	// outlive it (retire the scene first, or behind the same serial).
	[[nodiscard]] virtual uint32_t CreateScene(
	    const SceneInstance *instances, uint32_t count ) = 0;
	// Queues a dispatch into the next submission and returns its serial (0 on
	// failure). Dispatches run in queue order; each one's writes are visible
	// to the next. `buffers` holds one resource per binding (a scene for a
	// Scene binding).
	[[nodiscard]] virtual uint64_t QueueDispatch( uint32_t program, const uint32_t *buffers,
	    uint32_t count, const void *push, uint32_t pushBytes, uint32_t groupsX, uint32_t groupsY,
	    uint32_t groupsZ ) = 0;
	[[nodiscard]] virtual uint64_t CompletedSerial() const = 0;
	// Frees `resource` once CompletedSerial() reaches `afterSerial`.
	virtual void Retire( uint32_t resource, uint64_t afterSerial ) = 0;
};

} // namespace gpu_compute

#endif // RENDER_GPU_COMPUTE_H
