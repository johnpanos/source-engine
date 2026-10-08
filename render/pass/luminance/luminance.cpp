//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.luminance (RFC 0016 K8 post cohort); see luminance.h.
//
//=============================================================================//

#include "render/pass/luminance/luminance.h"

#include "render/shaderlib/core_artifacts.h"
#include "render/shaderlib/pipeline_recipe.h"

#include <map>
#include <mutex>
#include <set>
#include <vector>

namespace render::pass::luminance
{

namespace
{

using namespace render::device;

constexpr const char *kCountKernel = "render/pass/luminance/luminance_count.comp";
constexpr std::uint32_t kGroup = 16; // the kernel's local size

struct Constants
{
	std::int32_t rect[4] = {}; // x0, y0, width, height
	float range[4] = {};       // minimum, maximum, scale
};
static_assert( sizeof( Constants ) == 32 );

// A recorded query's GPU objects until its count is read.
struct Pending
{
	std::uint32_t serial = 0;
	BufferId counter;
	BufferId readback;
	BindGroupId group;
	bool submitted = false;
	CompletionToken token;
};

// Queries queued and not recorded, or results not yet taken, are bounded:
// a stream that never records (no backend slots) cannot grow them.
constexpr std::size_t kMaxKept = 1024;

} // namespace

struct LuminanceCounter::State
{
	// Main thread and render sequence, guarded by lock.
	mutable std::mutex lock;
	std::uint32_t nextSerial = 1;
	std::map<std::uint32_t, Query> queued;
	std::set<std::uint32_t> recorded; // a capture records the same slot again
	std::map<std::uint32_t, std::int64_t> results;
	LuminanceStats stats;
	std::span<const std::uint32_t> countModule;

	// Render sequence only.
	IRenderDevice2 *device = nullptr;
	bool made = false;
	std::string madeFailure;
	BindGroupLayoutId layout;
	PipelineId pipeline;
	SamplerId pointSampler;
	std::vector<Pending> pending;

	void Fail( std::uint32_t serial, const std::string &why )
	{
		std::lock_guard<std::mutex> guard( lock );
		results[serial] = -1;
		++stats.failed;
		stats.lastFailure = why;
	}

	std::optional<std::string> Make()
	{
		if ( made )
			return madeFailure.empty() ? std::nullopt : std::optional<std::string>( madeFailure );
		made = true;
		if ( !device->Facts().capabilities.Has( Capability::kCompute ) )
			return madeFailure = "the device has no compute (the luminance count)";
		const ShaderStageSet compute{ ShaderStage::kCompute };
		const BindingDesc bindings[] = { { 0, BindingKind::kSampledTexture, 1, compute },
		    { 1, BindingKind::kStorageBuffer, 1, compute },
		    { 2, BindingKind::kSampler, 1, compute } };
		auto layoutMade = device->CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
		if ( !layoutMade )
			return madeFailure = "the luminance bind group layout was refused";
		layout = layoutMade.Value();
		shaderlib::ArtifactOverlay artifacts( shaderlib::CoreArtifacts() );
		if ( !countModule.empty() &&
		     !artifacts.ReplaceSpirv( kCountKernel, countModule, device->Facts().artifactFormat ) )
			return madeFailure = "the seeded luminance kernel does not replace the core one";
		shaderlib::PipelineRecipe recipe =
		    shaderlib::CoreRecipe( { kCountKernel }, PipelineKind::kCompute );
		recipe.layouts = { {}, {}, {}, layout };
		recipe.debugName = "render.pass.luminance";
		auto resolved = shaderlib::Resolve( recipe, artifacts, device->Facts().artifactFormat );
		if ( !resolved )
			return madeFailure = "the luminance kernel has no artifact for the device";
		PipelineDesc desc = resolved.Value().Desc();
		desc.drawConstantBytes = sizeof( Constants );
		auto made = device->CreatePipeline( desc );
		if ( !made )
			return madeFailure = "the luminance pipeline was refused";
		pipeline = made.Value();
		SamplerDesc point;
		point.minFilter = point.magFilter = point.mipFilter = Filter::kNearest;
		point.address = AddressMode::kClampToEdge;
		auto sampler = device->CreateSampler( point );
		if ( !sampler )
			return madeFailure = "the luminance sampler was refused";
		pointSampler = sampler.Value();
		return std::nullopt;
	}

	void ReleasePending( const Pending &p, CompletionToken after )
	{
		for ( ResourceId resource :
		    { ResourceId( p.group ), ResourceId( p.counter ), ResourceId( p.readback ) } )
		{
			if ( resource.value != 0 )
				(void)device->Release( resource, after );
		}
	}

	void ReleaseAll( CompletionToken after )
	{
		if ( !device )
			return;
		for ( const Pending &p : pending )
		{
			ReleasePending( p, after );
			std::lock_guard<std::mutex> guard( lock );
			results[p.serial] = -1;
			++stats.failed;
			stats.lastFailure = "the device went before the count was read";
		}
		pending.clear();
		for ( ResourceId resource :
		    { ResourceId( pipeline ), ResourceId( layout ), ResourceId( pointSampler ) } )
		{
			if ( resource.value != 0 )
				(void)device->Release( resource, after );
		}
		pipeline = PipelineId();
		layout = BindGroupLayoutId();
		pointSampler = SamplerId();
		made = false;
		madeFailure.clear();
		device = nullptr;
	}
};

LuminanceCounter::LuminanceCounter( std::span<const std::uint32_t> countModule )
    : m_State( std::make_unique<State>() )
{
	m_State->countModule = countModule;
}

// Without ReleaseDevice the device may be gone: the handles are dropped.
LuminanceCounter::~LuminanceCounter() = default;

std::uint32_t LuminanceCounter::Queue( const Query &query )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	if ( query.texture == 0 || query.x0 < 0 || query.y0 < 0 || query.x1 < query.x0 ||
	     query.y1 < query.y0 || !( query.scale > 0.0f ) )
	{
		++s.stats.refused;
		s.stats.lastFailure = "a malformed luminance query";
		return 0;
	}
	const std::uint32_t serial = s.nextSerial;
	s.nextSerial = ( s.nextSerial + 1 ) & kLuminanceSerialMask;
	if ( s.nextSerial == 0 )
		s.nextSerial = 1;
	s.queued[serial] = query;
	++s.stats.queued;
	while ( s.queued.size() > kMaxKept )
	{
		s.results[s.queued.begin()->first] = -1;
		++s.stats.failed;
		s.stats.lastFailure = "a luminance query's slot never recorded";
		s.queued.erase( s.queued.begin() );
	}
	while ( s.results.size() > kMaxKept )
		s.results.erase( s.results.begin() );
	return kLuminanceTag | serial;
}

std::optional<std::int64_t> LuminanceCounter::Result( std::uint32_t tag )
{
	State &s = *m_State;
	std::lock_guard<std::mutex> guard( s.lock );
	const auto found = s.results.find( tag & kLuminanceSerialMask );
	if ( found == s.results.end() )
		return std::nullopt;
	const std::int64_t count = found->second;
	s.results.erase( found );
	return count;
}

LuminanceStats LuminanceCounter::Stats() const
{
	std::lock_guard<std::mutex> guard( m_State->lock );
	return m_State->stats;
}

void LuminanceCounter::Record(
    std::uint32_t tag, CommandEncoder &encoder, const LuminanceTarget &target )
{
	State &s = *m_State;
	if ( !IsLuminanceTag( tag ) )
		return;
	const std::uint32_t serial = tag & kLuminanceSerialMask;
	Query query;
	{
		std::lock_guard<std::mutex> guard( s.lock );
		const auto found = s.queued.find( serial );
		if ( found == s.queued.end() )
		{
			// A capture's second recording, or a query already given up.
			if ( !s.recorded.count( serial ) )
			{
				++s.stats.failed;
				s.stats.lastFailure = "a luminance slot names no queued query";
			}
			return;
		}
		query = found->second;
		s.queued.erase( found );
		s.recorded.insert( serial );
		while ( s.recorded.size() > kMaxKept )
			s.recorded.erase( s.recorded.begin() );
	}
	if ( !target.device || !target.textures )
		return s.Fail( serial, "a luminance slot's target is incomplete" );
	if ( s.device && s.device != target.device )
		s.ReleaseAll( CompletionToken() );
	s.device = target.device;
	if ( std::optional<std::string> why = s.Make() )
		return s.Fail( serial, *why );
	const TextureId texture = target.textures->Import( query.texture );
	if ( !texture.IsValid() )
		return s.Fail( serial, "the luminance query's texture " + std::to_string( query.texture ) +
		                           " does not import" );

	Pending p;
	p.serial = serial;
	BufferDesc counter;
	counter.size = 4;
	counter.usages = {
	    ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource };
	counter.debugName = "render.pass.luminance count";
	BufferDesc readback;
	readback.size = 4;
	readback.usages = { ResourceUsage::kCopyDestination };
	readback.memory = MemoryKind::kReadback;
	readback.debugName = "render.pass.luminance readback";
	auto counterMade = s.device->CreateBuffer( counter );
	auto readbackMade = s.device->CreateBuffer( readback );
	if ( counterMade )
		p.counter = counterMade.Value();
	if ( readbackMade )
		p.readback = readbackMade.Value();
	if ( counterMade && readbackMade )
	{
		const BindGroupEntry entries[] = { { 0, {}, 0, 0, texture, {} },
		    { 1, p.counter, 0, 0, {}, {} }, { 2, {}, 0, 0, {}, s.pointSampler } };
		auto group = s.device->CreateBindGroup( { s.layout, entries } );
		if ( group )
			p.group = group.Value();
	}
	if ( !p.group.IsValid() )
	{
		s.ReleasePending( p, target.submitted );
		return s.Fail( serial, "the luminance count's buffers were refused" );
	}

	Constants constants;
	constants.rect[0] = query.x0;
	constants.rect[1] = query.y0;
	constants.rect[2] = query.x1 - query.x0 + 1;
	constants.rect[3] = query.y1 - query.y0 + 1;
	constants.range[0] = query.minimum;
	constants.range[1] = query.maximum;
	constants.range[2] = query.scale;
	const std::uint32_t zero = 0;
	encoder.BeginLabel( "luminance count" );
	encoder.TransitionBuffer(
	    p.counter, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( p.counter, 0, std::as_bytes( std::span( &zero, 1 ) ) );
	encoder.TransitionBuffer(
	    p.counter, ResourceUsage::kCopyDestination, ResourceUsage::kStorageWrite );
	encoder.SetPipeline( s.pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, p.group );
	encoder.SetDrawConstants( 0, std::as_bytes( std::span( &constants, 1 ) ) );
	encoder.Dispatch( ( std::uint32_t( constants.rect[2] ) + kGroup - 1 ) / kGroup,
	    ( std::uint32_t( constants.rect[3] ) + kGroup - 1 ) / kGroup );
	encoder.TransitionBuffer( p.counter, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	encoder.TransitionBuffer(
	    p.readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	BufferCopy copy;
	copy.size = 4;
	encoder.CopyBuffer( p.counter, p.readback, copy );
	encoder.EndLabel();
	s.pending.push_back( p );
	std::lock_guard<std::mutex> guard( s.lock );
	++s.stats.recorded;
}

void LuminanceCounter::FrameSubmitted( IRenderDevice2 &device, CompletionToken token )
{
	State &s = *m_State;
	if ( s.device != &device )
		return;
	for ( Pending &p : s.pending )
	{
		if ( !p.submitted )
		{
			p.submitted = true;
			p.token = token;
		}
	}
	std::erase_if( s.pending,
	    [&]( const Pending &p )
	    {
		    if ( !p.submitted || !device.IsComplete( p.token ) )
			    return false;
		    std::uint32_t count = 0;
		    const bool read = bool( device.ReadBuffer(
		        p.readback, 0, std::as_writable_bytes( std::span( &count, 1 ) ) ) );
		    s.ReleasePending( p, p.token );
		    std::lock_guard<std::mutex> guard( s.lock );
		    s.results[p.serial] = read ? std::int64_t( count ) : -1;
		    if ( read )
			    ++s.stats.resolved;
		    else
		    {
			    ++s.stats.failed;
			    s.stats.lastFailure = "the luminance count's read back failed";
		    }
		    return true;
	    } );
}

void LuminanceCounter::ReleaseDevice( IRenderDevice2 &device )
{
	State &s = *m_State;
	if ( s.device != &device )
		return;
	s.ReleaseAll( CompletionToken() );
}

} // namespace render::pass::luminance
