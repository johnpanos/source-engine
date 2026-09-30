//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.indirect (RFC 0016 K12, "Runtime GI on the core"): the
//			renderer's compute service for the engine's indirect-light
//			producers (render/gpu_compute.h, render.gpu-compute.v1) on the
//			render core's device port. RFC 0011 owns the producers, their
//			contract and their math; this pass runs their GPU work: the SDF
//			producer's probe update ("sdf-probe-trace",
//			render/pass/indirect/sdf_probe_trace.comp). Ray-query programs and
//			scenes are not offered (the port has no acceleration structures
//			yet; the backend's service keeps the ray-query producer).
//
//			Buffers are CPU copies until the render sequence's Flush:
//			- an Upload buffer: Map returns the copy; the bytes the producer
//			  wrote reach the device buffer before the next dispatch that
//			  reads it (every Map marks the copy written);
//			- a Readback buffer: device-local storage the dispatches write,
//			  copied to a readback buffer in the same submission. Its first
//			  contents (what the producer wrote through Map before any
//			  dispatch bound it) are uploaded; after that the device owns it,
//			  and Map returns the copy read back when the submission that
//			  last wrote it completed, before CompletedSerial() reaches it.
//
//			Serials: a dispatch queued now runs in the next Flush's
//			submission, whose serial QueueDispatch returns; serials rise by
//			one per Flush with work and start at 1. CompletedSerial() rises
//			once a Flush observes that submission's completion token.
//
//			Threads: producers call it from one thread (the engine's main
//			thread); Flush and ReleaseDevice run on the render sequence. The
//			state between them is locked; nothing waits for the GPU.
//
//=============================================================================//

#ifndef RENDER_PASS_INDIRECT_PORT_COMPUTE_H
#define RENDER_PASS_INDIRECT_PORT_COMPUTE_H

#include "render/device/device.h"
#include "render/gpu_compute.h"

#include <cstdint>
#include <memory>
#include <span>

namespace render::pass::indirect
{

// The program names the service builds.
inline constexpr const char *kSdfProbeTraceProgram = "sdf-probe-trace";

class PortCompute final : public gpu_compute::IGpuCompute
{
public:
	// `spirv` replaces the SDF program's module (a suite's seeded variant).
	static std::unique_ptr<PortCompute> Create( std::span<const std::uint32_t> spirv = {} );
	~PortCompute() override;
	PortCompute( const PortCompute & ) = delete;
	PortCompute &operator=( const PortCompute & ) = delete;

	// gpu_compute::IGpuCompute (the producers' thread).
	gpu_compute::Caps Capabilities() const override;
	std::uint32_t CreateBuffer( std::size_t bytes, gpu_compute::BufferUse use ) override;
	void *Map( std::uint32_t buffer ) override;
	std::uint32_t CreateProgram( const char *name, const gpu_compute::Binding *bindings,
	    std::uint32_t count, std::uint32_t pushBytes ) override;
	std::uint32_t CreateGeometry(
	    const float *, std::uint32_t, const std::uint32_t *, std::uint32_t ) override
	{
		return 0;
	}
	std::uint32_t CreateScene( const gpu_compute::SceneInstance *, std::uint32_t ) override
	{
		return 0;
	}
	std::uint64_t QueueDispatch( std::uint32_t program, const std::uint32_t *buffers,
	    std::uint32_t count, const void *push, std::uint32_t pushBytes, std::uint32_t groupsX,
	    std::uint32_t groupsY, std::uint32_t groupsZ ) override;
	void WrittenRanges(
	    std::uint32_t buffer, const gpu_compute::ByteRange *ranges, std::uint32_t count ) override;
	std::uint64_t CompletedSerial() const override;
	void Retire( std::uint32_t resource, std::uint64_t afterSerial ) override;

	// Render sequence: submits the queued dispatches (with the uploads they
	// read and the read-backs they write) in a submission of their own on
	// `device`'s graphics queue, ahead of what the caller submits next, and
	// reads back and publishes what completed. False when a device call
	// failed (the dispatches are dropped and the next Flush starts over).
	bool Flush( device::IRenderDevice2 &device );
	// Render sequence: the device goes (after an idle wait); its objects are
	// released and the service starts over on the next device.
	void ReleaseDevice( device::IRenderDevice2 &device );

private:
	PortCompute();
	struct State;
	std::unique_ptr<State> m_State;
};

} // namespace render::pass::indirect

#endif // RENDER_PASS_INDIRECT_PORT_COMPUTE_H
