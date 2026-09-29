//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The preview's source of studio models (RFC 0002
//			hammer.adapters.render; the R17 follow-up, props and instances),
//			next to IMaterialTextures. ViewportRenderer asks it for a model
//			the first time the scene names the model's path, on the render
//			sequence only, and keeps the answer (a failure included) for its
//			lifetime. The composition root implements it over the game's
//			assets (the GTK shell: hammer::gtk::CatalogModels over the mounted
//			VPKs, reading through content.studio-model), so nothing here is
//			shared with the host's sequence.
//
//			Contract: Model(path) takes the entity's "model" value as
//			authored (any case, either slash). It returns the parsed model
//			(mdl::LoadModel, body 0) with its textures resolved
//			(mdl::ResolveMaterials) against the same assets, or the reader's
//			error; a model whose files are absent is MissingFile.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_MODEL_SOURCE_H
#define HAMMER_ADAPTERS_RENDER_MODEL_SOURCE_H

#include "foundation/expected.h"
#include "mdl/studio_model.h"
#include "scene_geometry.h"

#include <string>

namespace hammer::render_adapter
{

class IModelSource
{
public:
	virtual ~IModelSource() = default;
	virtual foundation::Expected<ModelAsset, mdl::ModelError> Model( const std::string &path ) = 0;
};

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_MODEL_SOURCE_H
