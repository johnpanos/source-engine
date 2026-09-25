//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Backend-agnostic render capability contract (RFC 0001, render.contracts).
//			This is the public API a render backend provider implements: a null /
//			recording provider today, a Vulkan or D3D9 provider tomorrow. Portable
//			engine code (the material system) requests these behaviors; it never
//			asks whether the backend is OpenGL or receives a native window handle.
//
//			The header is deliberately self-contained: standard fixed-width types
//			only, no tier0/platform.h, no SDL/Vulkan/Win32 types. That absence is
//			part of the contract ("absence of native window and graphics API types
//			from public portable headers", RFC 0001) and is proven mechanically by
//			this header compiling under the linux-headless-core profile (-I public
//			only). The shared conformance suite in unittests/rendertest exercises
//			every obligation below against any provider that claims it, including a
//			deterministic null fake and deliberately broken providers.
//
//			Names and ownership representation here are one concrete realization of
//			RFC 0001's "render-device capability family"; the semantic obligations
//			(immutable capabilities, structured failure, child-resource lifetime,
//			presentation resize without device recreation, provider-owned GPU
//			completion tokens, deterministic device-loss states) are the normative
//			part and are what the conformance suite pins.
//
//			Presentation is deliberately absent from the device: a pair-specific
//			bridge (render_presentation.h) joins one window system to one render
//			provider and creates presentations from a device and a surface, so no
//			device ever interprets a window (RFC 0001 "Window and render interop").
//
//=============================================================================//

#ifndef RENDER_RENDER_BACKEND_H
#define RENDER_RENDER_BACKEND_H

#include <cstddef>
#include <cstdint>

namespace render
{

// String helpers local to the render contracts. Legacy material-system targets
// forbid <cstring> by defining strncpy away, so these headers cannot include it.
namespace contract_detail
{

inline bool EqualStrings( const char *a, const char *b, size_t limit )
{
	for ( size_t i = 0; i < limit; ++i )
	{
		if ( a[i] != b[i] )
			return false;
		if ( a[i] == '\0' )
			return true;
	}
	return true;
}

} // namespace contract_detail

// ---------------------------------------------------------------------------
// Backend identity
// ---------------------------------------------------------------------------

// Stable, human-readable identity of a backend provider. 'id' is a stable token
// such as "null", "vulkan", "d3d9", or "gl"; 'name' is descriptive; 'version'
// is the provider implementation version. Identity is a fact about the backend,
// not a signal portable code may branch on to select behavior.
struct RenderBackendId
{
	const char *id = nullptr;
	const char *name = nullptr;
	uint32_t version = 0;
};

// ---------------------------------------------------------------------------
// Semantic features (never backend identity)
// ---------------------------------------------------------------------------

// Semantic capability questions. Portable code asks whether sampled sRGB reads
// are supported, not whether the backend is OpenGL. Values are bit positions in
// a RenderFeatureSet; the synthetic kNeverSupported bit exists so conformance
// tests can request a feature no real adapter advertises.
enum class RenderFeature : uint32_t
{
	kSampledSrgb = 0,
	kDepthColorPairing = 1,
	kComputeShaders = 2,
	kMultiSample4x = 3,
	kRuntimeShaderCompile = 4,
	kOffscreenRender = 5,
	// RFC 0011 G5: storage images a compute pass writes (RGBA16F and R32F),
	// and ray queries (acceleration structures, VK_KHR_ray_query). Claimed only
	// when the provider's device has them enabled (render_profile.h
	// ValidateDeviceClaims).
	kStorageImages = 6,
	kRayQuery = 7,
	kNeverSupported = 31,
};

// An immutable value set of semantic features. Copyable; carries no pointer into
// provider memory, so a captured set stays valid independently of its source.
struct RenderFeatureSet
{
	uint32_t bits = 0;

	static uint32_t Bit( RenderFeature f ) { return uint32_t( 1 ) << static_cast<uint32_t>( f ); }

	void Add( RenderFeature f ) { bits |= Bit( f ); }
	bool Has( RenderFeature f ) const { return ( bits & Bit( f ) ) != 0; }

	// True when every feature in 'required' is present in this set.
	bool Contains( const RenderFeatureSet &required ) const
	{
		return ( bits & required.bits ) == required.bits;
	}

	bool operator==( const RenderFeatureSet &o ) const { return bits == o.bits; }
	bool operator!=( const RenderFeatureSet &o ) const { return bits != o.bits; }
};

// ---------------------------------------------------------------------------
// Adapter enumeration (stable value descriptions)
// ---------------------------------------------------------------------------

// A value description of an adapter. It is a pure value with fixed-size storage:
// it MUST remain valid after the enumeration object that produced it is gone,
// so no field may point into transient provider memory.
//
// vendorId, deviceId, driverVersion and driverApi are driver facts. Portable code
// must not branch on them; they exist so documented quirks (render_profile.h)
// can name the adapters and driver ranges they affect. driverApi names the API
// the provider ultimately drives ("d3d9", "vulkan", "opengl", "none"), which can
// differ from the provider's own identity when it translates.
struct RenderAdapterInfo
{
	char id[64] = { 0 };
	char name[64] = { 0 };
	uint32_t vendorId = 0;
	uint32_t deviceId = 0;
	uint64_t driverVersion = 0;
	char driverApi[16] = { 0 };
	uint64_t deviceMemoryBytes = 0;
	bool isSoftware = false;
	RenderFeatureSet supportedFeatures;

	// Value equality over all fields. Adapter descriptions are pure values, so a
	// captured copy compares equal to a re-query (and stays valid independently).
	bool operator==( const RenderAdapterInfo &o ) const
	{
		return contract_detail::EqualStrings( id, o.id, sizeof( id ) ) &&
		       contract_detail::EqualStrings( name, o.name, sizeof( name ) ) &&
		       vendorId == o.vendorId && deviceId == o.deviceId &&
		       driverVersion == o.driverVersion &&
		       contract_detail::EqualStrings( driverApi, o.driverApi, sizeof( driverApi ) ) &&
		       deviceMemoryBytes == o.deviceMemoryBytes && isSoftware == o.isSoftware &&
		       supportedFeatures == o.supportedFeatures;
	}
	bool operator!=( const RenderAdapterInfo &o ) const { return !( *this == o ); }
};

// ---------------------------------------------------------------------------
// Device capabilities, requests, and structured failure
// ---------------------------------------------------------------------------

// Immutable hardware/driver facts reported by a created device. These MUST NOT
// change after construction. This is distinct from feature profile (application
// policy) and from quirks (documented workarounds).
struct RenderDeviceCaps
{
	RenderFeatureSet features;
	uint32_t maxTextureDimension = 0;
	uint32_t maxColorTargets = 0;
	uint32_t maxSampleCount = 0;

	bool operator==( const RenderDeviceCaps &o ) const
	{
		return features == o.features && maxTextureDimension == o.maxTextureDimension &&
			maxColorTargets == o.maxColorTargets && maxSampleCount == o.maxSampleCount;
	}
};

// A device-creation request. Required features must be satisfied or creation
// fails with a structured error naming the missing feature. Optional features
// are honored when available and silently absent otherwise.
struct RenderDeviceRequest
{
	int adapterIndex = 0;
	RenderFeatureSet requiredFeatures;
	RenderFeatureSet optionalFeatures;
	bool enableValidation = false;
};

// Why an object could not be created. Structured so callers diagnose failures
// without parsing log text.
enum class RenderCreateStatus : uint32_t
{
	kOk = 0,
	kInvalidAdapter,
	kUnsupportedRequiredFeature,
	kSurfaceIncompatible,
	kTooManyPresentations,
	kNotAdvertised,
	// Presentation-bridge composition failures (render_presentation.h).
	kUnsupportedPair,   // no bridge joins the requested window system and render backend
	kForeignObject,     // the device or surface was not created by the bridge's providers
	kSurfaceBusy,       // the surface already has a live presentation
	kSurfaceLost,       // the surface's window is destroyed
	kDeviceUnavailable, // the device is lost/fatal and cannot present now
};

struct RenderCreateError
{
	RenderCreateStatus status = RenderCreateStatus::kOk;
	// Valid only when status == kUnsupportedRequiredFeature.
	RenderFeature missingFeature = RenderFeature::kNeverSupported;
	char message[128] = { 0 };
};

// ---------------------------------------------------------------------------
// Device lifecycle state
// ---------------------------------------------------------------------------

enum class RenderDeviceState : uint32_t
{
	kAvailable = 0,
	kPresentationUnavailable,
	kDeviceLost,
	kRecovering,
	kFatal,
};

// ---------------------------------------------------------------------------
// Resources
// ---------------------------------------------------------------------------

enum class RenderResourceType : uint32_t
{
	kBuffer = 0,
	kTexture,
	kShaderArtifact,
};

// Opaque device-resource handle. 0 is the invalid handle. A resource's storage
// is owned by the device and cannot outlive it.
using RenderResourceHandle = uint64_t;
static constexpr RenderResourceHandle kInvalidResource = 0;

// A provider-owned token reporting GPU completion of a submission. A resource
// referenced by outstanding work may be recycled only after its token reports
// complete -- never merely because a CPU job finished, an atomic fence flipped,
// or a fixed number of frames elapsed (RFC 0006). The token is owned by the
// device and remains valid until the device is destroyed.
class IRenderCompletionToken
{
public:
	virtual ~IRenderCompletionToken() = default;
	virtual bool IsComplete() const = 0;
};

// A recorded unit of GPU work. Records which resources a submission uses so the
// device can gate deferred destruction on the submission's completion.
class IRenderCommandContext
{
public:
	virtual ~IRenderCommandContext() = default;
	virtual void RecordUse( RenderResourceHandle handle ) = 0;
};

// ---------------------------------------------------------------------------
// Render device
// ---------------------------------------------------------------------------

class IRenderDevice
{
public:
	virtual ~IRenderDevice() = default;

	// Immutable capabilities. Repeated calls return equal values.
	virtual const RenderDeviceCaps &GetCapabilities() const = 0;
	virtual RenderDeviceState GetState() const = 0;

	// -- Resources (child lifetime bound to this device) --
	virtual RenderResourceHandle CreateResource( RenderResourceType type ) = 0;
	virtual bool IsResourceLive( RenderResourceHandle handle ) const = 0;
	virtual size_t GetLiveResourceCount() const = 0;

	// Schedules destruction of a resource once 'token' reports complete. The
	// resource MUST stay live until then.
	virtual void DestroyResourceWhenComplete( RenderResourceHandle handle,
		IRenderCompletionToken &token ) = 0;

	// Applies any deferred destructions whose completion tokens are now complete.
	virtual void CollectCompletedDestructions() = 0;

	// -- Command submission and completion --
	virtual IRenderCommandContext *CreateCommandContext() = 0;

	// Submits recorded work in program order. Returns a device-owned completion
	// token. Submissions complete in submission order.
	virtual IRenderCompletionToken *Submit( IRenderCommandContext &context ) = 0;

	// Observes and applies GPU-side completion progress (e.g. polling fences).
	// Returns the number of submissions newly completed. For an offscreen/null
	// provider this deterministically completes the oldest outstanding submission.
	virtual uint32_t PollCompletion() = 0;

	// Monotonic id of the most recently completed submission (0 before any).
	virtual uint64_t LastCompletedSubmission() const = 0;

	// -- Device loss (only where advertised) --
	// Transitions the device to kDeviceLost. Returns false if the provider cannot
	// simulate loss.
	virtual bool SimulateDeviceLoss() = 0;

	// Attempts recovery from kDeviceLost. On success returns true and the device
	// returns to kAvailable (via kRecovering). A provider without recovery returns
	// false and transitions to kFatal.
	virtual bool RecoverDevice() = 0;
};

// ---------------------------------------------------------------------------
// Provider capability advertisement ("claims")
// ---------------------------------------------------------------------------

// What a provider claims to support. The conformance suite runs each optional
// section only when the provider advertises it -- a null/recording provider
// certifies only the command and lifetime behavior it claims, never image
// fidelity.
struct RenderProviderCaps
{
	bool supportsOffscreenDevice = false;
	bool supportsDeviceLossRecovery = false;
	bool supportsRuntimeShaderCompile = false;
};

// ---------------------------------------------------------------------------
// Backend provider
// ---------------------------------------------------------------------------

class IRenderBackendProvider
{
public:
	virtual ~IRenderBackendProvider() = default;

	virtual RenderBackendId GetBackendId() const = 0;
	virtual RenderProviderCaps GetProviderCaps() const = 0;

	// -- Adapter enumeration --
	virtual int GetAdapterCount() const = 0;

	// Fills 'out' with a stable value description. Returns false for an invalid
	// index, leaving 'out' unchanged.
	virtual bool GetAdapterInfo( int index, RenderAdapterInfo *out ) const = 0;

	// -- Device creation --
	// Returns a device or nullptr. On failure 'error' (when non-null) is populated
	// with structured information. Required features are validated before a device
	// is returned.
	virtual IRenderDevice *CreateDevice( const RenderDeviceRequest &request,
		RenderCreateError *error ) = 0;
	virtual void DestroyDevice( IRenderDevice *device ) = 0;

	// Number of devices this provider still owns. Used to prove complete
	// destruction before a backend module would be unloaded.
	virtual size_t GetLiveDeviceCount() const = 0;

	// True when 'device' is a live device this provider created. Presentation
	// bridges use it to reject a device from another provider structurally.
	virtual bool OwnsDevice( const IRenderDevice &device ) const = 0;
};

} // namespace render

#endif // RENDER_RENDER_BACKEND_H
