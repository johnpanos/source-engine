//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 pipelines (RFC 0016). A pipeline is created from
//			shader artifacts, a bind-group layout per role, vertex input and
//			fixed-function state; nothing is mutable after creation. An adapter
//			whose API has no pipeline objects caches programs and applies state
//			differences itself, invisibly above the port.
//
//			Creation fails with kLayoutMismatch when an artifact's reflected
//			bindings are not in the layouts, kTooManyBindGroups past four
//			layouts, and kUnsupported for an artifact format the device does
//			not accept (render/device/validation.h holds the shared rules).
//
//=============================================================================//

#ifndef RENDER_DEVICE_PIPELINE_H
#define RENDER_DEVICE_PIPELINE_H

#include "render/device/bind_group.h"
#include "render/device/facts.h"
#include "render/device/resources.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace render::device
{

// A binding an artifact's reflection says it uses.
struct ReflectedBinding
{
	std::uint32_t group = 0; // the BindGroupRole index
	std::uint32_t binding = 0;
	BindingKind kind = BindingKind::kUniformBuffer;
};

// Draw constants (port clause D16): a small block of bytes a pipeline's
// stages read, set per draw or dispatch with CommandEncoder::SetDrawConstants.
// Vulkan maps it to push constants, OpenGL to a uniform block. At most this
// many bytes, a multiple of four.
inline constexpr std::uint32_t kMaxDrawConstantBytes = 128;

struct ShaderArtifactView
{
	ShaderStage stage = ShaderStage::kVertex;
	ArtifactFormat format = ArtifactFormat::kSpirv;
	std::span<const std::byte> code;
	std::string_view entryPoint = "main";
	std::span<const ReflectedBinding> bindings;
	std::uint32_t drawConstantBytes = 0; // reflected: the draw-constant bytes the stage reads
};

enum class VertexFormat : std::uint8_t
{
	kFloat2,
	kFloat3,
	kFloat4,
	kUnorm8x4
};

struct VertexAttribute
{
	std::uint32_t location = 0;
	VertexFormat format = VertexFormat::kFloat3;
	std::uint32_t offset = 0;
	std::uint32_t bufferSlot = 0;
};

struct VertexBufferLayout
{
	std::uint32_t stride = 0;
	bool perInstance = false;
};

struct VertexLayout
{
	std::span<const VertexAttribute> attributes;
	std::span<const VertexBufferLayout> buffers;
};

enum class PipelineKind : std::uint8_t
{
	kGraphics,
	kCompute
};

enum class PrimitiveTopology : std::uint8_t
{
	kTriangleList,
	kTriangleStrip,
	kLineList,
	kPointList
};

enum class CullMode : std::uint8_t
{
	kNone,
	kBack,
	kFront
};

enum class CompareOp : std::uint8_t
{
	kNever,
	kLess,
	kLessEqual,
	kEqual,
	kGreaterEqual,
	kGreater,
	kAlways
};

enum class BlendMode : std::uint8_t
{
	kOpaque,
	kAlpha,         // src * a + dst * (1 - a)
	kPremultiplied, // src + dst * (1 - a)
	kAdditive       // src + dst
};

struct RasterState
{
	CullMode cull = CullMode::kBack;
	bool frontCounterClockwise = true;
};

struct DepthStencilState
{
	bool depthTest = false;
	bool depthWrite = false;
	CompareOp compare = CompareOp::kLessEqual;
};

struct PipelineDesc
{
	PipelineKind kind = PipelineKind::kGraphics;
	std::span<const ShaderArtifactView> stages;
	// Index = BindGroupRole; an invalid id leaves that role unused.
	std::span<const BindGroupLayoutId> layouts;
	// The draw-constant block every stage may read (D16): 0 for none, at most
	// kMaxDrawConstantBytes, a multiple of four, and no smaller than any
	// stage's reflected drawConstantBytes.
	std::uint32_t drawConstantBytes = 0;
	VertexLayout vertex;
	PrimitiveTopology topology = PrimitiveTopology::kTriangleList;
	RasterState raster;
	DepthStencilState depthStencil;
	std::span<const Format> colorFormats;
	std::span<const BlendMode> blends; // one per color format, or empty for opaque
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::string_view debugName;
};

} // namespace render::device

#endif // RENDER_DEVICE_PIPELINE_H
