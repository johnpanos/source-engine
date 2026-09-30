//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The owners of the bind groups a renderer draws families with
//			(RFC 0016 K4, render.material).
//
//			GroupResidency is the one mechanism: per id, a group of one layout
//			holding an optional uniform buffer of packed constants and named
//			textures with their samplers. Textures come from render.resources'
//			TextureCache by name. An id has a group only while every texture
//			it names is in the cache; until then it resolves to nothing and
//			the pass counts the draw as unresolved. The group is rebuilt when a
//			texture's revision rises (the cache replaced it), so a group never
//			names a replaced texture. Samplers are shared by description.
//
//			MaterialPrograms holds, per material id, the family's program (a
//			pipeline, stride, draw-constant size and the draw layout it reads)
//			with its material group, and serves them as IDrawPrograms.
//			DrawGroups holds per-draw groups (a lightmap page per surface) and
//			serves them as IDrawGroups. A family turns a claimed material into
//			a ProgramRequest, and a per-draw resource into a GroupRequest.
//
//			Protocol (the caches' own): Set/Remove at any time on the render
//			sequence; RecordUploads(encoder) records the pending constant
//			uploads, leaving each buffer in kUniform, and (re)creates the
//			groups; Retire(token), with that encoder's submission token,
//			releases replaced buffers and groups behind it. Lookups are valid
//			for recording after RecordUploads, in a submission no earlier than
//			the one holding the textures' own uploads (TextureCache counts a
//			texture as present from Stage). Not thread-safe: one sequence.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_MATERIAL_PROGRAMS_H
#define RENDER_MATERIAL_MATERIAL_PROGRAMS_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/material/draw_program.h"
#include "render/resources/texture_cache.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace render::material
{

struct ProgramTexture
{
	std::uint32_t binding = 0;        // the sampled texture's binding
	std::string name;                 // its TextureCache name
	std::uint32_t samplerBinding = 0; // the sampler's binding
	device::SamplerDesc sampler;
	// Sampled through an sRGB view of its format (the family decodes it
	// before filtering, as D3D9's SRGBTEXTURE does).
	bool srgb = false;
	// A cube map binding takes a kCube texture (an absent one, a neutral
	// cube).
	device::TextureDimension dimension = device::TextureDimension::k2D;
	// A texture the group's owner made on the device instead of a cache
	// name (a pass's output: the frame's shadow atlas), bound as it is. The
	// owner keeps it alive past the group's last use and has it in kSampled
	// wherever the group is read; the group is rebuilt when the id changes.
	device::TextureId external = {};
	device::TextureDesc externalDesc = {};
};

// A read-only storage buffer of a group: its binding and contents (at least
// one byte).
struct GroupBuffer
{
	std::uint32_t binding = 0;
	std::vector<std::byte> bytes;
};

struct GroupRequest
{
	device::BindGroupLayoutId layout;
	std::uint32_t constantsBinding = 0;
	std::vector<std::byte> constants; // empty: the group has no uniform buffer
	std::vector<ProgramTexture> textures;
	std::vector<GroupBuffer> storage; // uploaded once, then in kStorageRead
};

struct ProgramRequest
{
	device::PipelineId pipeline;
	std::uint32_t vertexStride = 0;
	std::uint32_t drawConstantBytes = 0;
	device::BindGroupLayoutId drawLayout;  // invalid when the family reads no draw group
	device::BindGroupLayoutId frameLayout; // invalid when the family reads no frame group
	device::BindGroupLayoutId viewLayout;  // invalid when the family reads no view group
	GroupRequest material;                 // role kMaterial
	// The view group a draw binds when its frame supplies none of viewLayout
	// (MaterialPrograms keeps one per layout).
	std::optional<GroupRequest> neutralView;
};

enum class ProgramStatus : std::uint8_t
{
	kInvalidRequest = 1, // no pipeline or layout, a stride of 0, or oversized draw constants
	kDevice              // a buffer or sampler was refused
};

class GroupResidency
{
public:
	// Borrows both; they outlive the residency.
	GroupResidency( device::IRenderDevice2 &device, const resources::TextureCache &textures );
	~GroupResidency();
	GroupResidency( const GroupResidency & ) = delete;
	GroupResidency &operator=( const GroupResidency & ) = delete;

	// Adds or replaces a group; its constants upload at the next RecordUploads.
	foundation::Expected<void, ProgramStatus> Set( std::uint64_t id, const GroupRequest &request );
	void Remove( std::uint64_t id );
	std::size_t RecordUploads( device::CommandEncoder &encoder );
	void Retire( device::CompletionToken token );

	// nullptr unless the group is ready.
	const ResidentGroup *Group( std::uint64_t id ) const;
	std::size_t Count() const { return m_Entries.size(); }
	std::size_t ReadyCount() const;
	// Groups that could not be created (device refusals).
	std::uint32_t Failures() const { return m_Failures; }

private:
	struct Entry
	{
		GroupRequest request;
		device::BufferId constants;
		std::vector<device::BufferId> storage; // per GroupRequest::storage
		bool uploaded = false;
		std::vector<std::uint64_t> revisions; // per texture, of the current group
		ResidentGroup resident;               // resident.group invalid until ready
	};

	foundation::Expected<device::SamplerId, ProgramStatus> SamplerFor(
	    const device::SamplerDesc &desc );
	void Refresh( Entry &entry );
	void Retiring( Entry &entry );

	device::IRenderDevice2 &m_Device;
	const resources::TextureCache &m_Textures;
	std::map<std::uint64_t, Entry> m_Entries;
	std::vector<std::pair<device::SamplerDesc, device::SamplerId>> m_Samplers;
	std::vector<device::ResourceId> m_Replaced; // released at Retire
	// The neutral textures an input named empty takes (a term that is off):
	// a 1x1 white 2D texture and a 1x1 cube, made at the first such input and
	// filled at the next RecordUploads.
	device::TextureId m_Neutral2D;
	device::TextureId m_NeutralCube;
	device::BufferId m_NeutralStaging;
	bool m_NeutralUploaded = false;
	device::TextureId Neutral( device::TextureDimension dimension );
	std::uint32_t m_Failures = 0;
	device::CompletionToken m_LastToken;
};

class MaterialPrograms final : public IDrawPrograms
{
public:
	MaterialPrograms( device::IRenderDevice2 &device, const resources::TextureCache &textures )
	    : m_Groups( device, textures ), m_Views( device, textures )
	{
	}

	foundation::Expected<void, ProgramStatus> Set(
	    std::uint64_t material, const ProgramRequest &request );
	void Remove( std::uint64_t material );
	// Records pending constant uploads and (re)creates the material groups
	// whose textures became resident or were replaced; returns the uploads.
	std::size_t RecordUploads( device::CommandEncoder &encoder );
	void Retire( device::CompletionToken token )
	{
		m_Groups.Retire( token );
		m_Views.Retire( token );
	}

	const DrawProgram *Program( std::uint64_t material ) const override;
	std::size_t Count() const { return m_Programs.size(); }
	std::size_t ReadyCount() const { return m_Groups.ReadyCount(); }
	std::uint32_t GroupFailures() const { return m_Groups.Failures(); }

private:
	GroupResidency m_Groups;
	GroupResidency m_Views; // the neutral view groups, by view layout
	std::set<std::uint64_t> m_NeutralViewLayouts;
	std::map<std::uint64_t, DrawProgram> m_Programs; // material group filled at RecordUploads
};

class DrawGroups final : public IDrawGroups
{
public:
	DrawGroups( device::IRenderDevice2 &device, const resources::TextureCache &textures )
	    : m_Groups( device, textures )
	{
	}

	foundation::Expected<void, ProgramStatus> Set( std::uint64_t id, const GroupRequest &request );
	void Remove( std::uint64_t id );
	std::size_t RecordUploads( device::CommandEncoder &encoder );
	void Retire( device::CompletionToken token ) { m_Groups.Retire( token ); }

	const DrawGroup *Group( std::uint64_t id ) const override;
	std::size_t Count() const { return m_Draws.size(); }
	std::size_t ReadyCount() const { return m_Groups.ReadyCount(); }
	std::uint32_t GroupFailures() const { return m_Groups.Failures(); }

private:
	GroupResidency m_Groups;
	std::map<std::uint64_t, DrawGroup> m_Draws; // resident filled at RecordUploads
};

} // namespace render::material

#endif // RENDER_MATERIAL_MATERIAL_PROGRAMS_H
