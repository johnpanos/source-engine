//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0015 serial content graph, compiler seam and package commit.
//
//=============================================================================//

#ifndef CONTENT_BUILD_GRAPH_H
#define CONTENT_BUILD_GRAPH_H

#include "content/asset_index.h"

#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace content
{

class BuildInputs
{
public:
	explicit BuildInputs( std::filesystem::path root );
	std::optional<std::span<const std::uint8_t>> Read(
	    std::string_view relative, std::string *error = nullptr );
	void Freeze() noexcept { m_Frozen = true; }
	const std::map<std::string, std::vector<std::uint8_t>> &Files() const noexcept
	{
		return m_Files;
	}

private:
	std::filesystem::path m_Root;
	std::map<std::string, std::vector<std::uint8_t>> m_Files;
	bool m_Frozen = false;
};

class IAssetCompiler
{
public:
	virtual ~IAssetCompiler() = default;
	virtual AssetKind Kind() const noexcept = 0;
	virtual std::string_view Id() const noexcept = 0;
	virtual std::uint32_t Version() const noexcept = 0;
	// Every input must be read through inputs before Plan returns. Compile can
	// reread those snapshots, but cannot add a hidden input after keying.
	virtual std::optional<std::vector<AssetEdge>> Plan(
	    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const = 0;
	virtual std::optional<std::vector<std::uint8_t>> Compile(
	    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const = 0;
	virtual std::map<std::string, std::string> ProfileFacts( std::string_view profile ) const
	{
		return {};
	}
};

class TexturePassthroughCompiler final : public IAssetCompiler
{
public:
	AssetKind Kind() const noexcept override { return AssetKind::Texture; }
	std::string_view Id() const noexcept override { return "texture.passthrough"; }
	std::uint32_t Version() const noexcept override { return 1; }
	std::optional<std::vector<AssetEdge>> Plan(
	    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const override;
	std::optional<std::vector<std::uint8_t>> Compile(
	    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const override;
};

struct BuildTraceNode
{
	AssetRef ref;
	std::string key;
	bool hit = false;
	std::vector<std::string> inputs;
};

struct BuildResult
{
	std::filesystem::path package;
	std::string version;
	std::vector<BuildTraceNode> trace;
};

class BuildGraph
{
public:
	BuildGraph( std::filesystem::path source, std::filesystem::path output, std::string profile,
	    std::array<std::uint8_t, 16> toolDigest );
	void Register( const IAssetCompiler &compiler );
	std::optional<BuildResult> Build(
	    std::span<const AssetRef> roots, std::string_view packageName, std::string &error );

private:
	std::filesystem::path m_Source;
	std::filesystem::path m_Output;
	std::string m_Profile;
	std::array<std::uint8_t, 16> m_ToolDigest;
	std::map<AssetKind, const IAssetCompiler *> m_Compilers;
};

// BLAKE2b-128 over the installed content_build executable or in-process
// compiler library. A changed compiler binary changes every action key.
std::optional<std::array<std::uint8_t, 16>> FileDigest(
    const std::filesystem::path &path, std::string &error );

} // namespace content

#endif // CONTENT_BUILD_GRAPH_H
