//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.scene.v1, the scene service (RFC 0016). The engine, game,
//			Hammer and previews own scenes; each is independent, so two scenes
//			render in one process.
//
//			- Reserve() hands out ids; Commit() applies a change set atomically
//			  (an unknown or duplicate id fails it, and nothing applies) and
//			  publishes a new snapshot with the next revision.
//			- Snapshot() returns the latest published snapshot. A snapshot
//			  stays valid and unchanged for as long as a holder keeps it:
//			  shared ownership is real here, since a frame in flight holds
//			  revision n while the owner commits n+1. Publication is under a
//			  mutex, so a snapshot's contents happen-before any reader sees it
//			  (the render.scene.publication TSan lane, with a seeded
//			  unsynchronized publication as its negative control).
//			- Revision() may be read from any thread: it is stored with release
//			  after the snapshot is published, so a snapshot taken after reading
//			  revision n has revision n or later.
//			- Commit and Reserve belong to the owner's sequence.
//
//=============================================================================//

#ifndef RENDER_SCENE_SCENE_H
#define RENDER_SCENE_SCENE_H

#include "foundation/expected.h"
#include "render/scene/change_set.h"
#include "render/scene/draw_list.h"
#include "render/scene/snapshot.h"

#include <cstdint>
#include <memory>

namespace render::scene
{

enum class SceneStatus : std::uint8_t
{
	kUnknownInstance = 1,
	kDuplicateInstance,
	kInvalidId
};

struct SceneError
{
	SceneStatus status = SceneStatus::kUnknownInstance;
	std::uint32_t change = 0; // index of the offending change
};

class IRenderScene
{
public:
	virtual ~IRenderScene() = default;

	virtual InstanceId Reserve() = 0;
	virtual foundation::Expected<std::uint64_t, SceneError> Commit( const ChangeSet &changes ) = 0;
	virtual std::shared_ptr<const SceneSnapshot> Snapshot() const = 0;
	virtual std::uint64_t Revision() const = 0;
};

std::unique_ptr<IRenderScene> CreateRenderScene();

} // namespace render::scene

#endif // RENDER_SCENE_SCENE_H
