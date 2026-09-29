//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The preview's model source for the GTK shell (RFC 0002
//			hammer.adapters.gtk; R17 follow-up, props and instances): a
//			hammer::render_adapter::IModelSource over the game's VPKs, reading
//			through content.studio-model (mdl::LoadModel, body 0) and
//			resolving each texture against the same archives
//			(mdl::ResolveMaterials). Like CatalogTextures it mounts its own
//			archives and search path, so the render sequence that owns it
//			shares nothing with the window.
//
//=============================================================================//

#ifndef HAMMER_GTK_CATALOG_MODELS_H
#define HAMMER_GTK_CATALOG_MODELS_H

#include "hammer/adapters/platform/disk_byte_store.h"
#include "hammer/adapters/render/model_source.h"
#include "hammer/formats/search_path_assets.h"
#include "hammer/formats/vpk_archive.h"

#include <memory>
#include <string>
#include <vector>

namespace hammer::gtk
{

// The instance search roots for a mounted game (vbsp's instance path): for
// each _dir.vpk path, the "sdk_content/maps" directory beside its game
// directory when it exists (Portal 2's authoring tools: <game>/portal2/
// pak01_dir.vpk -> <game>/sdk_content/maps), then each directory of the
// comma-separated 'extra' list (the --instances option), without repeats.
std::vector<std::string> GameInstanceRoots( const std::string &vpkList, const std::string &extra );

class CatalogModels final : public render_adapter::IModelSource
{
public:
	// Mounts a comma-separated list of _dir.vpk paths. Nothing when none
	// mounts; 'errors' names each path that failed.
	static std::unique_ptr<CatalogModels> Open( const std::string &vpkList, std::string &errors );

	foundation::Expected<render_adapter::ModelAsset, mdl::ModelError> Model(
	    const std::string &path ) override;

private:
	// The search path as the model reader's file port.
	class Files final : public mdl::IModelFiles
	{
	public:
		explicit Files( const formats::SearchPathAssets &assets ) : m_Assets( assets ) {}
		bool Exists( const std::string &path ) const override { return m_Assets.HasAsset( path ); }
		bool Read( const std::string &path, std::string &out ) const override
		{
			return m_Assets.ReadAsset( path, out );
		}

	private:
		const formats::SearchPathAssets &m_Assets;
	};

	CatalogModels() = default;

	adapters::platform::DiskByteStore m_Store;
	std::vector<std::unique_ptr<formats::VpkArchive>> m_Archives;
	formats::SearchPathAssets m_Assets;
	Files m_Files{ m_Assets };
};

} // namespace hammer::gtk

#endif // HAMMER_GTK_CATALOG_MODELS_H
