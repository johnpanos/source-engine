//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Descriptor sets grouped by update frequency for the native PBR and
//          GI stages (world_pbr.frag, world_pbr_glass.frag, model_pbr.frag).
//
//          The legacy stages give every texture its own one-sampler set, so a
//          PBR draw that reads a material, the split-sum table, the map's
//          lightmap, a probe volume and a shadow field needed up to eleven
//          sets. Vulkan guarantees only four (maxBoundDescriptorSets), and
//          devices at that limit lost PBR and GI. Grouped, the stages bind
//          three:
//
//            set 0  frame:    what the frame or the map selects (the split-sum
//                             table, the lightmap, the indirect source, the
//                             probe grid table, the shadow field);
//            set 1  material: the draw's material (base, MRAO, normal,
//                             emission or scene color, $envmap cube, scene
//                             depth);
//            set 2  constants: a dynamic uniform block (the frame's direct
//                             lights, or a model draw's pixel constants),
//                             owned by the caller.
//
//          Sets 0 and 1 are written on demand into per-frame-slot pools. A
//          slot's pools are reset only when that slot's previous submission
//          has completed (BeginFrame after the slot's fence), so a set is
//          never rewritten while a submitted frame may read it. Identical
//          requests within a slot reuse one set. A view or sampler may be
//          destroyed only after every submission that read it completed (the
//          owner's texture retirement); a sampler replaced in place must call
//          Invalidate.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_DESCRIPTOR_GROUPS_H
#define SHADERAPIVULKAN_VULKAN_DESCRIPTOR_GROUPS_H

#ifdef _WIN32
#pragma once
#endif

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace render_vulkan
{

class CGroupedDescriptors
{
public:
	enum Group
	{
		kFrameGroup = 0,
		kMaterialGroup = 1,
		kGroupCount
	};
	// Combined image samplers per group; every binding is written.
	enum
	{
		kFrameBindings = 6,
		kMaterialBindings = 6,
		kMaxBindings = 6
	};
	// Frame set bindings.
	enum
	{
		kFrameSplitSum = 0,
		kFrameLightmap = 1,
		kFrameIndirect = 2, // the producer's indirect atlas, change volume or probe atlas
		kFrameProbeGrid = 3,
		kFrameShadowField = 4,      // 3D
		kFrameReflectionProbes = 5, // the map's RPRB texture (R50-PARALLAX)
	};
	// Material set bindings.
	enum
	{
		kMaterialBase = 0,
		kMaterialMrao = 1,
		kMaterialNormal = 2,
		kMaterialEmission = 3,    // the scene color for glass
		kMaterialEnvironment = 4, // a cube
		kMaterialSceneDepth = 5,
	};
	struct Image
	{
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		bool operator==( const Image &other ) const
		{
			return view == other.view && sampler == other.sampler;
		}
	};

	CGroupedDescriptors() = default;
	~CGroupedDescriptors() { Shutdown(); }
	CGroupedDescriptors( const CGroupedDescriptors & ) = delete;
	CGroupedDescriptors &operator=( const CGroupedDescriptors & ) = delete;

	// `device` outlives this object. `slots` is the number of frames in
	// flight. False (with a reason) leaves nothing created.
	bool Init( VkDevice device, uint32_t slots, std::string *outError );
	// The owner has waited for the device.
	void Shutdown();
	bool Ready() const { return m_device != VK_NULL_HANDLE; }

	VkDescriptorSetLayout Layout( Group group ) const { return m_layouts[group]; }

	// Starts recording into `slot`, whose previous submission has completed:
	// its sets return to the pools.
	void BeginFrame( uint32_t slot );
	// A set of `group` holding `images` (exactly the group's binding count,
	// every view valid) in the current slot; VK_NULL_HANDLE when allocation
	// fails. Equal requests in one slot return the same set.
	VkDescriptorSet Acquire( Group group, const Image *images );
	// Forget the current slot's reuse table (a sampler was replaced in place).
	// Sets already bound stay valid until the slot is reset.
	void Invalidate();

	// Counters for the suites: sets written and requests served from reuse
	// since Init.
	uint64_t SetsWritten() const { return m_written; }
	uint64_t SetsReused() const { return m_reused; }
	// The seeded defect (render.grouped-descriptors' control): reuse keyed by
	// the views alone, so a request with another sampler gets a stale set.
	void SetSamplerKeyDefect( bool on ) { m_samplerKeyDefect = on; }

private:
	struct Key
	{
		uint32_t group = 0;
		Image images[kMaxBindings];
		bool operator==( const Key &other ) const
		{
			if ( group != other.group )
				return false;
			for ( int i = 0; i < kMaxBindings; ++i )
				if ( !( images[i] == other.images[i] ) )
					return false;
			return true;
		}
	};
	struct KeyHash
	{
		size_t operator()( const Key &key ) const;
	};
	struct Slot
	{
		std::vector<VkDescriptorPool> pools;
		size_t current = 0; // the pool new sets come from
		std::unordered_map<Key, VkDescriptorSet, KeyHash> reuse;
	};
	VkDescriptorPool NewPool();
	VkDescriptorSet Allocate( Slot &slot, Group group );

	VkDevice m_device = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_layouts[kGroupCount] = {};
	std::vector<Slot> m_slots;
	uint32_t m_slot = 0;
	uint64_t m_written = 0;
	uint64_t m_reused = 0;
	bool m_samplerKeyDefect = false;
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_DESCRIPTOR_GROUPS_H
