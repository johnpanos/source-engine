//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 building (RFC 0016). A frame declares passes and
//			the resources each reads and writes, in abstract usages; the graph
//			compiler derives order, culling, transitions and transient
//			lifetimes. Passes never issue transitions themselves.
//
//			- Imported resources (the swapchain image, persistent targets)
//			  enter in a declared usage and leave in a declared final usage.
//			- Transient resources exist only inside one execution. Their
//			  usages are the union of the accesses the graph declares.
//			- A pass is kept if it has a side effect or writes a resource that
//			  a kept pass reads or that is imported; others are culled.
//			- Declaration order is execution order among kept passes; a read
//			  sees the latest earlier write.
//
//=============================================================================//

#ifndef RENDER_GRAPH_GRAPH_BUILDER_H
#define RENDER_GRAPH_GRAPH_BUILDER_H

#include "render/device/encoder.h"
#include "render/device/resources.h"
#include "render/device/usage.h"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace render::graph
{

struct ResourceRef
{
	std::uint32_t index = UINT32_MAX;

	constexpr bool IsValid() const { return index != UINT32_MAX; }
	friend constexpr bool operator==( ResourceRef, ResourceRef ) = default;
};

enum class PassKind : std::uint8_t
{
	kRender,
	kCompute,
	kCopy
};

class RecordContext;
using ExecuteFn = std::function<void( RecordContext & )>;

struct Access
{
	ResourceRef resource;
	device::ResourceUsage usage = device::ResourceUsage::kUndefined;
	bool write = false;
};

struct ResourceDecl
{
	std::string name;
	bool imported = false;
	bool isTexture = true;
	device::TextureDesc texture;
	device::BufferDesc buffer;
	device::TextureId importedTexture;
	device::BufferId importedBuffer;
	device::ResourceUsage initialUsage = device::ResourceUsage::kUndefined;
	device::ResourceUsage finalUsage = device::ResourceUsage::kUndefined;
};

struct PassDecl
{
	std::string name;
	PassKind kind = PassKind::kRender;
	std::vector<Access> accesses;
	bool sideEffect = false;
	ExecuteFn execute;
};

class GraphBuilder;

// Declares one pass's accesses. Valid until the builder adds another pass.
class PassBuilder
{
public:
	PassBuilder &Read( ResourceRef resource, device::ResourceUsage usage );
	PassBuilder &Write( ResourceRef resource, device::ResourceUsage usage );
	// Kept even if nothing reads what it writes (presentation, readback).
	PassBuilder &SideEffect();
	PassBuilder &Execute( ExecuteFn execute );

private:
	friend class GraphBuilder;
	PassBuilder( GraphBuilder &builder, std::size_t pass ) : m_Builder( builder ), m_Pass( pass ) {}

	GraphBuilder &m_Builder;
	std::size_t m_Pass;
};

class GraphBuilder
{
public:
	ResourceRef ImportTexture( std::string_view name, device::TextureId texture,
	    const device::TextureDesc &desc, device::ResourceUsage current,
	    device::ResourceUsage final );
	ResourceRef ImportBuffer( std::string_view name, device::BufferId buffer,
	    const device::BufferDesc &desc, device::ResourceUsage current,
	    device::ResourceUsage final );
	ResourceRef CreateTexture( std::string_view name, const device::TextureDesc &desc );
	ResourceRef CreateBuffer( std::string_view name, const device::BufferDesc &desc );
	PassBuilder AddPass( std::string_view name, PassKind kind );

	const std::vector<ResourceDecl> &Resources() const { return m_Resources; }
	const std::vector<PassDecl> &Passes() const { return m_Passes; }

private:
	friend class PassBuilder;
	std::vector<ResourceDecl> m_Resources;
	std::vector<PassDecl> m_Passes;
};

} // namespace render::graph

#endif // RENDER_GRAPH_GRAPH_BUILDER_H
