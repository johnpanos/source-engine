//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Staged document edits and their recorded change sets (RFC 0002,
//			hammer.scene; "Edit transactions and history").
//
//			DocumentEdit stages changes against a borrowed base document without
//			touching it: the first write to an object copies it into the edit,
//			reads see staged state, and removals hide objects. Finish() turns
//			the staged state into a ChangeSet of per-object before/after values,
//			dropping every change whose after equals its before, so an edit that
//			ends where it started records nothing.
//
//			A ChangeSet is a lossless patch: Apply(Forward) installs every
//			'after', Apply(Backward) every 'before'. Undo therefore restores
//			recorded values and never inverts geometry numerically.
//
//			Operations are written against DocumentEdit alone, so they are pure
//			functions over values and are unit-tested without a session.
//
//=============================================================================//

#ifndef HAMMER_SCENE_CHANGE_SET_H
#define HAMMER_SCENE_CHANGE_SET_H

#include "hammer/scene/map_document.h"

#include <map>
#include <optional>
#include <variant>
#include <vector>

namespace hammer::scene
{

using MapObject = std::variant<Solid, Entity, Group>;

ObjectId IdOf( const MapObject &object );
ObjectKind KindOf( const MapObject &object );

struct ObjectChange
{
	ObjectId id;
	std::optional<MapObject> before; // nothing = the object was created
	std::optional<MapObject> after;  // nothing = the object was removed
};

struct ChangeSet
{
	std::vector<ObjectChange> objects; // in id order
	std::optional<DocumentSettings> settingsBefore;
	std::optional<DocumentSettings> settingsAfter;

	bool Empty() const { return objects.empty() && !settingsAfter; }

	std::vector<ObjectId> Created() const;
	std::vector<ObjectId> Removed() const;
	std::vector<ObjectId> Modified() const;
};

enum class ApplyDirection
{
	Forward,  // install 'after' values (commit, redo)
	Backward, // install 'before' values (undo)
};

// Applies a change set. Returns false, leaving 'doc' unchanged, when an object
// id does not carry the document's serial.
bool Apply( MapDocument &doc, const ChangeSet &changes, ApplyDirection direction );

class DocumentEdit final : public DocumentReader
{
public:
	// Borrows 'base', which must outlive the edit and not change while it lives.
	explicit DocumentEdit( const MapDocument &base );

	const MapDocument &Base() const { return m_base; }

	// --- Reads through the staged state ---------------------------------------
	const Solid *FindSolid( ObjectId id ) const override;
	const Entity *FindEntity( ObjectId id ) const override;
	const Group *FindGroup( ObjectId id ) const override;
	std::optional<ObjectKind> KindOf( ObjectId id ) const override;
	const DocumentSettings &Settings() const override;

	// Every live id of a kind, in id order (base ids plus created, minus removed).
	std::vector<ObjectId> SolidIds() const override;
	std::vector<ObjectId> EntityIds() const override;
	std::vector<ObjectId> GroupIds() const override;

	// --- Writes -----------------------------------------------------------------
	// Copy-on-write access; nullptr when the id is not a live object of that kind.
	Solid *MutableSolid( ObjectId id );
	Entity *MutableEntity( ObjectId id );
	Group *MutableGroup( ObjectId id );
	DocumentSettings &MutableSettings();

	// Adds a new object. Its id is allocated here (any id in the value is
	// replaced) and returned; persistent VMF ids of zero are allocated too
	// (the solid, each of its sides, the entity or the group).
	ObjectId Add( Solid solid );
	ObjectId Add( Entity entity );
	ObjectId Add( Group group );

	// Installs an object under its existing id (a restore or a replacement). A
	// value whose id is new to the document is treated as a creation.
	void Put( MapObject object );

	// Removes a live object. Returns false when it is not live.
	bool Remove( ObjectId id );

	// A fresh persistent VMF id (for new sides of an existing solid).
	std::uint32_t AllocateVmfId();
	ObjectId AllocateId();

	// Every live object's id, in id order.
	std::vector<ObjectId> AllIds() const;

	// Ids this edit created, modified or removed so far, in id order.
	std::vector<ObjectId> TouchedIds() const;

	// Builds the change set. The edit stays usable (a second Finish() returns
	// the same changes).
	ChangeSet Finish() const;

	// Id counters after this edit's allocations, for the committing session.
	std::uint32_t NextLocal() const { return m_nextLocal; }
	std::uint32_t NextVmfId() const { return m_nextVmfId; }

private:
	template <typename T> const T *FindStaged( ObjectId id ) const;
	const MapObject *StagedObject( ObjectId id ) const;
	std::vector<ObjectId> IdsOfKind( ObjectKind kind ) const;

	const MapDocument &m_base;
	// Staged objects: a value = created or modified, nothing = removed.
	std::map<ObjectId, std::optional<MapObject>> m_staged;
	std::optional<DocumentSettings> m_settings;
	std::uint32_t m_nextLocal = 1;
	std::uint32_t m_nextVmfId = 1;
};

// The document invariants (MapDocument::Validate) checked for what an edit
// touches: every created or modified object's references, side count, unique
// side ids and closed geometry, group cycles, and live objects left referring
// to removed ones. Empty when the edit may be committed.
std::vector<std::string> ValidateEdit( const DocumentEdit &edit );

// Commits 'edit' into 'doc' (which must be the edit's base): applies the change
// set and advances the document's id counters past the edit's allocations,
// even when the change set is empty. Returns the change set.
ChangeSet CommitEdit( MapDocument &doc, const DocumentEdit &edit );

} // namespace hammer::scene

#endif // HAMMER_SCENE_CHANGE_SET_H
