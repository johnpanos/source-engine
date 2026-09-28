//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VMT import (RFC 0016 "Materials", K4 "VMT corpus"). Reads a legacy
//			.vmt as the material system does (patch files with include,
//			insert and replace; [$SYMBOL] tags; "cond?$key" variables;
//			fallback blocks such as ">=dx90", "GPU>=1" and
//			"<shader>_hdr_dx9"; first definitions winning) for one profile,
//			then maps it through the VMT mapping (vmt_mapping.h) onto a
//			family: its parameters, texture references with their
//			"materials/" prefix, the proxies block as written, editor ('%')
//			keys, metadata keys and the keys the family does not map. Nothing
//			in the VMT is dropped silently. This is also the material compiler
//			RFC 0015 C1 calls `material.vmt`, so runtime and build share one
//			mapping.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_VMT_IMPORT_H
#define RENDER_MATERIAL_VMT_IMPORT_H

#include "foundation/expected.h"
#include "render/material/parameter_block.h"
#include "render/material/vmt_mapping.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace render::material
{

// The hardware a VMT's conditions are evaluated for. The defaults are the
// native Vulkan backend's: DirectX support level 95 with ps_2_b, integer HDR,
// linear-space sRGB blending, gpu_level 3 (high; the material system's default
// without a game value) and a Linux PC for [$SYMBOL] tags.
struct VmtProfile
{
	int dxLevel = 95;
	bool pixelShader20b = true;
	bool hdr = true;
	bool srgbBlending = true;
	int gpuLevel = 3;
	bool reduceParticles = false; // mat_reduceparticles ("lowfill?")
	// [$SYMBOL] tags that hold; any other symbol is false ($WIN32 means "a PC").
	std::vector<std::string> symbols = { "WIN32", "LINUX", "POSIX" };
};

enum class ImportStatus : std::uint8_t
{
	kMalformed = 1,       // not KeyValues text, or no material block
	kMissingInclude,      // a patch's include is not found
	kPatchWithoutInclude, // a patch names no include
	kIncludeDepth,        // patches nest more than ten deep, as the legacy loader allows
	kUnknownShader,       // no family and no legacy shader has the name
	kMissingRequired,     // a family's required parameter is absent (PBRMetalRough)
	kInvalidReference     // a material reference is malformed, missing or itself
};

struct ImportError
{
	ImportStatus status = ImportStatus::kMalformed;
	std::string detail;
};

std::string_view ImportStatusName( ImportStatus status );

// Finds a file of the game's search path ("materials/a/b.vmt", lower case,
// forward slashes); nullopt when it does not exist.
using VmtResolver = std::function<std::optional<std::string>( std::string_view path )>;

struct VmtImportContext
{
	VmtProfile profile;
	VmtResolver resolve;                      // empty: every include is missing
	std::string_view path;                    // the material's own file, for self-references
	const VmtMappingTable *mapping = nullptr; // nullptr: BuiltinVmtMapping()
};

struct MaterialValue
{
	std::string parameter; // the family parameter
	std::string key;       // the VMT key, as written
	ValueKind kind = ValueKind::kFloat;
	float numbers[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	std::string text; // kTexture and kMaterial: the normalized reference
};

struct VmtPair
{
	std::string key;
	std::string value;
};

struct ProxyDesc
{
	std::string name;
	std::vector<VmtPair> parameters;
};

struct MaterialDesc
{
	std::string family;                // a mapping family, or kLegacyFamily
	std::string shader;                // the shader as the VMT names it
	std::string legacyShader;          // lower case, after fallback aliases
	std::string reason;                // the mapping row's reason
	std::string fallbackBlock;         // the fallback block the profile selected, or empty
	std::vector<MaterialValue> values; // family parameters the VMT sets, in key-row order
	std::vector<VmtPair> variables;    // every variable after conditions, fallback and patches
	std::vector<std::string> unmapped; // variables the family does not map (lower case)
	std::vector<VmtPair> metadata;     // keys read outside the renderer
	std::vector<VmtPair> editorKeys;   // '%' keys
	std::vector<ProxyDesc> proxies;
	std::vector<std::string> includes;    // patch includes, in load order
	std::vector<std::string> diagnostics; // values read with a fallback, unknown conditions
};

foundation::Expected<MaterialDesc, ImportError> ImportVmt(
    std::string_view text, const VmtImportContext &context );

// Writes a material's GPU parameters into a block of its family (textures and
// material references are bound elsewhere). Fails on a parameter the block's
// schema lacks or declares with another type.
foundation::Expected<void, MaterialError> ApplyValues(
    const MaterialDesc &material, ParameterBlock &block );

} // namespace render::material

#endif // RENDER_MATERIAL_VMT_IMPORT_H
