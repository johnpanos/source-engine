//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The entity inspector presentation model (RFC 0002,
//			hammer.presenters; "Selection and property editing"). It shows the
//			entities the selection stands for (ops::EntitiesOf) the way legacy
//			Hammer's Object Properties sheet does (ObjectPage/op_entity SmartEdit
//			keys, op_flags, op_output, op_input), with no toolkit: a widget binds
//			to Rows()/Flags()/Outputs()/Inputs() and refreshes when Revision()
//			moves.
//
//			Rows (SmartEdit on): the schema keys in catalog order, then the keys
//			the entities carry that the schema lacks, in first-seen order. The
//			schema is the class's resolved keys when every entity has one class;
//			with mixed classes it is the keys every class declares (in the first
//			class's order); with an unknown class it is empty. Flags-typed keys
//			("spawnflags") are shown in the Flags section instead of as rows.
//			SmartEdit off: every key the entities carry, in first-seen order,
//			raw (display name = key), flags key included.
//
//			Values: each row aggregates the entities' values with
//			app::PropertyValue. A key present on some entities and absent on
//			others is Mixed (the shown value would be wrong for some); absent
//			on all is Unset; "" is a Single value distinct from Unset.
//			differsFromDefault: Single and != the schema default, or Mixed, or
//			any value on a key without a schema default row.
//
//			World: when the selection stands for no entity (nothing selected, or
//			only world brushes or faces, as legacy Hammer shows the worldspawn
//			for world brushes) the inspector shows the worldspawn: IsWorld(),
//			class "worldspawn", its schema when the catalog has one, its keys
//			from DocumentSettings::worldKeys, and no flags, outputs or inputs.
//			Drafts commit with ops::SetWorldKey ("Edit world properties");
//			class, flag, key-removal and output operations are refused.
//
//			Draft policy (the UI owns the draft until commit; RFC 0002):
//			  * SetDraft stores text per key without validating it; the row
//			    shows the draft and its validation message (DraftError).
//			  * Commit() validates every drafted key; any invalid key refuses the
//			    whole commit (EditErrorCode::Rejected, message names each key)
//			    and changes nothing. A valid draft is applied to the entities it
//			    was started on as ONE Execute labeled "Edit properties": a
//			    "targetname" draft through ops::RenameEntity with reference
//			    updates (catalog-aware; an empty name removes the key), every
//			    other key through ops::SetKey. The draft then clears.
//			  * Cancel() discards the draft.
//			  * Selection changes and document replacement (every entry point:
//			    pointer, keyboard, menu, script) pass the inspector's selection
//			    guard first:
//			      - no draft: allowed;
//			      - invalid draft: vetoed (the session returns Vetoed), the draft
//			        is kept and LastError() says which key is invalid and why;
//			      - valid draft: committed inside the guard (its own undo step,
//			        recorded before the selection changes), then allowed; a
//			        commit the session refuses vetoes the change and is reported.
//			  * A draft whose entities change without passing the guard (undo,
//			    redo, or an edit that selects its result) is discarded and
//			    reported in LastError(): it would otherwise apply to objects the
//			    user did not draft against.
//
//			Validation (ValidateKeyValue, one owner): no value may contain '"'
//			or a line break (a VMF cannot store them). By KeyType:
//			  Integer: optional sign and digits; Float: a finite number;
//			  Boolean: "0" or "1"; Flags: a non-negative integer;
//			  Choices: one of the choice values exactly, unless free text is
//			    allowed (SetAllowFreeChoices; off by default, as legacy's
//			    SmartEdit choice list);
//			  Color255: 3 or 4 numbers, the first three integers in 0..255 (or
//			    all three -1, the HDR "same as LDR" value);
//			  Color1: 3 or 4 finite numbers;
//			  Angle, Vector: exactly 3 finite numbers;
//			  SideList: space-separated non-negative integers;
//			  every other type (strings, targets, assets): any text.
//			Numeric types refuse the empty string. Keys without a schema
//			definition take any text.
//
//			Class choices (ClassChoices; legacy COP_Entity::LoadClassList, and
//			the kind rule ops::SetClass enforces): the catalog's classes a
//			class change may pick, sorted as the catalog lists them, without
//			"worldspawn": solid classes when every inspected entity is a brush
//			entity (owns solids), point and point-like classes when none is,
//			and none when the selection mixes both or shows the world. A
//			Single class the list lacks (unknown, or no catalog) is put first,
//			so a view can always show it.
//
//			Colours (ColorOfRow, ValueWithColor; legacy's colour picker for
//			color255/color1 keys): the red, green and blue a picker shows and
//			the value text a picked colour becomes. Components after the
//			third (brightness) are kept.
//
//			Settle(), for a host closing its view: a valid draft is committed
//			(as Commit), an invalid one is discarded and reported (Rejected,
//			LastError names each key), no draft does nothing. Closing a view
//			must not leave a draft that would veto later selection changes.
//
//			Immediate operations (one undo step each, never drafted): class
//			change (ops::SetClass), flag toggles (ops::SetSpawnFlag), key
//			removal and output add/replace/remove (ops::RemoveConnectionAt). A
//			pending draft survives them. A refused operation changes nothing and sets LastError().
//
//			Lifetime: construct with a session that outlives every call; the
//			subscriptions are RAII, so the inspector may be destroyed before or
//			after its session.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_ENTITY_INSPECTOR_H
#define HAMMER_PRESENTERS_ENTITY_INSPECTOR_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/property_value.h"
#include "hammer/ports/entity_catalog.h"
#include "hammer/scene/map_objects.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace hammer::presenters
{

enum class FlagState
{
	Off,
	On,
	Mixed,
};

struct KeyRow
{
	std::string key;
	std::string displayName; // schema display name (SmartEdit), else the key
	std::string help;
	ports::KeyType type = ports::KeyType::String;
	std::string typeName;
	std::vector<ports::KeyChoice> choices;
	bool readOnly = false;
	bool inSchema = false;
	std::string defaultValue;
	app::PropertyValue value;     // aggregate over the entities
	std::size_t presentCount = 0; // entities that carry the key
	bool differsFromDefault = false;
	std::string valueLabel; // Choices: the label of a Single value, else the value
	bool drafted = false;
	std::string draftValue;
	std::string draftError; // empty when the draft is valid
};

struct FlagRow
{
	long long bit = 0;
	std::string label;
	bool defaultOn = false;
	FlagState state = FlagState::Off;
};

struct OutputRow
{
	scene::ObjectId entity; // the selected entity that owns the connection
	std::size_t index = 0;  // its position in the entity's connections
	std::string entityLabel;
	scene::Connection connection;
	bool targetValid = false; // procedural, or names at least one entity
	bool outputKnown = true;  // false only when the class is known and lacks it
};

struct InputRow
{
	scene::ObjectId source; // the entity that owns the connection
	std::size_t index = 0;
	std::string sourceLabel;
	scene::ObjectId target; // the selected entity it names
	std::string targetLabel;
	scene::Connection connection;
	bool inputKnown = true; // false only when the target's class is known and lacks it
};

// A colour in 0..1 per channel.
struct KeyColor
{
	double r = 0.0;
	double g = 0.0;
	double b = 0.0;
};

// The colour a Color255 or Color1 row shows: its draft, else its Single
// value, else (Unset) its default. Nothing for other types, for a Mixed row
// without a draft, and for text that does not hold three numbers.
std::optional<KeyColor> ColorOfRow( const KeyRow &row );
// The value text for 'row' with its colour replaced by 'color' (clamped to
// 0..1): Color255 as integers 0..255, Color1 as numbers of up to four
// decimals. Components after the third are kept from the row's draft, Single
// value or default, in that order of preference; "" for other types.
std::string ValueWithColor( const KeyRow &row, const KeyColor &color );

// Why 'value' is not acceptable for a key of definition 'definition' (nullptr:
// no schema), or nothing when it is. See the header comment for the rules.
std::optional<std::string> ValidateKeyValue(
    const ports::KeyDefinition *definition, const std::string &value, bool allowFreeChoices );

class EntityInspector
{
public:
	using Result = foundation::Expected<void, app::EditError>;

	EntityInspector( app::EditSession &session, const ports::IEntityCatalog *catalog );
	~EntityInspector();
	EntityInspector( const EntityInspector & ) = delete;
	EntityInspector &operator=( const EntityInspector & ) = delete;

	// Moves whenever anything a widget shows changed.
	std::uint64_t Revision() const { return m_revision; }

	const std::vector<scene::ObjectId> &Entities() const { return m_entities; }
	// True when the worldspawn is shown (see "World").
	bool IsWorld() const { return m_world; }
	const app::PropertyValue &Class() const { return m_class; }
	// The class schema when Class() is Single and known, else nullptr.
	const ports::EntityClassInfo *ClassInfo() const { return m_classInfo; }
	const std::vector<KeyRow> &Rows() const { return m_rows; }
	// The Flags-typed schema key ("spawnflags"), or empty when there is none.
	const std::string &FlagsKey() const { return m_flagsKey; }
	const std::vector<FlagRow> &Flags() const { return m_flags; }
	const std::vector<OutputRow> &Outputs() const { return m_outputs; }
	const std::vector<InputRow> &Inputs() const { return m_inputs; }
	// The classes a class change may pick (see "Class choices").
	const std::vector<std::string> &ClassChoices() const { return m_classChoices; }

	bool SmartEdit() const { return m_smartEdit; }
	void SetSmartEdit( bool on );
	bool AllowFreeChoices() const { return m_allowFreeChoices; }
	void SetAllowFreeChoices( bool allow );

	// --- Draft ------------------------------------------------------------------
	// Refuses (Rejected): a reserved key ("classname", "id"), an empty key or
	// one with '"' or a line break, a read-only key. With no entity inspected
	// the draft edits the worldspawn.
	Result SetDraft( const std::string &key, const std::string &value );
	void ClearDraft( const std::string &key );
	bool HasDraft() const { return !m_draft.empty(); }
	const std::vector<std::pair<std::string, std::string>> &Draft() const { return m_draft; }
	// One "key: reason" line per invalid drafted key.
	std::vector<std::string> DraftErrors() const;
	Result Commit();
	void Cancel();
	// For a host closing its view (see "Settle").
	Result Settle();

	// --- Immediate operations -----------------------------------------------------
	Result SetClass( const std::string &classname );
	Result SetFlag( long long bit, bool on );
	Result RemoveKey( const std::string &key );
	// Adds 'connection' to every selected entity.
	Result AddOutput( const scene::Connection &connection );
	Result ReplaceOutput(
	    scene::ObjectId entity, std::size_t index, const scene::Connection &connection );
	// Removes connection 'index' of 'entity' (exactly that one).
	Result RemoveOutput( scene::ObjectId entity, std::size_t index );

	// The last refusal, validation failure or discarded draft, for the host.
	const std::string &LastError() const { return m_lastError; }
	void ClearError();

private:
	void OnEvent( const app::SessionEvent &event );
	bool OnGuard();
	void Rebuild();
	void Bump() { ++m_revision; }
	Result Fail( app::EditError error );
	Result Run( const std::string &label, const app::EditSession::Operation &operation );
	const ports::KeyDefinition *DefinitionOf( const std::string &key ) const;
	std::optional<std::string> DraftProblem() const;
	Result ApplyDraft();
	Result RefuseInWorld();

	app::EditSession &m_session;
	const ports::IEntityCatalog *m_catalog = nullptr;
	app::SessionSubscription m_subscription;
	app::SessionSubscription m_guard;

	std::uint64_t m_revision = 0;
	bool m_smartEdit = true;
	bool m_allowFreeChoices = false;
	bool m_world = false; // no entity inspected: the worldspawn is shown
	std::vector<scene::ObjectId> m_entities;
	app::PropertyValue m_class;
	const ports::EntityClassInfo *m_classInfo = nullptr;
	std::vector<const ports::KeyDefinition *> m_schema;
	std::vector<KeyRow> m_rows;
	std::string m_flagsKey;
	std::vector<FlagRow> m_flags;
	std::vector<OutputRow> m_outputs;
	std::vector<InputRow> m_inputs;
	std::vector<std::string> m_classChoices;
	// The catalog's classes for the last kind listed (see Rebuild), so the
	// catalog is walked again only when the kind changes.
	std::vector<std::string> m_kindClasses;
	int m_kindListed = -1;

	std::vector<std::pair<std::string, std::string>> m_draft;
	std::vector<scene::ObjectId> m_draftTargets;
	bool m_draftWorld = false;
	std::string m_lastError;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_ENTITY_INSPECTOR_H
