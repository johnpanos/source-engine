//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's check kernels (RFC 0016 K11); see lab_compute.h.
//
//=============================================================================//

#include "lab_compute.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace render::lab
{

using namespace render::device;

CheckKernel::~CheckKernel()
{
	(void)m_Device.WaitIdle();
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, {} );
	for ( SamplerId sampler : m_SamplerIds )
		(void)m_Device.Release( sampler, {} );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, {} );
}

std::optional<std::string> CheckKernel::Create( std::span<const std::uint32_t> module,
    std::uint32_t textures, std::uint32_t samplers, std::string_view name,
    std::span<const SamplerDesc> samplerDescs, std::uint32_t extraBuffers )
{
	if ( !m_Device.Facts().capabilities.Has( Capability::kCompute ) )
		return std::string( "the device has no compute" );
	if ( !samplerDescs.empty() && samplerDescs.size() != samplers )
		return std::string( "the sampler description count differs from the bindings" );
	m_Textures = textures;
	m_Samplers = samplers;
	m_Extra = extraBuffers;
	const ShaderStageSet compute{ ShaderStage::kCompute };
	std::vector<BindingDesc> bindings;
	std::vector<ReflectedBinding> reflected;
	const std::uint32_t role = std::uint32_t( BindGroupRole::kDraw );
	for ( std::uint32_t t = 0; t < textures; ++t )
	{
		bindings.push_back( { t, BindingKind::kSampledTexture, 1, compute } );
		reflected.push_back( { role, t, BindingKind::kSampledTexture } );
	}
	for ( std::uint32_t s = textures; s < textures + samplers; ++s )
	{
		bindings.push_back( { s, BindingKind::kSampler, 1, compute } );
		reflected.push_back( { role, s, BindingKind::kSampler } );
	}
	for ( std::uint32_t b = textures + samplers; b <= textures + samplers + 1 + extraBuffers; ++b )
	{
		bindings.push_back( { b, BindingKind::kStorageBuffer, 1, compute } );
		reflected.push_back( { role, b, BindingKind::kStorageBuffer } );
	}
	auto layout = m_Device.CreateBindGroupLayout( { BindGroupRole::kDraw, bindings } );
	if ( !layout )
		return std::string( "the kernel's layout was refused" );
	m_Layout = layout.Value();
	const ShaderArtifactView stage{ ShaderStage::kCompute, ArtifactFormat::kSpirv,
	    std::as_bytes( module ), "main", reflected, 0 };
	const BindGroupLayoutId layouts[] = { {}, {}, {}, m_Layout };
	PipelineDesc desc;
	desc.kind = PipelineKind::kCompute;
	desc.stages = { &stage, 1 };
	desc.layouts = layouts;
	desc.debugName = name;
	auto pipeline = m_Device.CreatePipeline( desc );
	if ( !pipeline )
		return std::string( "the check kernel was refused" );
	m_Pipeline = pipeline.Value();
	SamplerDesc samplerDesc;
	samplerDesc.address = AddressMode::kClampToEdge;
	for ( std::uint32_t i = 0; i < samplers; ++i )
	{
		auto sampler =
		    m_Device.CreateSampler( samplerDescs.empty() ? samplerDesc : samplerDescs[i] );
		if ( !sampler )
			return std::string( "the sampler was refused" );
		m_SamplerIds.push_back( sampler.Value() );
	}
	return std::nullopt;
}

std::optional<std::string> CheckKernel::Run( resources::TextureCache &cache,
    std::span<const TextureId> textures, std::uint32_t caseCount, std::span<const std::byte> cases,
    std::uint64_t resultBytes, std::vector<std::byte> &out, Comparison *comparison,
    std::span<const std::span<const std::byte>> extra )
{
	if ( extra.size() != m_Extra )
		return std::string( "the kernel takes " ) + std::to_string( m_Extra ) + " extra buffers";
	if ( textures.size() != m_Textures )
		return std::string( "the kernel takes " ) + std::to_string( m_Textures ) + " textures";
	if ( comparison &&
	     ( comparison->times.empty() || comparison->control.m_Textures != m_Textures ||
	         comparison->control.m_Samplers != m_Samplers ) )
		return std::string( "the kernel comparison has no samples or incompatible bindings" );
	if ( comparison && ( !m_Device.Facts().capabilities.Has( Capability::kTimestamps ) ||
	                       !( m_Device.Facts().timestampPeriodNs > 0.0 ) ) )
		return std::string( "the kernel benchmark requires GPU timestamps" );
	struct RunResources
	{
		IRenderDevice2 &device;
		std::vector<ResourceId> ids;
		CompletionToken token;
		~RunResources()
		{
			for ( ResourceId id : ids )
				(void)device.Release( id, token );
			device.Poll();
		}
	} resources{ m_Device, {}, {} };
	const std::uint64_t caseBytes = 16 + cases.size();
	auto buffer = [&]( std::uint64_t size, std::initializer_list<ResourceUsage> usages,
	                  MemoryKind memory ) -> BufferId
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = UsageSet( usages );
		desc.memory = memory;
		auto made = m_Device.CreateBuffer( desc );
		if ( made )
			resources.ids.push_back( made.Value() );
		return made ? made.Value() : BufferId();
	};
	const BufferId input =
	    buffer( caseBytes, { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead },
	        MemoryKind::kDeviceLocal );
	const BufferId results = buffer( resultBytes,
	    { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource }, MemoryKind::kDeviceLocal );
	const BufferId readback =
	    buffer( resultBytes, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
	if ( !input.IsValid() || !results.IsValid() || !readback.IsValid() )
		return std::string( "a buffer was refused" );
	BufferId times, controlResults, controlReadback;
	BindGroupId controlGroup;
	if ( comparison )
	{
		times = buffer( comparison->times.size() * 32, { ResourceUsage::kCopyDestination },
		    MemoryKind::kReadback );
		controlResults =
		    buffer( resultBytes, { ResourceUsage::kStorageWrite, ResourceUsage::kCopySource },
		        MemoryKind::kDeviceLocal );
		controlReadback =
		    buffer( resultBytes, { ResourceUsage::kCopyDestination }, MemoryKind::kReadback );
		if ( !times.IsValid() || !controlResults.IsValid() || !controlReadback.IsValid() )
			return std::string( "the benchmark timestamp buffer was refused" );
	}
	std::vector<BindGroupEntry> entries;
	for ( std::uint32_t t = 0; t < m_Textures; ++t )
		entries.push_back( { t, {}, 0, 0, textures[t], {} } );
	for ( std::uint32_t s = 0; s < m_Samplers; ++s )
		entries.push_back( { m_Textures + s, {}, 0, 0, {}, m_SamplerIds[s] } );
	const std::uint32_t buffers = m_Textures + m_Samplers;
	entries.push_back( { buffers, input, 0, caseBytes, {}, {} } );
	entries.push_back( { buffers + 1, results, 0, resultBytes, {}, {} } );
	const std::size_t resultsEntry = entries.size() - 1;
	std::vector<BufferId> extras;
	for ( std::size_t i = 0; i < extra.size(); ++i )
	{
		const std::uint64_t size = std::max<std::uint64_t>( extra[i].size(), 16 );
		const BufferId made =
		    buffer( size, { ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead },
		        MemoryKind::kDeviceLocal );
		if ( !made.IsValid() )
			return std::string( "an extra buffer was refused" );
		extras.push_back( made );
		entries.push_back( { buffers + 2 + std::uint32_t( i ), made, 0, size, {}, {} } );
	}
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
		return std::string( "the kernel's group was refused" );
	resources.ids.push_back( group.Value() );
	if ( comparison )
	{
		for ( std::uint32_t s = 0; s < m_Samplers; ++s )
			entries[m_Textures + s].sampler = comparison->control.m_SamplerIds[s];
		entries[resultsEntry].buffer = controlResults;
		auto made = m_Device.CreateBindGroup( { comparison->control.m_Layout, entries } );
		if ( !made )
			return std::string( "the comparison group was refused" );
		controlGroup = made.Value();
		resources.ids.push_back( controlGroup );
	}

	std::vector<std::byte> bytes( caseBytes );
	const std::uint32_t header[4] = { caseCount, 0, 0, 0 };
	std::memcpy( bytes.data(), header, sizeof( header ) );
	std::memcpy( bytes.data() + 16, cases.data(), cases.size() );

	auto encoded = m_Device.BeginEncoder( QueueKind::kGraphics );
	if ( !encoded )
		return std::string( "no encoder" );
	CommandEncoder &encoder = encoded.Value();
	cache.RecordUploads( encoder );
	encoder.TransitionBuffer( input, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.WriteBuffer( input, 0, bytes );
	encoder.TransitionBuffer( input, ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead );
	for ( std::size_t i = 0; i < extra.size(); ++i )
	{
		encoder.TransitionBuffer(
		    extras[i], ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		if ( !extra[i].empty() )
			encoder.WriteBuffer( extras[i], 0, extra[i] );
		encoder.TransitionBuffer(
		    extras[i], ResourceUsage::kCopyDestination, ResourceUsage::kStorageRead );
	}
	encoder.TransitionBuffer( results, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	if ( times.IsValid() )
	{
		encoder.TransitionBuffer(
		    times, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.TransitionBuffer(
		    controlResults, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
		const std::vector<std::byte> unset( comparison->times.size() * 32, std::byte{ 0xff } );
		encoder.WriteBuffer( times, 0, unset );
	}
	// Sustained work brings the GPU out of its idle clock before short samples.
	const std::size_t warmup = comparison ? 256 : 0;
	const std::size_t iterations = comparison ? comparison->times.size() + warmup : 1;
	for ( std::size_t i = 0; i < iterations; ++i )
	{
		for ( int order = 0; order < ( comparison ? 2 : 1 ); ++order )
		{
			const bool candidate = !comparison || ( ( i % 2 == 0 ) == ( order == 0 ) );
			const BufferId written = candidate ? results : controlResults;
			if ( i > 0 )
				encoder.TransitionBuffer(
				    written, ResourceUsage::kStorageWrite, ResourceUsage::kStorageWrite );
			encoder.SetPipeline( candidate ? m_Pipeline : comparison->control.m_Pipeline );
			encoder.SetBindGroup( BindGroupRole::kDraw, candidate ? group.Value() : controlGroup );
			const std::size_t offset =
			    comparison && i >= warmup ? ( i - warmup ) * 32 + ( candidate ? 0 : 16 ) : 0;
			if ( comparison && i >= warmup )
				encoder.WriteTimestamp( times, offset );
			encoder.Dispatch( ( caseCount + 63 ) / 64 );
			if ( comparison && i >= warmup )
				encoder.WriteTimestamp( times, offset + 8 );
		}
	}
	encoder.TransitionBuffer( results, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	encoder.TransitionBuffer(
	    readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.CopyBuffer( results, readback, { 0, 0, resultBytes } );
	if ( comparison )
	{
		encoder.TransitionBuffer(
		    controlResults, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
		encoder.TransitionBuffer(
		    controlReadback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
		encoder.CopyBuffer( controlResults, controlReadback, { 0, 0, resultBytes } );
	}
	auto token = m_Device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !token )
		return std::string( "the dispatch was refused at submission" );
	resources.token = token.Value();
	if ( !m_Device.WaitIdle() )
		return std::string( "the kernel did not complete" );
	cache.Retire( token.Value() );
	out.resize( resultBytes );
	const bool read = bool( m_Device.ReadBuffer( readback, 0, out ) );
	if ( !read )
		return std::string( "the results did not read back" );
	if ( times.IsValid() )
	{
		comparison->controlOutput.resize( resultBytes );
		if ( !m_Device.ReadBuffer( controlReadback, 0, comparison->controlOutput ) )
			return std::string( "the control results did not read back" );
		std::vector<std::uint64_t> ticks( comparison->times.size() * 4 );
		if ( !m_Device.ReadBuffer( times, 0, std::as_writable_bytes( std::span( ticks ) ) ) )
			return std::string( "the timestamps did not read back" );
		for ( std::size_t i = 0; i < comparison->times.size(); ++i )
		{
			for ( int variant = 0; variant < 2; ++variant )
			{
				const std::size_t begin = i * 4 + variant * 2;
				if ( ticks[begin] == ~std::uint64_t( 0 ) || ticks[begin + 1] <= ticks[begin] )
					return std::string( "a benchmark timestamp is missing or unordered" );
				const double ms = double( ticks[begin + 1] - ticks[begin] ) *
				                  m_Device.Facts().timestampPeriodNs / 1e6;
				if ( !std::isfinite( ms ) )
					return std::string( "a benchmark GPU duration is invalid" );
				( variant == 0 ? comparison->times[i].candidateMs
				               : comparison->times[i].controlMs ) = ms;
			}
		}
	}
	return std::nullopt;
}

} // namespace render::lab
