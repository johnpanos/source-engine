//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/render/material/material_programs.h.
//
//=============================================================================//

#include "render/material/material_programs.h"

#include <algorithm>

namespace render::material
{

using namespace render::device;

namespace
{

bool SameSampler( const SamplerDesc &a, const SamplerDesc &b )
{
	return a.minFilter == b.minFilter && a.magFilter == b.magFilter && a.mipFilter == b.mipFilter &&
	       a.address == b.address && a.maxAnisotropy == b.maxAnisotropy;
}

} // namespace

// --- GroupResidency ---------------------------------------------------------

GroupResidency::GroupResidency( IRenderDevice2 &device, const resources::TextureCache &textures )
    : m_Device( device ), m_Textures( textures )
{
}

GroupResidency::~GroupResidency()
{
	for ( auto &[id, entry] : m_Entries )
		Retiring( entry );
	for ( const ResourceId &resource : m_Replaced )
		(void)m_Device.Release( resource, m_LastToken );
	for ( const auto &[desc, sampler] : m_Samplers )
		(void)m_Device.Release( sampler, m_LastToken );
	for ( TextureId neutral : { m_Neutral2D, m_NeutralCube } )
	{
		if ( neutral.IsValid() )
			(void)m_Device.Release( neutral, m_LastToken );
	}
	if ( m_NeutralStaging.IsValid() )
		(void)m_Device.Release( m_NeutralStaging, m_LastToken );
}

TextureId GroupResidency::Neutral( TextureDimension dimension )
{
	TextureId &slot = dimension == TextureDimension::kCube ? m_NeutralCube : m_Neutral2D;
	if ( slot.IsValid() )
		return slot;
	TextureDesc desc;
	desc.dimension = dimension;
	desc.format = Format::kRGBA8Unorm;
	desc.width = desc.height = 1;
	desc.depthOrLayers = dimension == TextureDimension::kCube ? 6 : 1;
	desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
	desc.debugName = "material neutral texture";
	auto texture = m_Device.CreateTexture( desc );
	if ( !texture )
		return {};
	slot = texture.Value();
	m_NeutralUploaded = false; // a new one to fill
	return slot;
}

// Queues an entry's group and buffer for release at the next Retire (they may
// still be in flight).
void GroupResidency::Retiring( Entry &entry )
{
	if ( entry.resident.group.IsValid() )
		m_Replaced.push_back( entry.resident.group );
	if ( entry.constants.IsValid() )
		m_Replaced.push_back( entry.constants );
	entry.resident = ResidentGroup();
	entry.constants = BufferId();
}

foundation::Expected<void, ProgramStatus> GroupResidency::Set(
    std::uint64_t id, const GroupRequest &request )
{
	if ( !request.layout.IsValid() )
		return foundation::MakeUnexpected( ProgramStatus::kInvalidRequest );
	for ( const ProgramTexture &texture : request.textures )
	{
		if ( !SamplerFor( texture.sampler ) )
			return foundation::MakeUnexpected( ProgramStatus::kDevice );
	}
	Entry entry;
	entry.request = request;
	if ( !request.constants.empty() )
	{
		BufferDesc desc;
		desc.size = request.constants.size();
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kUniform };
		auto buffer = m_Device.CreateBuffer( desc );
		if ( !buffer )
			return foundation::MakeUnexpected( ProgramStatus::kDevice );
		entry.constants = buffer.Value();
	}
	if ( auto found = m_Entries.find( id ); found != m_Entries.end() )
	{
		Retiring( found->second );
		found->second = std::move( entry );
	}
	else
	{
		m_Entries.emplace( id, std::move( entry ) );
	}
	return {};
}

void GroupResidency::Remove( std::uint64_t id )
{
	auto found = m_Entries.find( id );
	if ( found == m_Entries.end() )
		return;
	Retiring( found->second );
	m_Entries.erase( found );
}

foundation::Expected<SamplerId, ProgramStatus> GroupResidency::SamplerFor( const SamplerDesc &desc )
{
	for ( const auto &[known, sampler] : m_Samplers )
	{
		if ( SameSampler( known, desc ) )
			return sampler;
	}
	auto sampler = m_Device.CreateSampler( desc );
	if ( !sampler )
		return foundation::MakeUnexpected( ProgramStatus::kDevice );
	m_Samplers.emplace_back( desc, sampler.Value() );
	return sampler.Value();
}

void GroupResidency::Refresh( Entry &entry )
{
	// The textures as the cache holds them now; any missing one means no group.
	std::vector<SampledTexture> sampled;
	std::vector<std::uint64_t> revisions;
	bool complete = true;
	for ( const ProgramTexture &texture : entry.request.textures )
	{
		// An input named empty is off: it takes the neutral texture of its
		// dimension, which the program does not read.
		if ( texture.name.empty() )
		{
			const TextureId neutral = Neutral( texture.dimension );
			if ( !neutral.IsValid() || !m_NeutralUploaded )
			{
				complete = false;
				continue;
			}
			TextureDesc desc;
			desc.dimension = texture.dimension;
			desc.format = Format::kRGBA8Unorm;
			desc.width = desc.height = 1;
			desc.depthOrLayers = texture.dimension == TextureDimension::kCube ? 6 : 1;
			sampled.push_back( { neutral, desc } );
			revisions.push_back( 0 );
			continue;
		}
		const resources::TextureEntry *found = m_Textures.Find( texture.name );
		if ( !found )
		{
			complete = false;
			continue;
		}
		SampledTexture resident{ found->texture, found->desc };
		resident.desc.debugName = {}; // the cache's name view is not ours to keep
		sampled.push_back( resident );
		revisions.push_back( found->revision );
	}
	const bool current = complete && entry.resident.group.IsValid() &&
	                     revisions == entry.revisions &&
	                     std::equal( sampled.begin(), sampled.end(),
	                         entry.resident.textures.begin(), entry.resident.textures.end(),
	                         []( const SampledTexture &a, const SampledTexture &b )
	                         {
		                         return a.texture == b.texture;
	                         } );
	if ( current )
		return;
	if ( entry.resident.group.IsValid() )
		m_Replaced.push_back( entry.resident.group );
	entry.resident = ResidentGroup();
	entry.revisions.clear();
	if ( !complete || !entry.uploaded )
		return;

	std::vector<BindGroupEntry> bindings;
	if ( entry.constants.IsValid() )
		bindings.push_back( { entry.request.constantsBinding, entry.constants, 0,
		    entry.request.constants.size(), {}, {} } );
	for ( std::size_t i = 0; i < entry.request.textures.size(); ++i )
	{
		const ProgramTexture &texture = entry.request.textures[i];
		auto sampler = SamplerFor( texture.sampler );
		if ( !sampler )
		{
			++m_Failures;
			return;
		}
		bindings.push_back( { texture.binding, {}, 0, 0, sampled[i].texture, {} } );
		bindings.push_back( { texture.samplerBinding, {}, 0, 0, {}, sampler.Value() } );
	}
	auto group = m_Device.CreateBindGroup( { entry.request.layout, bindings } );
	if ( !group )
	{
		++m_Failures;
		return;
	}
	entry.resident.group = group.Value();
	entry.resident.textures = std::move( sampled );
	if ( entry.constants.IsValid() )
		entry.resident.uniforms.push_back( entry.constants );
	entry.revisions = std::move( revisions );
}

std::size_t GroupResidency::RecordUploads( CommandEncoder &encoder )
{
	std::size_t recorded = 0;
	// The neutral textures any entry names: made on first sight, filled once.
	for ( auto &[id, entry] : m_Entries )
	{
		for ( const ProgramTexture &texture : entry.request.textures )
		{
			if ( texture.name.empty() )
				(void)Neutral( texture.dimension );
		}
	}
	if ( !m_NeutralUploaded && ( m_Neutral2D.IsValid() || m_NeutralCube.IsValid() ) )
	{
		if ( !m_NeutralStaging.IsValid() )
		{
			BufferDesc staging;
			staging.size = 4;
			staging.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kCopySource };
			auto buffer = m_Device.CreateBuffer( staging );
			if ( buffer )
				m_NeutralStaging = buffer.Value();
		}
		if ( m_NeutralStaging.IsValid() )
		{
			const std::byte white[4] = {
			    std::byte( 255 ), std::byte( 255 ), std::byte( 255 ), std::byte( 255 ) };
			encoder.TransitionBuffer(
			    m_NeutralStaging, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
			encoder.WriteBuffer( m_NeutralStaging, 0, white );
			encoder.TransitionBuffer(
			    m_NeutralStaging, ResourceUsage::kCopyDestination, ResourceUsage::kCopySource );
			for ( TextureId neutral : { m_Neutral2D, m_NeutralCube } )
			{
				if ( !neutral.IsValid() )
					continue;
				const std::uint32_t layers = neutral == m_NeutralCube ? 6 : 1;
				encoder.TransitionTexture(
				    neutral, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
				for ( std::uint32_t layer = 0; layer < layers; ++layer )
					encoder.CopyBufferToTexture( m_NeutralStaging, neutral, { 0, 0, layer, 1, 1 } );
				encoder.TransitionTexture(
				    neutral, ResourceUsage::kCopyDestination, ResourceUsage::kSampled );
			}
			m_NeutralUploaded = true;
			++recorded;
		}
	}
	for ( auto &[id, entry] : m_Entries )
	{
		if ( !entry.uploaded )
		{
			if ( entry.constants.IsValid() )
			{
				encoder.TransitionBuffer(
				    entry.constants, ResourceUsage::kUndefined, ResourceUsage::kCopyDestination );
				encoder.WriteBuffer( entry.constants, 0, entry.request.constants );
				encoder.TransitionBuffer(
				    entry.constants, ResourceUsage::kCopyDestination, ResourceUsage::kUniform );
				++recorded;
			}
			entry.uploaded = true;
		}
		Refresh( entry );
	}
	return recorded;
}

void GroupResidency::Retire( CompletionToken token )
{
	for ( const ResourceId &resource : m_Replaced )
		(void)m_Device.Release( resource, token );
	m_Replaced.clear();
	m_LastToken = token;
}

const ResidentGroup *GroupResidency::Group( std::uint64_t id ) const
{
	auto found = m_Entries.find( id );
	if ( found == m_Entries.end() || !found->second.resident.group.IsValid() )
		return nullptr;
	return &found->second.resident;
}

std::size_t GroupResidency::ReadyCount() const
{
	return static_cast<std::size_t>( std::count_if( m_Entries.begin(), m_Entries.end(),
	    []( const auto &pair )
	    {
		    return pair.second.resident.group.IsValid();
	    } ) );
}

// --- MaterialPrograms -------------------------------------------------------

foundation::Expected<void, ProgramStatus> MaterialPrograms::Set(
    std::uint64_t material, const ProgramRequest &request )
{
	if ( !request.pipeline.IsValid() || request.vertexStride == 0 ||
	     request.drawConstantBytes > sizeof( FamilyDrawConstants ) ||
	     request.drawConstantBytes % 4 != 0 )
		return foundation::MakeUnexpected( ProgramStatus::kInvalidRequest );
	if ( auto set = m_Groups.Set( material, request.material ); !set )
		return set;
	DrawProgram &program = m_Programs[material];
	program = DrawProgram();
	program.pipeline = request.pipeline;
	program.vertexStride = request.vertexStride;
	program.drawConstantBytes = request.drawConstantBytes;
	program.drawLayout = request.drawLayout;
	program.frameLayout = request.frameLayout;
	program.viewLayout = request.viewLayout;
	return {};
}

void MaterialPrograms::Remove( std::uint64_t material )
{
	m_Groups.Remove( material );
	m_Programs.erase( material );
}

std::size_t MaterialPrograms::RecordUploads( CommandEncoder &encoder )
{
	const std::size_t recorded = m_Groups.RecordUploads( encoder );
	for ( auto &[material, program] : m_Programs )
	{
		const ResidentGroup *group = m_Groups.Group( material );
		program.material = group ? *group : ResidentGroup();
	}
	return recorded;
}

const DrawProgram *MaterialPrograms::Program( std::uint64_t material ) const
{
	auto found = m_Programs.find( material );
	if ( found == m_Programs.end() || !found->second.material.group.IsValid() )
		return nullptr;
	return &found->second;
}

// --- DrawGroups -------------------------------------------------------------

foundation::Expected<void, ProgramStatus> DrawGroups::Set(
    std::uint64_t id, const GroupRequest &request )
{
	if ( auto set = m_Groups.Set( id, request ); !set )
		return set;
	DrawGroup &draw = m_Draws[id];
	draw = DrawGroup();
	draw.layout = request.layout;
	return {};
}

void DrawGroups::Remove( std::uint64_t id )
{
	m_Groups.Remove( id );
	m_Draws.erase( id );
}

std::size_t DrawGroups::RecordUploads( CommandEncoder &encoder )
{
	const std::size_t recorded = m_Groups.RecordUploads( encoder );
	for ( auto &[id, draw] : m_Draws )
	{
		const ResidentGroup *group = m_Groups.Group( id );
		draw.resident = group ? *group : ResidentGroup();
	}
	return recorded;
}

const DrawGroup *DrawGroups::Group( std::uint64_t id ) const
{
	auto found = m_Draws.find( id );
	if ( found == m_Draws.end() || !found->second.resident.group.IsValid() )
		return nullptr;
	return &found->second;
}

} // namespace render::material
