//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's check kernels (RFC 0016 K11); see lab_compute.h.
//
//=============================================================================//

#include "lab_compute.h"

#include <cstring>

namespace render::lab
{

using namespace render::device;

CheckKernel::~CheckKernel()
{
	(void)m_Device.WaitIdle();
	if ( m_Pipeline.IsValid() )
		(void)m_Device.Release( m_Pipeline, {} );
	if ( m_Sampler.IsValid() )
		(void)m_Device.Release( m_Sampler, {} );
	if ( m_Layout.IsValid() )
		(void)m_Device.Release( m_Layout, {} );
}

std::optional<std::string> CheckKernel::Create( std::span<const std::uint32_t> module,
    std::uint32_t textures, std::uint32_t samplers, std::string_view name )
{
	if ( !m_Device.Facts().capabilities.Has( Capability::kCompute ) )
		return std::string( "the device has no compute" );
	m_Textures = textures;
	m_Samplers = samplers;
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
	for ( std::uint32_t b = textures + samplers; b <= textures + samplers + 1; ++b )
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
	auto sampler = m_Device.CreateSampler( samplerDesc );
	if ( !sampler )
		return std::string( "the sampler was refused" );
	m_Sampler = sampler.Value();
	return std::nullopt;
}

std::optional<std::string> CheckKernel::Run( resources::TextureCache &cache,
    std::span<const TextureId> textures, std::uint32_t caseCount, std::span<const std::byte> cases,
    std::uint64_t resultBytes, std::vector<std::byte> &out )
{
	if ( textures.size() != m_Textures )
		return std::string( "the kernel takes " ) + std::to_string( m_Textures ) + " textures";
	const std::uint64_t caseBytes = 16 + cases.size();
	auto buffer = [&]( std::uint64_t size, std::initializer_list<ResourceUsage> usages,
	                  MemoryKind memory ) -> BufferId
	{
		BufferDesc desc;
		desc.size = size;
		desc.usages = UsageSet( usages );
		desc.memory = memory;
		auto made = m_Device.CreateBuffer( desc );
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
	std::vector<BindGroupEntry> entries;
	for ( std::uint32_t t = 0; t < m_Textures; ++t )
		entries.push_back( { t, {}, 0, 0, textures[t], {} } );
	for ( std::uint32_t s = 0; s < m_Samplers; ++s )
		entries.push_back( { m_Textures + s, {}, 0, 0, {}, m_Sampler } );
	const std::uint32_t buffers = m_Textures + m_Samplers;
	entries.push_back( { buffers, input, 0, caseBytes, {}, {} } );
	entries.push_back( { buffers + 1, results, 0, resultBytes, {}, {} } );
	auto group = m_Device.CreateBindGroup( { m_Layout, entries } );
	if ( !group )
		return std::string( "the kernel's group was refused" );

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
	encoder.TransitionBuffer( results, ResourceUsage::kUndefined, ResourceUsage::kStorageWrite );
	encoder.SetPipeline( m_Pipeline );
	encoder.SetBindGroup( BindGroupRole::kDraw, group.Value() );
	encoder.Dispatch( ( caseCount + 63 ) / 64 );
	encoder.TransitionBuffer( results, ResourceUsage::kStorageWrite, ResourceUsage::kCopySource );
	encoder.TransitionBuffer(
	    readback, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
	encoder.CopyBuffer( results, readback, { 0, 0, resultBytes } );
	auto token = m_Device.Submit( QueueKind::kGraphics, { &encoder, 1 }, {} );
	if ( !token )
		return std::string( "the dispatch was refused at submission" );
	(void)m_Device.WaitIdle();
	cache.Retire( token.Value() );
	out.resize( resultBytes );
	const bool read = bool( m_Device.ReadBuffer( readback, 0, out ) );
	for ( BufferId id : { input, results, readback } )
		(void)m_Device.Release( id, token.Value() );
	(void)m_Device.Release( group.Value(), token.Value() );
	if ( !read )
		return std::string( "the results did not read back" );
	return std::nullopt;
}

} // namespace render::lab
