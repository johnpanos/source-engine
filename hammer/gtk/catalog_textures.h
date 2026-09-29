//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The textured preview's material source for the GTK shell (RFC
//			0002 hammer.adapters.gtk; RFC 0016 K5 ResolvePreview): a
//			hammer::render_adapter::IMaterialTextures over the game's VPKs.
//			A material is imported by render::material::ImportVmt
//			(render_adapter::ImportSourceMaterial) from its own asset source,
//			which resolves patch includes, and its base texture is decoded by
//			hammer::formats::MaterialCatalog::TextureImage (KTX2 first, with
//			the KTX2 preview decoder when the build has it, else VTF).
//			It mounts its own archives, search path and catalog, so the
//			render sequence that owns it shares nothing with the window's
//			material browser.
//
//=============================================================================//

#ifndef HAMMER_GTK_CATALOG_TEXTURES_H
#define HAMMER_GTK_CATALOG_TEXTURES_H

#include "hammer/adapters/platform/disk_byte_store.h"
#include "hammer/adapters/render/material_textures.h"
#include "hammer/formats/material_catalog.h"
#include "hammer/formats/search_path_assets.h"
#include "hammer/formats/vpk_archive.h"

#include <memory>
#include <string>
#include <vector>

namespace hammer::gtk
{

class CatalogTextures final : public render_adapter::IMaterialTextures
{
public:
	// Mounts a comma-separated list of _dir.vpk paths. Nothing when none
	// mounts; 'errors' names each path that failed.
	static std::unique_ptr<CatalogTextures> Open( const std::string &vpkList, std::string &errors );

	foundation::Expected<render_adapter::SourceMaterial, std::string> Material(
	    const std::string &material ) override;
	std::size_t ArchiveCount() const { return m_Archives.size(); }

private:
	CatalogTextures() = default;

	adapters::platform::DiskByteStore m_Store;
	std::vector<std::unique_ptr<formats::VpkArchive>> m_Archives;
	formats::SearchPathAssets m_Assets;
	std::unique_ptr<formats::MaterialCatalog> m_Catalog;
};

} // namespace hammer::gtk

#endif // HAMMER_GTK_CATALOG_TEXTURES_H
