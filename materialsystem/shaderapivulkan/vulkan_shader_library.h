//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What the backend's embedded SPIR-V is called and, optionally, the
//          debug build it runs instead. Each embedded array is known by its
//          content hash: material_spv_index.h (generated with material_spv.h)
//          names it, and a debug directory written by
//          `shaders/regen_material_spv.py --debug-out DIR` holds, per hash, the
//          same GLSL compiled with source-level debug information, which a
//          capture tool shows as GLSL names and source steps.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_SHADER_LIBRARY_H
#define SHADERAPIVULKAN_VULKAN_SHADER_LIBRARY_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace render_vulkan
{

// FNV-1a 64 over the code's bytes (regen_material_spv.py's spirv_hash).
[[nodiscard]] uint64_t SpirvHash( const uint32_t *code, size_t sizeBytes );

// The index's name of an embedded array with this hash ("world_pbr.frag
// -DCLIP_PLANES"), or nullptr for code the index does not list.
[[nodiscard]] const char *EmbeddedShaderName( uint64_t hash );

// The code a module is created from.
struct ShaderModuleCode
{
	const uint32_t *code = nullptr;
	size_t sizeBytes = 0;
	const char *name = nullptr; // nullptr when the index does not list it
	bool debugVariant = false;
};

// Immutable after LoadDebugDirectory; Resolve may be called from any thread.
class VulkanShaderLibrary
{
public:
	struct LoadReport
	{
		uint32_t loaded = 0;
		// One line per file that was not used, with the reason.
		std::vector<std::string> rejected;
	};

	// Reads every <array>.<16 hex digits>.spv in `directory`. A file is used
	// when its hash names an embedded array (else the embedded code changed
	// since it was written: stale), it is well-formed SPIR-V, and it needs no
	// NonSemantic extended instructions the device cannot take
	// (VK_KHR_shader_non_semantic_info). Replaces any earlier load.
	LoadReport LoadDebugDirectory( const char *directory, bool nonSemanticInfoSupported );
	// Loads one variant (the directory scan's per-file step; also the suites').
	bool AddDebugVariant( uint64_t hash, std::vector<uint32_t> code, bool nonSemanticInfoSupported,
	    std::string *outReason );

	// The embedded code, or its debug variant when one was loaded, and its name.
	ShaderModuleCode Resolve( const uint32_t *code, size_t sizeBytes ) const;
	size_t DebugVariants() const { return m_debug.size(); }

private:
	std::unordered_map<uint64_t, std::vector<uint32_t>> m_debug;
};

// True for code that imports a NonSemantic.* extended instruction set.
[[nodiscard]] bool SpirvUsesNonSemanticInfo( const uint32_t *code, size_t words );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_SHADER_LIBRARY_H
