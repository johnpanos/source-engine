//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 facts (RFC 0016). Facts are what an adapter can
//			do; they are immutable for the device's lifetime. Families and
//			features declare the capabilities they require, and the composition
//			root selects a declared fallback or fails by name. Nothing falls
//			back silently.
//
//			diagnosticBackend ("null", "vulkan", "gl", "pica") exists for logs and
//			evidence only. Portable code must not compare it (CAP011 rule 5):
//			behavior follows capabilities.
//
//=============================================================================//

#ifndef RENDER_DEVICE_FACTS_H
#define RENDER_DEVICE_FACTS_H

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string_view>

namespace render::device
{

enum class Capability : std::uint8_t
{
	kCompute,           // compute pipelines and dispatch
	kStorageBuffers,    // storage buffer and storage texture bindings
	kTransientAliasing, // transient resources may share memory
	kParallelRecording, // encoders record natively on several threads at once
	kAsyncCompute,      // a compute queue separate from graphics
	kAsyncTransfer,     // a transfer queue separate from graphics
	kRayQuery,
	kExternalImages,        // textures whose memory is exported (external_images.h, clause D18)
	kTextureCompressionBC,  // the kBC* formats (clause D19)
	kTimestamps,            // CommandEncoder::WriteTimestamp (clause D23)
	kMultiDrawIndirect,     // CommandEncoder::DrawIndexedIndirect (clause D30)
	kDrawIndirectCount,     // CommandEncoder::DrawIndexedIndirectCount (clause D31)
	kIndirectFirstInstance, // indirect records may carry a nonzero firstInstance (D30)
	kCubeArrays,            // kCube textures of more than six layers (clause D36)
	kFillModeLines,         // RasterState::fill kLines, triangle edges as lines (clause D38)
	// Colour targets of the float formats (kRG16Float, kRGBA16Float, kR32Float,
	// kRGBA32Float, kRG11B10Float) and float depth (kD32Float, kD32FloatS8),
	// as attachments and sampled (clause D39). A device without it refuses
	// those formats with kUnsupported (RFC 0026: the PICA200 has none).
	kFloatTargets,
	kTextureCompressionETC1, // the kETC1* formats (clause D40)
	kCount
};

const char *CapabilityName( Capability capability );

class CapabilitySet
{
public:
	constexpr CapabilitySet() = default;
	constexpr CapabilitySet( std::initializer_list<Capability> capabilities )
	{
		for ( Capability capability : capabilities )
			m_Bits |= Bit( capability );
	}

	static constexpr CapabilitySet All()
	{
		CapabilitySet set;
		set.m_Bits = ( 1u << static_cast<std::uint32_t>( Capability::kCount ) ) - 1u;
		return set;
	}

	constexpr bool Has( Capability capability ) const
	{
		return ( m_Bits & Bit( capability ) ) != 0;
	}
	constexpr CapabilitySet &Add( Capability capability )
	{
		m_Bits |= Bit( capability );
		return *this;
	}
	constexpr CapabilitySet &Remove( Capability capability )
	{
		m_Bits &= ~Bit( capability );
		return *this;
	}
	constexpr std::uint32_t Bits() const { return m_Bits; }

	friend constexpr bool operator==( CapabilitySet, CapabilitySet ) = default;

private:
	static constexpr std::uint32_t Bit( Capability capability )
	{
		return 1u << static_cast<std::uint32_t>( capability );
	}

	std::uint32_t m_Bits = 0;
};

// The first capability of required that have lacks, in enum order.
std::optional<Capability> FirstMissing( CapabilitySet have, CapabilitySet required );

enum class Format : std::uint8_t; // resources.h

// The capability a texture format needs (kTextureCompressionBC for kBC*,
// kTextureCompressionETC1 for kETC1*, kFloatTargets for the float formats),
// or nullopt for the formats every device has. Each adapter refuses a format
// whose capability it does not claim with kUnsupported.
std::optional<Capability> FormatCapability( Format format );

// The shader artifact format an adapter accepts (RFC 0016 "Shader artifacts").
enum class ArtifactFormat : std::uint8_t
{
	kSpirv,
	kGlsl450,
	kGlslEs310, // the GL adapter's ES dialect (RFC 0022)
	kHlsl,      // the Direct3D 12 adapter's HLSL, shader model 6.6 (RFC 0024)
	kMsl,       // the Metal adapter's Metal Shading Language 3.0 (RFC 0025)
	kPica       // PICA200: PVS1 vertex programs, PFP1 combiner programs (RFC 0026)
};

struct Limits
{
	std::uint32_t maxBindGroups = 4; // never more than kMaxBindGroups
	std::uint32_t maxTextureDimension2D = 0;
	std::uint32_t maxColorAttachments = 0;
	std::uint32_t maxVertexBuffers = 0;
	std::uint32_t uniformBufferAlignment = 0;
	std::uint32_t sampleCounts = 1; // bit n set: 2^n samples supported

	friend constexpr bool operator==( const Limits &, const Limits & ) = default;
};

struct DeviceFacts
{
	std::string_view diagnosticBackend; // logs and evidence only
	std::string_view adapterName;       // owned by the device
	CapabilitySet capabilities;
	Limits limits;
	ArtifactFormat artifactFormat = ArtifactFormat::kSpirv;
	// Nanoseconds per timestamp tick (clause D23); 0 without kTimestamps.
	double timestampPeriodNs = 0.0;

	friend constexpr bool operator==( const DeviceFacts &, const DeviceFacts & ) = default;
};

} // namespace render::device

#endif // RENDER_DEVICE_FACTS_H
