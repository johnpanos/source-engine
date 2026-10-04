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

// A specialization constant (clause D20): the 32-bit value (the bits of a bool,
// int, uint or float) of a stage's constant with this id. An id the stage does
// not declare is ignored; a stage takes each id at most once.
struct SpecializationConstant
{
	ShaderStage stage = ShaderStage::kFragment;
	std::uint32_t id = 0;
	std::uint32_t value = 0;
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

enum class BlendMode : std::uint8_t
{
	kOpaque,
	kAlpha,         // src * a + dst * (1 - a)
	kPremultiplied, // src + dst * (1 - a)
	kAdditive,      // src + dst
	// D21: src + dst * a in color, the destination's alpha kept: the source
	// carries a transmittance in alpha (a participating medium's, RFC 0016).
	// It is applied as written, so a small transmittance keeps its relative
	// precision where 1 - a would lose it to the target's format.
	kTransmittance
};

// The color components a pipeline writes to an attachment (clause D17): a bit
// per channel. The rest keep what the attachment held.
inline constexpr std::uint8_t kColorWriteRed = 1;
inline constexpr std::uint8_t kColorWriteGreen = 2;
inline constexpr std::uint8_t kColorWriteBlue = 4;
inline constexpr std::uint8_t kColorWriteAlpha = 8;
inline constexpr std::uint8_t kColorWriteAll = 15;

struct RasterState
{
	CullMode cull = CullMode::kBack;
	bool frontCounterClockwise = true;
	bool alphaToCoverage = false; // fragment alpha controls multisample coverage
	float depthBiasConstant = 0.0f; // depth-buffer units, as in Vulkan and glPolygonOffset
	float depthBiasSlope = 0.0f;
};

enum class StencilOp : std::uint8_t
{
	kKeep,
	kZero,
	kReplace,
	kIncrementClamp,
	kDecrementClamp,
	kInvert,
	kIncrementWrap,
	kDecrementWrap
};

// Both faces use the same state. The reference is compared against the stored
// value after readMask; writeMask applies to the selected operation's result.
struct StencilState
{
	bool enabled = false;
	CompareOp compare = CompareOp::kAlways;
	StencilOp fail = StencilOp::kKeep;
	StencilOp depthFail = StencilOp::kKeep;
	StencilOp pass = StencilOp::kKeep;
	std::uint8_t reference = 0;
	std::uint8_t readMask = 255;
	std::uint8_t writeMask = 255;
	auto operator<=>( const StencilState & ) const = default;
};

struct DepthStencilState
{
	bool depthTest = false;
	bool depthWrite = false;
	CompareOp compare = CompareOp::kLessEqual;
	StencilState stencil = {};
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
	// One kColorWrite* mask per color format, or empty for every channel (D17).
	std::span<const std::uint8_t> colorWriteMasks;
	Format depthFormat = Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::string_view debugName;
	// The stages' specialization constants (D20).
	std::span<const SpecializationConstant> constants;
};

} // namespace render::device

#endif // RENDER_DEVICE_PIPELINE_H
