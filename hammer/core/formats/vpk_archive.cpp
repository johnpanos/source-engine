//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hammer's VPK asset source over the content.vpk reader. See
//			public/hammer/formats/vpk_archive.h.
//
//=============================================================================//

#include "hammer/formats/vpk_archive.h"

namespace hammer::formats
{

std::unique_ptr<VpkArchive> VpkArchive::Open(
    const hammer::ports::IByteStore &store, const std::string &dirVpkPath, std::string &error )
{
	std::unique_ptr<VpkArchive> archive( new VpkArchive( store ) );
	archive->m_archive = content::VpkArchive::Open( archive->m_source, dirVpkPath, error );
	if ( !archive->m_archive )
		return nullptr;
	return archive;
}

bool VpkArchive::HasAsset( const std::string &path ) const
{
	return m_archive->Find( path ) != nullptr;
}

bool VpkArchive::ReadAsset( const std::string &path, std::string &out ) const
{
	return m_archive->Read( path, out );
}

void VpkArchive::ListAssets( const std::string &prefix, const std::string &extensionWithDot,
    std::vector<std::string> &out ) const
{
	m_archive->List( prefix, extensionWithDot, out );
}

} // namespace hammer::formats
