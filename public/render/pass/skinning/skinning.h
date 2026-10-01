//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.skinning (RFC 0016 K6): compute skinning with flex.
//
//			The records below are the std430 layouts render/pass/skinning/
//			skin.comp reads and writes. The math is the legacy one, so the
//			CPU oracle (SkinReference) is today's emit skinning in the native
//			Vulkan backend, which follows common_vs_fxc.h:
//
//			- flex first: each vertex's deltas add weight * delta to the
//			  position, weight * normal delta to the normal and to the
//			  tangent S, and weight * wrinkle to the wrinkle, with
//			  weight = mix( w[0], w[1], side ) of the delta's flex (a stereo
//			  flex's two weights, split by the vertex's side as studiorender
//			  does; a mono flex has w[0] == w[1]), mixed with the delayed
//			  flex's stereo weight by delta.delay (zero preserves captures
//			  made before delayed weights were represented);
//			- then three bones: weights w0, w1 and 1 - w0 - w1, indices in
//			  the low three bytes of `bones`, each bone a pose-to-world 3x4
//			  matrix (matrix3x4_t, row-major). An index past the palette
//			  reads bone 0. Normal and tangent go through the rotation part
//			  and stay unnormalized; the tangent keeps its w sign.
//
//			SkinningKernel creates the pipeline on a device with the compute
//			capability; AddSkinningPass declares the dispatch as a graph pass
//			whose accesses the graph compiler turns into transitions.
//
//=============================================================================//

#ifndef RENDER_PASS_SKINNING_SKINNING_H
#define RENDER_PASS_SKINNING_SKINNING_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/graph/graph_builder.h"

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace render::pass::skinning
{

struct SkinVertex // 64 bytes
{
	float position[3] = {};
	float weight0 = 1.0f;
	float normal[3] = {};
	float weight1 = 0.0f;
	float tangent[4] = {};   // xyz tangent S, w the bitangent sign
	std::uint32_t bones = 0; // bone indices in bytes 0, 1 and 2
	std::uint32_t reserved[3] = {};
	[[nodiscard]] std::array<float, 3> Weights() const
	{
		return { weight0, weight1, 1.0f - ( weight0 + weight1 ) };
	}
};

struct FlexDelta // 48 bytes
{
	float position[3] = {};
	std::uint32_t flex = 0; // index into the flex weights
	float normal[3] = {};
	float side = 0.0f; // 0 takes the flex's first weight, 1 its second
	float wrinkle = 0.0f;
	// Reuses zeroed reserved bytes from the original layout: old captures
	// have delay 0 and retain their original current-weight behavior.
	std::uint32_t delayedFlex = 0; // index into the same flex-weight array
	float delay = 0.0f; // fraction of the delayed stereo weights, 0..1
	float reserved = 0.0f;
};

struct FlexWeights // 8 bytes
{
	float weight[2] = {};
};

struct BoneMatrix // 48 bytes: pose to world, row-major 3x4
{
	float rows[3][4] = {};
};

struct SkinnedVertex // 48 bytes
{
	float position[3] = {};
	float wrinkle = 0.0f;
	float normal[4] = {};  // w 0
	float tangent[4] = {}; // w the input's sign
};

static_assert( sizeof( SkinVertex ) == 64 && sizeof( FlexDelta ) == 48 &&
               sizeof( FlexWeights ) == 8 && sizeof( BoneMatrix ) == 48 &&
               sizeof( SkinnedVertex ) == 48 );

// One mesh's inputs. flexOffsets has vertices.size() + 1 entries: vertex v's
// deltas are flexDeltas[flexOffsets[v], flexOffsets[v + 1]). Empty
// flexOffsets means no flex.
struct SkinInputs
{
	std::span<const SkinVertex> vertices;
	std::span<const BoneMatrix> bones;
	std::span<const std::uint32_t> flexOffsets;
	std::span<const FlexDelta> flexDeltas;
	std::span<const FlexWeights> flexWeights;
};

// The CPU oracle. out.size() must equal inputs.vertices.size().
void SkinReference( const SkinInputs &inputs, std::span<SkinnedVertex> out );

// Largest per-component differences between two skinned meshes.
struct SkinError
{
	float position = 0.0f;
	float normal = 0.0f;
	float tangent = 0.0f;
	float wrinkle = 0.0f;
};
SkinError CompareSkinned( std::span<const SkinnedVertex> a, std::span<const SkinnedVertex> b );

// The dispatch's buffers. Each input is bound as a read-only storage buffer
// (usage kStorageRead) and the output as a written one (kStorageWrite).
struct SkinningBuffers
{
	device::BufferId vertices;
	device::BufferId bones;
	device::BufferId flexOffsets; // at least vertexCount + 1 entries (zeros without flex)
	device::BufferId flexDeltas;  // at least one record
	device::BufferId flexWeights; // at least one record
	device::BufferId output;
	std::uint32_t vertexCount = 0;
	std::uint32_t boneCount = 0;
	std::uint32_t deltaCount = 0; // bound ranges; at least 1
	std::uint32_t flexCount = 0;  // at least 1
};

enum class SkinningStatus : std::uint8_t
{
	kNoCompute = 1, // the device lacks Capability::kCompute
	kDevice         // the device refused the layout, pipeline or bind group
};

class SkinningKernel
{
public:
	// `code` is the kernel's SPIR-V; the default is skin.comp. The suite
	// passes its seeded defective variants here.
	static foundation::Expected<std::unique_ptr<SkinningKernel>, SkinningStatus> Create(
	    device::IRenderDevice2 &device, std::span<const std::uint32_t> code = {} );
	~SkinningKernel();
	SkinningKernel( const SkinningKernel & ) = delete;
	SkinningKernel &operator=( const SkinningKernel & ) = delete;

	// Records one dispatch. The buffers must already be in their usages.
	// The dispatch's bind group stays live until Collect.
	foundation::Expected<void, SkinningStatus> Record(
	    device::CommandEncoder &encoder, const SkinningBuffers &buffers );
	// Releases the bind groups of recorded dispatches behind `token`, the
	// submission that ran them. The kernel may be destroyed once every
	// recorded dispatch has been collected.
	void Collect( device::CompletionToken token );
	// Dispatches a graph pass could not record (a refused bind group). A
	// pass has no error channel, so its owner checks this after execution
	// and fails the frame when it rose.
	std::uint32_t RecordFailures() const { return m_RecordFailures; }

private:
	SkinningKernel( device::IRenderDevice2 &device );
	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_Layout;
	device::PipelineId m_Pipeline;
	std::vector<device::BindGroupId> m_Pending;
	device::CompletionToken m_LastToken;
	std::uint32_t m_RecordFailures = 0;
};

// Graph resources of one skinning dispatch.
struct SkinningPassResources
{
	graph::ResourceRef vertices;
	graph::ResourceRef bones;
	graph::ResourceRef flexOffsets;
	graph::ResourceRef flexDeltas;
	graph::ResourceRef flexWeights;
	graph::ResourceRef output;
	std::uint32_t vertexCount = 0;
	std::uint32_t boneCount = 0;
	std::uint32_t deltaCount = 1;
	std::uint32_t flexCount = 1;
};

// A compute pass reading the inputs and writing the output. The kernel must
// outlive the graph's execution; a dispatch it cannot record counts in
// RecordFailures.
void AddSkinningPass(
    graph::GraphBuilder &builder, SkinningKernel &kernel, const SkinningPassResources &resources );

} // namespace render::pass::skinning

#endif // RENDER_PASS_SKINNING_SKINNING_H
