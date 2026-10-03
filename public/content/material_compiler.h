//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0015 VMT passthrough compiler and reference extraction.
//
//=============================================================================//

#ifndef CONTENT_MATERIAL_COMPILER_H
#define CONTENT_MATERIAL_COMPILER_H

#include "content/build_graph.h"

namespace content
{

class MaterialVmtCompiler final : public IAssetCompiler
{
public:
	AssetKind Kind() const noexcept override { return AssetKind::Material; }
	std::string_view Id() const noexcept override { return "material.vmt"; }
	std::uint32_t Version() const noexcept override { return 1; }
	std::optional<std::vector<AssetEdge>> Plan(
	    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const override;
	std::optional<std::vector<std::uint8_t>> Compile(
	    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const override;
};

} // namespace content

#endif // CONTENT_MATERIAL_COMPILER_H
