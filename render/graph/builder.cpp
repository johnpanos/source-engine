//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.graph.v1 builder (RFC 0016).
//
//=============================================================================//

#include "render/graph/graph_builder.h"

#include <utility>

namespace render::graph
{

PassBuilder &PassBuilder::Read( ResourceRef resource, device::ResourceUsage usage )
{
	m_Builder.m_Passes[m_Pass].accesses.push_back( { resource, usage, false } );
	return *this;
}

PassBuilder &PassBuilder::Write( ResourceRef resource, device::ResourceUsage usage )
{
	m_Builder.m_Passes[m_Pass].accesses.push_back( { resource, usage, true } );
	return *this;
}

PassBuilder &PassBuilder::SideEffect()
{
	m_Builder.m_Passes[m_Pass].sideEffect = true;
	return *this;
}

PassBuilder &PassBuilder::OnQueue( Queue queue )
{
	m_Builder.m_Passes[m_Pass].queue = queue;
	return *this;
}

PassBuilder &PassBuilder::Execute( ExecuteFn execute )
{
	m_Builder.m_Passes[m_Pass].execute = std::move( execute );
	return *this;
}

ResourceRef GraphBuilder::ImportTexture( std::string_view name, device::TextureId texture,
    const device::TextureDesc &desc, device::ResourceUsage current, device::ResourceUsage final )
{
	ResourceDecl decl;
	decl.name = name;
	decl.imported = true;
	decl.isTexture = true;
	decl.texture = desc;
	decl.texture.debugName = {};
	decl.importedTexture = texture;
	decl.initialUsage = current;
	decl.finalUsage = final;
	m_Resources.push_back( std::move( decl ) );
	return { static_cast<std::uint32_t>( m_Resources.size() - 1 ) };
}

ResourceRef GraphBuilder::ImportBuffer( std::string_view name, device::BufferId buffer,
    const device::BufferDesc &desc, device::ResourceUsage current, device::ResourceUsage final )
{
	ResourceDecl decl;
	decl.name = name;
	decl.imported = true;
	decl.isTexture = false;
	decl.buffer = desc;
	decl.buffer.debugName = {};
	decl.importedBuffer = buffer;
	decl.initialUsage = current;
	decl.finalUsage = final;
	m_Resources.push_back( std::move( decl ) );
	return { static_cast<std::uint32_t>( m_Resources.size() - 1 ) };
}

ResourceRef GraphBuilder::CreateTexture( std::string_view name, const device::TextureDesc &desc )
{
	ResourceDecl decl;
	decl.name = name;
	decl.isTexture = true;
	decl.texture = desc;
	decl.texture.debugName = {};
	m_Resources.push_back( std::move( decl ) );
	return { static_cast<std::uint32_t>( m_Resources.size() - 1 ) };
}

ResourceRef GraphBuilder::CreateBuffer( std::string_view name, const device::BufferDesc &desc )
{
	ResourceDecl decl;
	decl.name = name;
	decl.isTexture = false;
	decl.buffer = desc;
	decl.buffer.debugName = {};
	m_Resources.push_back( std::move( decl ) );
	return { static_cast<std::uint32_t>( m_Resources.size() - 1 ) };
}

PassBuilder GraphBuilder::AddPass( std::string_view name, PassKind kind )
{
	PassDecl pass;
	pass.name = name;
	pass.kind = kind;
	m_Passes.push_back( std::move( pass ) );
	return PassBuilder( *this, m_Passes.size() - 1 );
}

} // namespace render::graph
