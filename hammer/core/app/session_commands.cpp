//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/session_commands.h. One table
//			owns every command's name, arguments, summary and handler, so
//			dispatch and the catalog cannot disagree.
//
//=============================================================================//

#include "hammer/app/session_commands.h"

#include "hammer/app/document_io.h"
#include "hammer/app/map_check.h"
#include "hammer/app/ops/cordon_ops.h"
#include "hammer/app/ops/create_ops.h"
#include "hammer/app/ops/displacement_ops.h"
#include "hammer/app/ops/csg_ops.h"
#include "hammer/app/ops/entity_ops.h"
#include "hammer/app/ops/structure_ops.h"
#include "hammer/app/ops/texture_ops.h"
#include "hammer/app/ops/transform_ops.h"
#include "hammer/app/ops/vertex_ops.h"
#include "hammer/app/ops/visgroup_ops.h"
#include "hammer/scene/map_queries.h"
#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <sstream>

namespace hammer::app
{

using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

std::string FormatVector( const Vec3d &v )
{
	return scene::FormatVec3( v );
}

std::string IdList( const std::vector<ObjectId> &ids )
{
	std::string out;
	for ( std::size_t i = 0; i < ids.size(); ++i )
	{
		out += ( i ? " " : "" ) + std::to_string( SessionCommands::ScriptId( ids[i] ) );
	}
	return out;
}

struct Context
{
	EditSession &session;
	EditorSettings &settings;
	const SessionServices &services;
	const CommandArgs &args;
	std::string_view name;

	bool Has( const char *key ) const { return args.find( key ) != args.end(); }
	const std::string &Arg( const char *key ) const { return args.at( key ); }

	foundation::Unexpected<CommandError> Fail( CommandStatus status, std::string detail ) const
	{
		return CommandFailure( status, name, std::move( detail ) );
	}

	foundation::Expected<double, CommandError> Number( const char *key ) const
	{
		if ( auto v = ParseCommandNumber( Arg( key ) ) )
			return *v;
		return Fail( CommandStatus::InvalidArgument,
		    std::string( key ) + " is not a number: " + Arg( key ) );
	}

	foundation::Expected<double, CommandError> NumberOr( const char *key, double fallback ) const
	{
		return Has( key ) ? Number( key ) : foundation::Expected<double, CommandError>( fallback );
	}

	foundation::Expected<int, CommandError> Integer( const char *key ) const
	{
		const auto v = ParseCommandNumber( Arg( key ) );
		if ( !v || *v != std::floor( *v ) || std::fabs( *v ) > 2147483647.0 )
			return Fail( CommandStatus::InvalidArgument,
			    std::string( key ) + " is not an integer: " + Arg( key ) );
		return static_cast<int>( *v );
	}

	foundation::Expected<bool, CommandError> Flag( const char *key, bool fallback ) const
	{
		if ( !Has( key ) )
			return fallback;
		if ( Arg( key ) == "1" )
			return true;
		if ( Arg( key ) == "0" )
			return false;
		return Fail( CommandStatus::InvalidArgument, std::string( key ) + " is 0 or 1" );
	}

	foundation::Expected<Vec3d, CommandError> Vector( const char *key ) const
	{
		if ( auto v = ParseCommandVector( Arg( key ) ) )
			return *v;
		return Fail( CommandStatus::InvalidArgument,
		    std::string( key ) + " is not an \"x y z\" vector: " + Arg( key ) );
	}

	ObjectId Resolve( std::uint32_t scriptId ) const
	{
		ObjectId id;
		id.value = ( static_cast<std::uint64_t>( session.Document().Serial() ) << 32 ) | scriptId;
		return id;
	}

	// An id list argument; the selection when absent and 'selectionDefault'.
	foundation::Expected<std::vector<ObjectId>, CommandError> Ids(
	    const char *key = "ids", bool selectionDefault = true ) const
	{
		if ( !Has( key ) )
		{
			if ( selectionDefault )
				return session.CurrentSelection().objects;
			return Fail( CommandStatus::MissingArgument, std::string( "missing " ) + key );
		}
		std::vector<ObjectId> out;
		std::istringstream in( Arg( key ) );
		std::string token;
		while ( in >> token )
		{
			const auto v = ParseCommandNumber( token );
			if ( !v || *v < 1 || *v != std::floor( *v ) || *v > 4294967295.0 )
				return Fail( CommandStatus::InvalidArgument, "not an id: " + token );
			const ObjectId id = Resolve( static_cast<std::uint32_t>( *v ) );
			if ( !session.Document().KindOf( id ) )
				return Fail( CommandStatus::Rejected, "no object with id " + token );
			out.push_back( id );
		}
		return out;
	}

	foundation::Expected<ObjectId, CommandError> Id( const char *key ) const
	{
		auto ids = Ids( key, false );
		if ( !ids )
			return foundation::MakeUnexpected( ids.Error() );
		if ( ids.Value().size() != 1 )
			return Fail( CommandStatus::InvalidArgument, std::string( key ) + " names one object" );
		return ids.Value().front();
	}

	// "solid:side" pairs; the selected faces when absent.
	foundation::Expected<std::vector<scene::FaceRef>, CommandError> Faces(
	    const char *key = "faces" ) const
	{
		if ( !Has( key ) )
			return session.CurrentSelection().faces;
		std::vector<scene::FaceRef> out;
		std::istringstream in( Arg( key ) );
		std::string token;
		while ( in >> token )
		{
			const std::size_t colon = token.find( ':' );
			const auto solid = colon == std::string::npos
			                       ? std::nullopt
			                       : ParseCommandNumber( token.substr( 0, colon ) );
			const auto side = colon == std::string::npos
			                      ? std::nullopt
			                      : ParseCommandNumber( token.substr( colon + 1 ) );
			if ( !solid || !side || *solid < 1 || *side < 1 )
				return Fail( CommandStatus::InvalidArgument, "not a solid:side face: " + token );
			out.push_back( { Resolve( static_cast<std::uint32_t>( *solid ) ),
			    static_cast<std::uint32_t>( *side ) } );
		}
		return out;
	}

	foundation::Expected<scene::Box, CommandError> Box() const
	{
		auto mins = Vector( "mins" );
		if ( !mins )
			return foundation::MakeUnexpected( mins.Error() );
		auto maxs = Vector( "maxs" );
		if ( !maxs )
			return foundation::MakeUnexpected( maxs.Error() );
		return scene::Box{ mins.Value(), maxs.Value() };
	}

	ops::TransformOptions Transform() const
	{
		ops::TransformOptions o;
		o.textureLock = settings.textureLock;
		return o;
	}

	CommandResult Committed(
	    const foundation::Expected<CommitInfo, EditError> &result, std::string output = {} ) const
	{
		if ( !result )
			return Fail(
			    CommandStatus::Rejected, std::string( EditErrorName( result.Error().code ) ) +
			                                 ": " + result.Error().message );
		return output;
	}

	// Runs one operation as one undo step and reports created ids.
	CommandResult Edit( const std::string &label, const EditSession::Operation &operation ) const
	{
		auto result = session.Execute( label, operation );
		if ( !result )
			return Committed( result );
		return IdList( result.Value().created );
	}

	// Runs an operation that also selects its result.
	CommandResult EditSelecting(
	    const std::string &label, const EditSession::SelectingOperation &operation ) const
	{
		auto result = session.ExecuteSelecting( label, operation );
		if ( !result )
			return Committed( result );
		return IdList( result.Value().created );
	}

	CommandResult Selected( const foundation::Expected<void, EditError> &result ) const
	{
		if ( !result )
			return Fail( CommandStatus::Rejected, result.Error().message );
		return std::string();
	}

	foundation::Expected<SelectMode, CommandError> Mode() const
	{
		if ( !Has( "mode" ) || Arg( "mode" ) == "replace" )
			return SelectMode::Replace;
		if ( Arg( "mode" ) == "add" )
			return SelectMode::Add;
		if ( Arg( "mode" ) == "toggle" )
			return SelectMode::Toggle;
		if ( Arg( "mode" ) == "remove" )
			return SelectMode::Remove;
		return Fail( CommandStatus::InvalidArgument, "mode is replace, add, toggle or remove" );
	}
};

#define TRY( var, expr )                                                                           \
	auto var = ( expr );                                                                           \
	if ( !var )                                                                                    \
		return foundation::MakeUnexpected( var.Error() );

struct CommandEntry
{
	CommandInfo info;
	std::function<CommandResult( const Context & )> run;
};

Selection SelectOnly( const std::vector<ObjectId> &ids )
{
	return CombineObjects( {}, ids, SelectMode::Replace );
}

std::string Describe( const scene::MapDocument &doc, ObjectId id )
{
	const std::string sid = std::to_string( SessionCommands::ScriptId( id ) );
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		std::string out = "solid id=" + sid + " vmfid=" + std::to_string( s->vmfId ) +
		                  " sides=" + std::to_string( s->sides.size() );
		if ( const std::optional<scene::Box> b = scene::SolidBounds( *s ) )
			out += " mins=\"" + FormatVector( b->mins ) + "\" maxs=\"" + FormatVector( b->maxs ) +
			       "\"";
		if ( s->owner.IsValid() )
			out += " owner=" + std::to_string( SessionCommands::ScriptId( s->owner ) );
		if ( !s->sides.empty() )
			out += " material=" + s->sides.front().texture.material;
		return out;
	}
	if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		std::string out = "entity id=" + sid + " classname=" + e->classname;
		for ( const kvtext::KeyValue &kv : e->keys )
			out += " " + kv.key + "=\"" + kv.value + "\"";
		out += " outputs=" + std::to_string( e->connections.size() );
		return out;
	}
	if ( doc.FindGroup( id ) )
		return "group id=" + sid + " members=" + IdList( scene::GroupMembers( doc, id ) );
	return {};
}

const std::vector<CommandEntry> &Table()
{
	static const std::vector<CommandEntry> table = {
	    // --- Document -------------------------------------------------------
	    { { "new_map", {}, {}, "Start an empty map (clears history)." },
	        []( const Context &c ) -> CommandResult
	        {
		        scene::MapDocument doc( c.session.Document().Serial() + 1 );
		        doc.MutableSettings().SetWorldKey( "skyname", "sky_day01_01" );
		        return c.Selected( c.session.Replace( std::move( doc ) ) );
	        } },
	    { { "open", { "path" }, {},
	          "Load a map through the codec; outputs warnings, one per line." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( !c.services.codec || !c.services.store )
			        return c.Fail(
			            CommandStatus::Rejected, "no codec or file store in this composition" );
		        auto opened = OpenDocument(
		            c.session, *c.services.codec, *c.services.store, c.Arg( "path" ) );
		        if ( !opened )
			        return c.Fail( opened.Error().status == DocumentIoStatus::ReadFailed
			                           ? CommandStatus::IoFailure
			                           : CommandStatus::Rejected,
			            opened.Error().message );
		        std::string out;
		        for ( const ports::CodecDiagnostic &w : opened.Value() )
			        out += ( out.empty() ? "" : "\n" ) + std::string( "line " ) +
			               std::to_string( w.line ) + ": " + w.message;
		        return out;
	        } },
	    { { "save", { "path" }, { "bump_version" },
	          "Write the map atomically and mark it saved (bump_version=0 keeps mapversion)." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( !c.services.codec || !c.services.store )
			        return c.Fail(
			            CommandStatus::Rejected, "no codec or file store in this composition" );
		        TRY( bump, c.Flag( "bump_version", true ) );
		        auto saved = SaveDocumentAs( c.session, *c.services.codec, *c.services.store,
		            c.Arg( "path" ), bump.Value() );
		        if ( !saved )
			        return c.Fail( saved.Error().status == DocumentIoStatus::WriteFailed
			                           ? CommandStatus::IoFailure
			                           : CommandStatus::Rejected,
			            saved.Error().message );
		        return std::string();
	        } },
	    { { "info", {}, {}, "Counts, the modified flag, the revision and the selection size." },
	        []( const Context &c ) -> CommandResult
	        {
		        const scene::MapDocument &d = c.session.Document();
		        return "solids=" + std::to_string( d.Solids().size() ) +
		               " entities=" + std::to_string( d.Entities().size() ) +
		               " groups=" + std::to_string( d.Groups().size() ) +
		               " modified=" + ( c.session.IsModified() ? "1" : "0" ) +
		               " revision=" + std::to_string( c.session.Revision() ) +
		               " selected=" + std::to_string( c.session.CurrentSelection().objects.size() );
	        } },
	    { { "describe", { "id" }, {}, "Describe a solid, entity or group." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( id, c.Id( "id" ) );
		        return Describe( c.session.Document(), id.Value() );
	        } },
	    { { "undo", {}, {}, "Undo the last edit." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Selected( c.session.Undo() );
	        } },
	    { { "redo", {}, {}, "Redo the last undone edit." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Selected( c.session.Redo() );
	        } },
	    { { "history", {}, {},
	          "List the history: one line per entry, '*' marks the current position." },
	        []( const Context &c ) -> CommandResult
	        {
		        const ChangeHistory &h = c.session.History();
		        std::string out;
		        for ( std::size_t i = 0; i < h.Size(); ++i )
			        out += std::to_string( i + 1 ) + ( i + 1 == h.Position() ? "* " : "  " ) +
			               h.Entries()[i].label + "\n";
		        return out;
	        } },
	    { { "jump", { "position" }, {},
	          "Undo or redo to a history position (0 = before the first entry)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( p, c.Integer( "position" ) );
		        if ( p.Value() < 0 )
			        return c.Fail( CommandStatus::InvalidArgument, "position cannot be negative" );
		        return c.Selected( c.session.JumpTo( static_cast<std::size_t>( p.Value() ) ) );
	        } },

	    // --- Settings ---------------------------------------------------------
	    { { "set_material", { "material" }, {},
	          "Choose the material new geometry and apply_material use." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( c.Arg( "material" ).empty() )
			        return c.Fail( CommandStatus::InvalidArgument, "empty material" );
		        c.settings.faceTexture.material = c.Arg( "material" );
		        return std::string();
	        } },
	    { { "set_entity_class", { "classname" }, {}, "Choose the class the entity tool places." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( c.services.catalog && !c.services.catalog->Find( c.Arg( "classname" ) ) )
			        return c.Fail(
			            CommandStatus::Rejected, "unknown entity class " + c.Arg( "classname" ) );
		        c.settings.entityClass = c.Arg( "classname" );
		        return std::string();
	        } },
	    { { "set_grid", { "size" }, {}, "Set the grid size (a power of two from 1 to 1024)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( size, c.Integer( "size" ) );
		        const int s = size.Value();
		        if ( s < 1 || s > 1024 || ( s & ( s - 1 ) ) != 0 )
			        return c.Fail( CommandStatus::InvalidArgument,
			            "grid size is a power of two from 1 to 1024" );
		        c.settings.gridSize = s;
		        return std::string();
	        } },
	    { { "set_snap", { "on" }, {}, "Grid snapping for tools (0 or 1)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( on, c.Flag( "on", true ) );
		        c.settings.snapToGrid = on.Value();
		        return std::string();
	        } },
	    { { "set_texture_lock", { "on" }, {}, "Texture lock for transforms (0 or 1)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( on, c.Flag( "on", true ) );
		        c.settings.textureLock = on.Value();
		        return std::string();
	        } },
	    { { "set_granularity", { "mode" }, {}, "What a click selects: groups, objects or solids." },
	        []( const Context &c ) -> CommandResult
	        {
		        const std::string &m = c.Arg( "mode" );
		        if ( m == "groups" )
			        c.settings.granularity = SelectionGranularity::Groups;
		        else if ( m == "objects" )
			        c.settings.granularity = SelectionGranularity::Objects;
		        else if ( m == "solids" )
			        c.settings.granularity = SelectionGranularity::Solids;
		        else
			        return c.Fail(
			            CommandStatus::InvalidArgument, "mode is groups, objects or solids" );
		        return std::string();
	        } },

	    // --- Selection --------------------------------------------------------
	    { { "select", { "ids" }, { "mode" },
	          "Select objects (mode replace, add, toggle or remove)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids( "ids", false ) );
		        TRY( mode, c.Mode() );
		        return c.Selected( c.session.SelectObjects( ids.Value(), mode.Value() ) );
	        } },
	    { { "select_faces", { "faces" }, { "mode" }, "Select faces given as solid:side pairs." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( mode, c.Mode() );
		        for ( const scene::FaceRef &f : faces.Value() )
			        if ( !ops::FindFace( c.session.Document(), f ) )
				        return c.Fail( CommandStatus::Rejected, "no such face" );
		        return c.Selected( c.session.SelectFaces( faces.Value(), mode.Value() ) );
	        } },
	    { { "select_none", {}, {}, "Clear the selection." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Selected( c.session.ClearSelection() );
	        } },
	    { { "select_all", {}, {}, "Select every visible object at the current granularity." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Selected( c.session.SetSelection(
		            SelectAll( c.session.Document(), c.settings.granularity ) ) );
	        } },
	    { { "invert_selection", {}, {}, "Select every visible object not selected." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Selected( c.session.SetSelection( InvertSelection( c.session.Document(),
		            c.session.CurrentSelection(), c.settings.granularity ) ) );
	        } },
	    { { "select_class", { "classname" }, { "mode" },
	          "Select entities by class (trailing * matches a prefix)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( mode, c.Mode() );
		        return c.Selected( c.session.SelectObjects(
		            scene::FindEntitiesByClass( c.session.Document(), c.Arg( "classname" ) ),
		            mode.Value() ) );
	        } },
	    { { "select_name", { "name" }, { "mode" },
	          "Select entities by targetname (trailing * matches a prefix)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( mode, c.Mode() );
		        return c.Selected( c.session.SelectObjects(
		            scene::FindEntitiesByName( c.session.Document(), c.Arg( "name" ) ),
		            mode.Value() ) );
	        } },

	    // --- Creation ---------------------------------------------------------
	    { { "create_block", { "mins", "maxs" }, { "material" },
	          "Add a box solid and select it; outputs its id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( box, c.Box() );
		        scene::FaceTexture tex = c.settings.faceTexture;
		        if ( c.Has( "material" ) )
			        tex.material = c.Arg( "material" );
		        return c.EditSelecting( "Create block",
		            [&]( scene::DocumentEdit &edit, Selection &after ) -> EditResult
		            {
			            ObjectId id;
			            if ( EditResult r = ops::CreatePrimitive(
			                     edit, ops::PrimitiveSpec{}, box.Value(), tex, id );
			                !r )
				            return r;
			            after = SelectOnly( { id } );
			            return {};
		            } );
	        } },
	    { { "create_primitive", { "kind", "mins", "maxs" }, { "sides", "axis", "material" },
	          "Add a block, wedge, cylinder, spike or sphere; outputs its id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( box, c.Box() );
		        ops::PrimitiveSpec spec;
		        const std::string &k = c.Arg( "kind" );
		        if ( k == "block" )
			        spec.kind = ops::PrimitiveKind::Block;
		        else if ( k == "wedge" )
			        spec.kind = ops::PrimitiveKind::Wedge;
		        else if ( k == "cylinder" )
			        spec.kind = ops::PrimitiveKind::Cylinder;
		        else if ( k == "spike" )
			        spec.kind = ops::PrimitiveKind::Spike;
		        else if ( k == "sphere" )
			        spec.kind = ops::PrimitiveKind::Sphere;
		        else
			        return c.Fail( CommandStatus::InvalidArgument,
			            "kind is block, wedge, cylinder, spike or sphere" );
		        if ( c.Has( "sides" ) )
		        {
			        TRY( sides, c.Integer( "sides" ) );
			        spec.sides = sides.Value();
		        }
		        if ( c.Has( "axis" ) )
		        {
			        TRY( axis, c.Integer( "axis" ) );
			        spec.axis = axis.Value();
		        }
		        scene::FaceTexture tex = c.settings.faceTexture;
		        if ( c.Has( "material" ) )
			        tex.material = c.Arg( "material" );
		        return c.EditSelecting( "Create " + k,
		            [&]( scene::DocumentEdit &edit, Selection &after ) -> EditResult
		            {
			            ObjectId id;
			            if ( EditResult r =
			                     ops::CreatePrimitive( edit, spec, box.Value(), tex, id );
			                !r )
				            return r;
			            after = SelectOnly( { id } );
			            return {};
		            } );
	        } },
	    { { "create_arch", { "mins", "maxs" }, { "sides", "wall", "arc", "start", "add_height" },
	          "Add an arch (grouped segments); outputs the group id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( box, c.Box() );
		        ops::ArchSpec spec = c.settings.arch;
		        if ( c.Has( "sides" ) )
		        {
			        TRY( sides, c.Integer( "sides" ) );
			        spec.sides = sides.Value();
		        }
		        TRY( wall, c.NumberOr( "wall", spec.wallWidth ) );
		        TRY( arc, c.NumberOr( "arc", spec.arc ) );
		        TRY( start, c.NumberOr( "start", spec.startAngle ) );
		        TRY( add, c.NumberOr( "add_height", spec.addHeight ) );
		        spec.wallWidth = wall.Value();
		        spec.arc = arc.Value();
		        spec.startAngle = start.Value();
		        spec.addHeight = add.Value();
		        std::string output;
		        auto result = c.EditSelecting( "Create arch",
		            [&]( scene::DocumentEdit &edit, Selection &after ) -> EditResult
		            {
			            ObjectId group;
			            if ( EditResult r = ops::CreateArch(
			                     edit, spec, box.Value(), c.settings.faceTexture, group );
			                !r )
				            return r;
			            after = SelectOnly( { group } );
			            output = std::to_string( SessionCommands::ScriptId( group ) );
			            return {};
		            } );
		        if ( !result )
			        return result;
		        return output;
	        } },
	    { { "place_entity", { "classname", "origin" }, {},
	          "Place a point entity and select it; outputs its id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( origin, c.Vector( "origin" ) );
		        return c.EditSelecting( "Place " + c.Arg( "classname" ),
		            [&]( scene::DocumentEdit &edit, Selection &after ) -> EditResult
		            {
			            ObjectId id;
			            if ( EditResult r = ops::PlaceEntity( edit, c.Arg( "classname" ),
			                     origin.Value(), c.services.catalog, id );
			                !r )
				            return r;
			            after = SelectOnly( { id } );
			            return {};
		            } );
	        } },
	    { { "place_on_surface", { "classname", "origin", "dir" }, { "offset" },
	          "Place a point entity where a ray first enters a solid, offset along the face normal "
	          "(default 1 unit); outputs its id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( origin, c.Vector( "origin" ) );
		        TRY( dir, c.Vector( "dir" ) );
		        TRY( offset, c.NumberOr( "offset", 1.0 ) );
		        const Vec3d d = mapgeometry::Normalize( dir.Value() );
		        if ( d == Vec3d() )
			        return c.Fail( CommandStatus::InvalidArgument, "dir has no length" );
		        const scene::MapDocument &doc = c.session.Document();
		        std::optional<scene::SolidRayHit> best;
		        for ( const auto &[id, solid] : doc.Solids() )
		        {
			        if ( !scene::IsVisible( doc, id ) )
				        continue;
			        const auto hit = scene::RayEnterSolid( solid, origin.Value(), d );
			        if ( hit && hit->t > 0.0 && ( !best || hit->t < best->t ) )
				        best = hit;
		        }
		        if ( !best )
			        return c.Fail( CommandStatus::Rejected, "the ray hits no solid" );
		        const Vec3d at = best->point + best->normal * offset.Value();
		        return c.EditSelecting( "Place " + c.Arg( "classname" ),
		            [&]( scene::DocumentEdit &edit, Selection &after ) -> EditResult
		            {
			            ObjectId id;
			            if ( EditResult r = ops::PlaceEntity(
			                     edit, c.Arg( "classname" ), at, c.services.catalog, id );
			                !r )
				            return r;
			            after = SelectOnly( { id } );
			            return {};
		            } );
	        } },
	    { { "raycast", { "origin", "dir" }, {},
	          "The nearest solid face a ray enters: id, side, point and normal." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( origin, c.Vector( "origin" ) );
		        TRY( dir, c.Vector( "dir" ) );
		        const scene::MapDocument &doc = c.session.Document();
		        double best = 1e300;
		        std::string out;
		        const Vec3d d = mapgeometry::Normalize( dir.Value() );
		        if ( d == Vec3d() )
			        return c.Fail( CommandStatus::InvalidArgument, "dir has no length" );
		        for ( const auto &[id, solid] : doc.Solids() )
		        {
			        if ( !scene::IsVisible( doc, id ) )
				        continue;
			        const std::optional<scene::SolidRayHit> hit =
			            scene::RayEnterSolid( solid, origin.Value(), d );
			        if ( !hit || hit->t <= 0.0 || hit->t >= best )
				        continue;
			        best = hit->t;
			        out = "id=" + std::to_string( SessionCommands::ScriptId( id ) ) +
			              " side=" + std::to_string( solid.sides[hit->side].vmfId ) + " point=\"" +
			              FormatVector( hit->point ) + "\" normal=\"" +
			              FormatVector( hit->normal ) + "\"";
		        }
		        if ( out.empty() )
			        return c.Fail( CommandStatus::Rejected, "the ray hits no solid" );
		        return out;
	        } },

	    // --- Transforms -------------------------------------------------------
	    { { "move", { "delta" }, { "ids" }, "Translate objects (the selection by default)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( delta, c.Vector( "delta" ) );
		        TRY( ids, c.Ids() );
		        return c.Edit( "Move",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::Translate( e, ids.Value(), delta.Value(), c.Transform() );
		            } );
	        } },
	    { { "move_selection", { "delta" }, {}, "Translate the selection (alias of move)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( delta, c.Vector( "delta" ) );
		        const std::vector<ObjectId> ids = c.session.CurrentSelection().objects;
		        return c.Edit( "Move",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::Translate( e, ids, delta.Value(), c.Transform() );
		            } );
	        } },
	    { { "rotate", { "axis", "degrees" }, { "pivot", "ids" },
	          "Rotate about world axis 0/1/2 through pivot (default: the bounds center)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( axis, c.Integer( "axis" ) );
		        TRY( deg, c.Number( "degrees" ) );
		        TRY( ids, c.Ids() );
		        Vec3d pivot;
		        if ( c.Has( "pivot" ) )
		        {
			        TRY( p, c.Vector( "pivot" ) );
			        pivot = p.Value();
		        }
		        else if ( const auto b = scene::ObjectsBounds( c.session.Document(), ids.Value() ) )
		        {
			        pivot = b->Center();
		        }
		        return c.Edit( "Rotate",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::Rotate(
			                e, ids.Value(), axis.Value(), deg.Value(), pivot, c.Transform() );
		            } );
	        } },
	    { { "scale", { "mins", "maxs" }, { "ids" },
	          "Scale objects so their bounds become mins..maxs." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( box, c.Box() );
		        TRY( ids, c.Ids() );
		        const auto from = scene::ObjectsBounds( c.session.Document(), ids.Value() );
		        if ( !from )
			        return c.Fail( CommandStatus::Rejected, "nothing to scale" );
		        return c.Edit( "Scale",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::ScaleToBox( e, ids.Value(), *from, box.Value(), c.Transform() );
		            } );
	        } },
	    { { "mirror", { "axis" }, { "pivot", "ids" },
	          "Mirror across the plane normal to axis 0/1/2 (default through the bounds center)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( axis, c.Integer( "axis" ) );
		        TRY( ids, c.Ids() );
		        Vec3d pivot;
		        if ( c.Has( "pivot" ) )
		        {
			        TRY( p, c.Vector( "pivot" ) );
			        pivot = p.Value();
		        }
		        else if ( const auto b = scene::ObjectsBounds( c.session.Document(), ids.Value() ) )
		        {
			        pivot = b->Center();
		        }
		        return c.Edit( "Mirror",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::Mirror( e, ids.Value(), axis.Value(), pivot, c.Transform() );
		            } );
	        } },
	    { { "snap", {}, { "ids" }, "Snap the objects' bounds corner to the grid." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Snap to grid",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SnapToGrid(
			                e, ids.Value(), c.settings.gridSize, c.Transform() );
		            } );
	        } },
	    { { "align", { "axis", "edge" }, { "ids" },
	          "Align objects' min or max edge on axis 0/1/2." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( axis, c.Integer( "axis" ) );
		        TRY( ids, c.Ids() );
		        if ( c.Arg( "edge" ) != "min" && c.Arg( "edge" ) != "max" )
			        return c.Fail( CommandStatus::InvalidArgument, "edge is min or max" );
		        const bool toMax = c.Arg( "edge" ) == "max";
		        return c.Edit( "Align",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::AlignObjects(
			                e, ids.Value(), axis.Value(), toMax, c.Transform() );
		            } );
	        } },

	    // --- CSG and vertex editing ------------------------------------------
	    { { "clip", { "normal", "point" }, { "keep", "ids" },
	          "Clip solids by the plane through point with outward normal; keep front, back or "
	          "both." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( normal, c.Vector( "normal" ) );
		        TRY( point, c.Vector( "point" ) );
		        TRY( ids, c.Ids() );
		        ops::ClipKeep keep = ops::ClipKeep::Back;
		        if ( c.Has( "keep" ) )
		        {
			        const std::string &k = c.Arg( "keep" );
			        if ( k == "front" )
				        keep = ops::ClipKeep::Front;
			        else if ( k == "both" )
				        keep = ops::ClipKeep::Both;
			        else if ( k != "back" )
				        return c.Fail(
				            CommandStatus::InvalidArgument, "keep is front, back or both" );
		        }
		        const mapgeometry::Plane plane =
		            mapgeometry::PlaneThrough( point.Value(), normal.Value() );
		        return c.Edit( "Clip",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::ClipSolids(
			                e, ids.Value(), plane, keep, c.settings.faceTexture );
		            } );
	        } },
	    { { "carve", {}, { "ids", "targets" },
	          "Subtract the carvers (the selection by default) from targets (default: all "
	          "solids)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( carvers, c.Ids() );
		        std::vector<ObjectId> targets;
		        if ( c.Has( "targets" ) )
		        {
			        TRY( t, c.Ids( "targets", false ) );
			        targets = t.Value();
		        }
		        else
		        {
			        targets = c.session.Document().SolidIds();
		        }
		        return c.Edit( "Carve",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::Carve( e, carvers.Value(), targets );
		            } );
	        } },
	    { { "hollow", { "thickness" }, { "ids", "id" },
	          "Replace solids with walls of the given thickness (negative: outward)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( thickness, c.Number( "thickness" ) );
		        TRY( ids, c.Has( "id" ) ? c.Ids( "id", false ) : c.Ids() );
		        return c.EditSelecting( "Hollow",
		            [&]( scene::DocumentEdit &e, Selection &after ) -> EditResult
		            {
			            std::vector<ObjectId> groups;
			            if ( EditResult r =
			                     ops::Hollow( e, ids.Value(), thickness.Value(), &groups );
			                !r )
				            return r;
			            after = SelectOnly( groups );
			            return {};
		            } );
	        } },
	    { { "move_vertices", { "id", "indices", "delta" }, {},
	          "Move vertices (indices into the solid's vertex list)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( id, c.Id( "id" ) );
		        TRY( delta, c.Vector( "delta" ) );
		        std::vector<int> indices;
		        std::istringstream in( c.Arg( "indices" ) );
		        std::string token;
		        while ( in >> token )
		        {
			        const auto v = ParseCommandNumber( token );
			        if ( !v || *v < 0 || *v != std::floor( *v ) )
				        return c.Fail(
				            CommandStatus::InvalidArgument, "not a vertex index: " + token );
			        indices.push_back( static_cast<int>( *v ) );
		        }
		        return c.Edit( "Move vertices",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::MoveVertices( e, id.Value(), indices, delta.Value() );
		            } );
	        } },
	    { { "push_face", { "face", "distance" }, {}, "Move a face along its normal." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces( "face" ) );
		        TRY( distance, c.Number( "distance" ) );
		        if ( faces.Value().size() != 1 )
			        return c.Fail( CommandStatus::InvalidArgument, "face names one solid:side" );
		        return c.Edit( "Push face",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::PushFace( e, faces.Value()[0], distance.Value() );
		            } );
	        } },
	    { { "extrude_face", { "face", "distance" }, {},
	          "Extrude a face into a new solid; outputs its id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces( "face" ) );
		        TRY( distance, c.Number( "distance" ) );
		        if ( faces.Value().size() != 1 )
			        return c.Fail( CommandStatus::InvalidArgument, "face names one solid:side" );
		        return c.EditSelecting( "Extrude face",
		            [&]( scene::DocumentEdit &e, Selection &after ) -> EditResult
		            {
			            ObjectId created;
			            if ( EditResult r =
			                     ops::ExtrudeFace( e, faces.Value()[0], distance.Value(), created );
			                !r )
				            return r;
			            after = SelectOnly( { created } );
			            return {};
		            } );
	        } },

	    // --- Textures -----------------------------------------------------------
	    { { "apply_material", {}, { "material", "ids", "faces" },
	          "Apply a material (default: the active one) to faces, or to every face of objects "
	          "(default: the selection)." },
	        []( const Context &c ) -> CommandResult
	        {
		        const std::string material =
		            c.Has( "material" ) ? c.Arg( "material" ) : c.settings.faceTexture.material;
		        if ( c.Has( "faces" ) ||
		             ( !c.Has( "ids" ) && !c.session.CurrentSelection().faces.empty() ) )
		        {
			        TRY( faces, c.Faces() );
			        return c.Edit( "Apply material",
			            [&]( scene::DocumentEdit &e )
			            {
				            return ops::ApplyMaterial( e, faces.Value(), material );
			            } );
		        }
		        TRY( ids, c.Ids() );
		        return c.Edit( "Apply material",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::ApplyMaterialToObjects( e, ids.Value(), material );
		            } );
	        } },
	    { { "set_texture", {},
	          { "faces", "shift_u", "shift_v", "scale_u", "scale_v", "rotation", "lightmap_scale" },
	          "Set texture values on faces (default: the selected faces)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        ops::TextureValues v;
		        const std::pair<const char *, std::optional<double> *> fields[] = {
		            { "shift_u", &v.shiftU }, { "shift_v", &v.shiftV }, { "scale_u", &v.scaleU },
		            { "scale_v", &v.scaleV }, { "rotation", &v.rotation },
		            { "lightmap_scale", &v.lightmapScale } };
		        for ( const auto &[key, slot] : fields )
		        {
			        if ( c.Has( key ) )
			        {
				        TRY( value, c.Number( key ) );
				        *slot = value.Value();
			        }
		        }
		        return c.Edit( "Texture values",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetTextureValues( e, faces.Value(), v );
		            } );
	        } },
	    { { "shift_texture", { "du", "dv" }, { "faces" }, "Nudge texture shifts." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( du, c.Number( "du" ) );
		        TRY( dv, c.Number( "dv" ) );
		        return c.Edit( "Shift texture",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::ShiftTexture( e, faces.Value(), du.Value(), dv.Value() );
		            } );
	        } },
	    { { "set_smoothing_group", { "group", "on" }, { "faces" },
	          "Set or clear smoothing group 1..32 on faces." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( group, c.Integer( "group" ) );
		        TRY( on, c.Flag( "on", true ) );
		        return c.Edit( "Smoothing group",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetSmoothingGroup(
			                e, faces.Value(), group.Value(), on.Value() );
		            } );
	        } },
	    { { "justify", { "mode" }, { "faces", "as_one", "fit_u", "fit_v" },
	          "Justify textures: left, right, top, bottom, center or fit." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( !c.services.materials )
			        return c.Fail(
			            CommandStatus::Rejected, "no material information in this composition" );
		        TRY( faces, c.Faces() );
		        const std::string &m = c.Arg( "mode" );
		        ops::Justification j;
		        if ( m == "left" )
			        j = ops::Justification::Left;
		        else if ( m == "right" )
			        j = ops::Justification::Right;
		        else if ( m == "top" )
			        j = ops::Justification::Top;
		        else if ( m == "bottom" )
			        j = ops::Justification::Bottom;
		        else if ( m == "center" )
			        j = ops::Justification::Center;
		        else if ( m == "fit" )
			        j = ops::Justification::Fit;
		        else
			        return c.Fail( CommandStatus::InvalidArgument,
			            "mode is left, right, top, bottom, center or fit" );
		        TRY( asOne, c.Flag( "as_one", false ) );
		        TRY( fitU, c.NumberOr( "fit_u", 1 ) );
		        TRY( fitV, c.NumberOr( "fit_v", 1 ) );
		        return c.Edit( "Justify texture",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::JustifyTexture( e, faces.Value(), j, *c.services.materials,
			                asOne.Value(), static_cast<int>( fitU.Value() ),
			                static_cast<int>( fitV.Value() ) );
		            } );
	        } },
	    { { "align_texture", { "mode" }, { "faces" }, "Realign texture axes: world or face." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        if ( c.Arg( "mode" ) != "world" && c.Arg( "mode" ) != "face" )
			        return c.Fail( CommandStatus::InvalidArgument, "mode is world or face" );
		        const ops::TextureAlignment a = c.Arg( "mode" ) == "world"
		                                            ? ops::TextureAlignment::World
		                                            : ops::TextureAlignment::Face;
		        return c.Edit( "Align texture",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::AlignTexture( e, faces.Value(), a );
		            } );
	        } },
	    { { "replace_material", { "find", "replace" }, { "substring", "ids" },
	          "Replace a material everywhere (or on ids); outputs the face count." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( substring, c.Flag( "substring", false ) );
		        std::vector<ObjectId> ids;
		        if ( c.Has( "ids" ) )
		        {
			        TRY( chosen, c.Ids() );
			        ids = chosen.Value();
		        }
		        int count = 0;
		        auto result = c.Edit( "Replace material",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::ReplaceMaterial(
			                e, ids, c.Arg( "find" ), c.Arg( "replace" ), substring.Value(), count );
		            } );
		        if ( !result )
			        return result;
		        return std::to_string( count );
	        } },

	    // --- Structure ----------------------------------------------------------
	    { { "delete", {}, { "ids" }, "Delete objects (the selection by default)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Delete",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::DeleteObjects( e, ids.Value() );
		            } );
	        } },
	    { { "delete_selection", {}, {}, "Delete the selection (alias of delete)." },
	        []( const Context &c ) -> CommandResult
	        {
		        const std::vector<ObjectId> ids = c.session.CurrentSelection().objects;
		        return c.Edit( "Delete",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::DeleteObjects( e, ids );
		            } );
	        } },
	    { { "group", {}, { "ids" }, "Group objects; outputs the group id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.EditSelecting( "Group",
		            [&]( scene::DocumentEdit &e, Selection &after ) -> EditResult
		            {
			            ObjectId g;
			            if ( EditResult r = ops::GroupObjects( e, ids.Value(), g ); !r )
				            return r;
			            after = SelectOnly( { g } );
			            return {};
		            } );
	        } },
	    { { "ungroup", {}, { "ids" }, "Dissolve groups." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Ungroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::UngroupObjects( e, ids.Value() );
		            } );
	        } },
	    { { "tie_to_entity", {}, { "classname", "ids", "entity" },
	          "Make solids a brush entity (a new one of classname, or an existing entity); outputs "
	          "its id." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        ObjectId existing;
		        if ( c.Has( "entity" ) )
		        {
			        TRY( e, c.Id( "entity" ) );
			        existing = e.Value();
		        }
		        const std::string classname =
		            c.Has( "classname" ) ? c.Arg( "classname" ) : "func_detail";
		        std::string output;
		        auto result = c.EditSelecting( "Tie to entity",
		            [&]( scene::DocumentEdit &e, Selection &after ) -> EditResult
		            {
			            ObjectId entity;
			            if ( EditResult r = ops::TieToEntity(
			                     e, ids.Value(), classname, c.services.catalog, existing, entity );
			                !r )
				            return r;
			            after = SelectOnly( { entity } );
			            output = std::to_string( SessionCommands::ScriptId( entity ) );
			            return {};
		            } );
		        if ( !result )
			        return result;
		        return output;
	        } },
	    { { "move_to_world", {}, { "ids" }, "Return brush entity solids to the world." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Move to world",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::MoveToWorld( e, ids.Value() );
		            } );
	        } },
	    { { "hide", {}, { "ids" }, "Quick-hide objects." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Hide",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetHidden( e, ids.Value(), true );
		            } );
	        } },
	    { { "hide_unselected", {}, {}, "Quick-hide everything not selected." },
	        []( const Context &c ) -> CommandResult
	        {
		        const std::vector<ObjectId> keep = c.session.CurrentSelection().objects;
		        return c.Edit( "Hide unselected",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::HideUnselected( e, keep );
		            } );
	        } },
	    { { "unhide_all", {}, {}, "Clear every quick-hide." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Edit( "Unhide all",
		            []( scene::DocumentEdit &e )
		            {
			            return ops::UnhideAll( e );
		            } );
	        } },

	    // --- Entities -----------------------------------------------------------
	    { { "set_key", { "key", "value" }, { "ids" },
	          "Set a key on entities (the selection by default)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Set " + c.Arg( "key" ),
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetKey( e, ids.Value(), c.Arg( "key" ), c.Arg( "value" ) );
		            } );
	        } },
	    { { "set_entity_property", { "id", "key", "value" }, {},
	          "Set a key on one entity (alias of set_key)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( id, c.Id( "id" ) );
		        return c.Edit( "Set " + c.Arg( "key" ),
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetKey( e, { id.Value() }, c.Arg( "key" ), c.Arg( "value" ) );
		            } );
	        } },
	    { { "set_entity_origin", { "id", "origin" }, {}, "Move a point entity to an origin." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( id, c.Id( "id" ) );
		        TRY( origin, c.Vector( "origin" ) );
		        return c.Edit( "Move entity",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetKey(
			                e, { id.Value() }, "origin", scene::FormatVec3( origin.Value() ) );
		            } );
	        } },
	    { { "remove_key", { "key" }, { "ids" }, "Remove a key from entities." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Remove " + c.Arg( "key" ),
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RemoveKey( e, ids.Value(), c.Arg( "key" ) );
		            } );
	        } },
	    { { "rename_key", { "from", "to" }, { "ids" }, "Rename a key on entities." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Rename key",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RenameKey( e, ids.Value(), c.Arg( "from" ), c.Arg( "to" ) );
		            } );
	        } },
	    { { "set_class", { "classname" }, { "ids" }, "Change entities' class." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        return c.Edit( "Change class",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetClass(
			                e, ids.Value(), c.Arg( "classname" ), c.services.catalog );
		            } );
	        } },
	    { { "set_flag", { "flag", "on" }, { "ids" }, "Set or clear a spawnflags bit." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        TRY( flag, c.Number( "flag" ) );
		        TRY( on, c.Flag( "on", true ) );
		        return c.Edit( "Spawnflags",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetSpawnFlag(
			                e, ids.Value(), static_cast<long long>( flag.Value() ), on.Value() );
		            } );
	        } },
	    { { "add_output", { "output", "target", "input" }, { "parameter", "delay", "times", "ids" },
	          "Add an output connection to entities." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        TRY( delay, c.NumberOr( "delay", 0 ) );
		        TRY( times, c.NumberOr( "times", -1 ) );
		        scene::Connection conn;
		        conn.output = c.Arg( "output" );
		        conn.target = c.Arg( "target" );
		        conn.input = c.Arg( "input" );
		        conn.parameter = c.Has( "parameter" ) ? c.Arg( "parameter" ) : "";
		        conn.delay = delay.Value();
		        conn.timesToFire = static_cast<int>( times.Value() );
		        return c.Edit( "Add output",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::AddConnection( e, ids.Value(), conn );
		            } );
	        } },
	    { { "remove_outputs", {}, { "output", "target", "ids" },
	          "Remove output connections matching output and/or target." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        const std::string output = c.Has( "output" ) ? c.Arg( "output" ) : "";
		        const std::string target = c.Has( "target" ) ? c.Arg( "target" ) : "";
		        return c.Edit( "Remove outputs",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RemoveConnections( e, ids.Value(),
			                [&]( const scene::Connection &conn )
			                {
				                return ( output.empty() || conn.output == output ) &&
				                       ( target.empty() || conn.target == target );
			                } );
		            } );
	        } },
	    { { "rename_entity", { "id", "name" }, { "update_references" },
	          "Rename an entity, updating references by default." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( id, c.Id( "id" ) );
		        TRY( update, c.Flag( "update_references", true ) );
		        return c.Edit( "Rename entity",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RenameEntity(
			                e, id.Value(), c.Arg( "name" ), update.Value(), c.services.catalog );
		            } );
	        } },
	    { { "set_world_key", { "key", "value" }, {}, "Set a worldspawn key." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Edit( "World " + c.Arg( "key" ),
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetWorldKey( e, c.Arg( "key" ), c.Arg( "value" ) );
		            } );
	        } },
	    { { "set_world_property", { "key", "value" }, {},
	          "Set a worldspawn key (alias of set_world_key)." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Edit( "World " + c.Arg( "key" ),
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetWorldKey( e, c.Arg( "key" ), c.Arg( "value" ) );
		            } );
	        } },

	    { { "remove_output_at", { "id", "index" }, {},
	          "Remove one output connection by its 0-based index." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( id, c.Id( "id" ) );
		        TRY( index, c.Integer( "index" ) );
		        if ( index.Value() < 0 )
			        return c.Fail( CommandStatus::InvalidArgument, "index cannot be negative" );
		        return c.Edit( "Remove output",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RemoveConnectionAt(
			                e, id.Value(), static_cast<std::size_t>( index.Value() ) );
		            } );
	        } },

	    // --- Clipboard ------------------------------------------------------------
	    { { "copy", {}, { "ids" },
	          "Copy objects (the selection by default) to the clipboard; outputs the object "
	          "count." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( !c.services.clipboard )
			        return c.Fail( CommandStatus::Rejected, "no clipboard in this composition" );
		        TRY( ids, c.Ids() );
		        if ( ids.Value().empty() )
			        return c.Fail( CommandStatus::Rejected, "nothing to copy" );
		        *c.services.clipboard = Copy( c.session.Document(), ids.Value() );
		        return std::to_string( c.services.clipboard->objects.size() );
	        } },
	    { { "cut", {}, { "ids" }, "Copy objects to the clipboard, then delete them." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( !c.services.clipboard )
			        return c.Fail( CommandStatus::Rejected, "no clipboard in this composition" );
		        TRY( ids, c.Ids() );
		        if ( ids.Value().empty() )
			        return c.Fail( CommandStatus::Rejected, "nothing to cut" );
		        MapFragment fragment = Copy( c.session.Document(), ids.Value() );
		        auto result = c.Edit( "Cut",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::DeleteObjects( e, ids.Value() );
		            } );
		        if ( result )
			        *c.services.clipboard = std::move( fragment );
		        return result;
	        } },
	    { { "paste", {}, { "offset", "rotation", "copies", "group", "name_fix", "name_text" },
	          "Paste the clipboard (Paste Special: offset and rotation per copy, copies, group, "
	          "name_fix keep|suffix|prefix); outputs the pasted ids." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( !c.services.clipboard )
			        return c.Fail( CommandStatus::Rejected, "no clipboard in this composition" );
		        PasteOptions options;
		        if ( c.Has( "offset" ) )
		        {
			        TRY( offset, c.Vector( "offset" ) );
			        options.offset = offset.Value();
		        }
		        if ( c.Has( "rotation" ) )
		        {
			        TRY( rotation, c.Vector( "rotation" ) );
			        options.rotation = rotation.Value();
		        }
		        if ( c.Has( "copies" ) )
		        {
			        TRY( copies, c.Integer( "copies" ) );
			        options.copies = copies.Value();
		        }
		        TRY( group, c.Flag( "group", false ) );
		        options.group = group.Value();
		        if ( c.Has( "name_fix" ) )
		        {
			        const std::string &f = c.Arg( "name_fix" );
			        if ( f == "keep" )
				        options.nameFix = PasteOptions::NameFix::Keep;
			        else if ( f == "suffix" )
				        options.nameFix = PasteOptions::NameFix::Suffix;
			        else if ( f == "prefix" )
				        options.nameFix = PasteOptions::NameFix::Prefix;
			        else
				        return c.Fail(
				            CommandStatus::InvalidArgument, "name_fix is keep, suffix or prefix" );
		        }
		        if ( c.Has( "name_text" ) )
			        options.nameText = c.Arg( "name_text" );
		        std::string output;
		        auto result = c.EditSelecting( "Paste",
		            [&]( scene::DocumentEdit &e, Selection &after ) -> EditResult
		            {
			            std::vector<ObjectId> created;
			            if ( EditResult r = Paste( e, *c.services.clipboard, options, &created );
			                !r )
				            return r;
			            after = SelectOnly( created );
			            output = IdList( created );
			            return {};
		            } );
		        if ( !result )
			        return result;
		        return output;
	        } },
	    { { "duplicate", {}, { "offset", "ids" },
	          "Duplicate objects in place plus an offset (default: the settings' offset); outputs "
	          "the new ids." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( ids, c.Ids() );
		        Vec3d offset = c.settings.duplicateOffset;
		        if ( c.Has( "offset" ) )
		        {
			        TRY( o, c.Vector( "offset" ) );
			        offset = o.Value();
		        }
		        std::string output;
		        auto result = c.EditSelecting( "Duplicate",
		            [&]( scene::DocumentEdit &e, Selection &after ) -> EditResult
		            {
			            std::vector<ObjectId> created;
			            if ( EditResult r = Duplicate( e, ids.Value(), offset, &created ); !r )
				            return r;
			            after = SelectOnly( created );
			            output = IdList( created );
			            return {};
		            } );
		        if ( !result )
			        return result;
		        return output;
	        } },

	    // --- Visgroups --------------------------------------------------------------
	    { { "visgroup_create", { "name" }, { "parent" }, "Create a visgroup; outputs its id." },
	        []( const Context &c ) -> CommandResult
	        {
		        int parent = 0;
		        if ( c.Has( "parent" ) )
		        {
			        TRY( p, c.Integer( "parent" ) );
			        parent = p.Value();
		        }
		        int created = 0;
		        auto result = c.Edit( "Create visgroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::CreateVisgroup( e, c.Arg( "name" ), parent, created );
		            } );
		        if ( !result )
			        return result;
		        return std::to_string( created );
	        } },
	    { { "visgroup_rename", { "visgroup", "name" }, {}, "Rename a visgroup." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( v, c.Integer( "visgroup" ) );
		        return c.Edit( "Rename visgroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RenameVisgroup( e, v.Value(), c.Arg( "name" ) );
		            } );
	        } },
	    { { "visgroup_delete", { "visgroup" }, {}, "Delete a visgroup (members are kept)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( v, c.Integer( "visgroup" ) );
		        return c.Edit( "Delete visgroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::DeleteVisgroup( e, v.Value() );
		            } );
	        } },
	    { { "visgroup_move", { "visgroup", "parent" }, {},
	          "Reparent a visgroup (parent 0: top level)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( v, c.Integer( "visgroup" ) );
		        TRY( p, c.Integer( "parent" ) );
		        return c.Edit( "Move visgroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::MoveVisgroup( e, v.Value(), p.Value() );
		            } );
	        } },
	    { { "visgroup_add", { "visgroup" }, { "ids", "exclusive" },
	          "Add objects to a visgroup (exclusive=1 removes them from others)." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( v, c.Integer( "visgroup" ) );
		        TRY( ids, c.Ids() );
		        TRY( exclusive, c.Flag( "exclusive", false ) );
		        return c.Edit( "Add to visgroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::AddToVisgroup( e, ids.Value(), v.Value(), exclusive.Value() );
		            } );
	        } },
	    { { "visgroup_remove", { "visgroup" }, { "ids" }, "Remove objects from a visgroup." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( v, c.Integer( "visgroup" ) );
		        TRY( ids, c.Ids() );
		        return c.Edit( "Remove from visgroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RemoveFromVisgroup( e, ids.Value(), v.Value() );
		            } );
	        } },
	    { { "visgroup_show", { "visgroup", "on" }, {},
	          "Show (1) or hide (0) a visgroup's members." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( v, c.Integer( "visgroup" ) );
		        TRY( on, c.Flag( "on", true ) );
		        return c.Edit( on.Value() ? "Show visgroup" : "Hide visgroup",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetVisgroupVisible( e, v.Value(), on.Value() );
		            } );
	        } },

	    // --- Cordons -----------------------------------------------------------------
	    { { "cordon_add", { "mins", "maxs" }, { "name" },
	          "Add a cordon with one box; outputs its index." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( box, c.Box() );
		        std::size_t index = 0;
		        const std::string name = c.Has( "name" ) ? c.Arg( "name" ) : "cordon";
		        auto result = c.Edit( "Add cordon",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::AddCordon( e, name, box.Value(), &index );
		            } );
		        if ( !result )
			        return result;
		        return std::to_string( index );
	        } },
	    { { "cordon_remove", { "index" }, {}, "Remove a cordon." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( i, c.Integer( "index" ) );
		        if ( i.Value() < 0 )
			        return c.Fail( CommandStatus::InvalidArgument, "index cannot be negative" );
		        return c.Edit( "Remove cordon",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::RemoveCordon( e, static_cast<std::size_t>( i.Value() ) );
		            } );
	        } },
	    { { "cordon_box", { "index", "box", "mins", "maxs" }, {},
	          "Set box 'box' of cordon 'index'." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( i, c.Integer( "index" ) );
		        TRY( b, c.Integer( "box" ) );
		        TRY( box, c.Box() );
		        if ( i.Value() < 0 || b.Value() < 0 )
			        return c.Fail( CommandStatus::InvalidArgument, "indices cannot be negative" );
		        return c.Edit( "Cordon box",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetCordonBox( e, static_cast<std::size_t>( i.Value() ),
			                static_cast<std::size_t>( b.Value() ), box.Value() );
		            } );
	        } },
	    { { "cordon_active", { "index", "on" }, {}, "Activate or deactivate one cordon." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( i, c.Integer( "index" ) );
		        TRY( on, c.Flag( "on", true ) );
		        if ( i.Value() < 0 )
			        return c.Fail( CommandStatus::InvalidArgument, "index cannot be negative" );
		        return c.Edit( "Cordon active",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetCordonActive(
			                e, static_cast<std::size_t>( i.Value() ), on.Value() );
		            } );
	        } },
	    { { "cordons_enabled", { "on" }, {}, "Enable or disable cordoning for builds." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( on, c.Flag( "on", true ) );
		        return c.Edit( "Cordons",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetCordonsEnabled( e, on.Value() );
		            } );
	        } },

	    // --- Displacements -------------------------------------------------------------
	    { { "create_displacement", { "power" }, { "faces" },
	          "Make quad faces flat displacements of power 2..4." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( power, c.Integer( "power" ) );
		        return c.Edit( "Create displacement",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::CreateDisplacement( e, faces.Value(), power.Value() );
		            } );
	        } },
	    { { "destroy_displacement", {}, { "faces" }, "Remove displacements from faces." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        return c.Edit( "Destroy displacement",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::DestroyDisplacement( e, faces.Value() );
		            } );
	        } },
	    { { "displacement_power", { "power" }, { "faces" },
	          "Resample displacements to power 2..4." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( power, c.Integer( "power" ) );
		        return c.Edit( "Displacement power",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetDisplacementPower( e, faces.Value(), power.Value() );
		            } );
	        } },
	    { { "sculpt", { "center", "radius", "amount" }, { "mode", "direction", "falloff", "faces" },
	          "Sculpt displacements: mode raise, lower, set or smooth; linear falloff by "
	          "default." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( center, c.Vector( "center" ) );
		        TRY( radius, c.Number( "radius" ) );
		        TRY( amount, c.Number( "amount" ) );
		        TRY( falloff, c.Flag( "falloff", true ) );
		        ops::SculptBrush brush;
		        brush.center = center.Value();
		        brush.radius = radius.Value();
		        brush.amount = amount.Value();
		        brush.falloff = falloff.Value();
		        if ( c.Has( "direction" ) )
		        {
			        TRY( dir, c.Vector( "direction" ) );
			        brush.direction = dir.Value();
		        }
		        if ( c.Has( "mode" ) )
		        {
			        const std::string &m = c.Arg( "mode" );
			        if ( m == "raise" )
				        brush.mode = ops::SculptMode::Raise;
			        else if ( m == "lower" )
				        brush.mode = ops::SculptMode::Lower;
			        else if ( m == "set" )
				        brush.mode = ops::SculptMode::Set;
			        else if ( m == "smooth" )
				        brush.mode = ops::SculptMode::Smooth;
			        else
				        return c.Fail(
				            CommandStatus::InvalidArgument, "mode is raise, lower, set or smooth" );
		        }
		        return c.Edit( "Sculpt",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::Sculpt( e, faces.Value(), brush );
		            } );
	        } },
	    { { "paint_alpha", { "center", "radius", "value" }, { "mode", "falloff", "faces" },
	          "Paint displacement alpha: mode set, raise or lower." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( center, c.Vector( "center" ) );
		        TRY( radius, c.Number( "radius" ) );
		        TRY( value, c.Number( "value" ) );
		        TRY( falloff, c.Flag( "falloff", true ) );
		        ops::AlphaMode mode = ops::AlphaMode::Set;
		        if ( c.Has( "mode" ) )
		        {
			        const std::string &m = c.Arg( "mode" );
			        if ( m == "raise" )
				        mode = ops::AlphaMode::Raise;
			        else if ( m == "lower" )
				        mode = ops::AlphaMode::Lower;
			        else if ( m != "set" )
				        return c.Fail(
				            CommandStatus::InvalidArgument, "mode is set, raise or lower" );
		        }
		        return c.Edit( "Paint alpha",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::PaintAlpha( e, faces.Value(), center.Value(), radius.Value(),
			                value.Value(), mode, falloff.Value() );
		            } );
	        } },
	    { { "displacement_elevation", { "elevation" }, { "faces" }, "Set displacement elevation." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        TRY( elevation, c.Number( "elevation" ) );
		        return c.Edit( "Displacement elevation",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SetDisplacementElevation( e, faces.Value(), elevation.Value() );
		            } );
	        } },
	    { { "sew", {}, { "faces" }, "Sew coincident displacement vertices together." },
	        []( const Context &c ) -> CommandResult
	        {
		        TRY( faces, c.Faces() );
		        return c.Edit( "Sew displacements",
		            [&]( scene::DocumentEdit &e )
		            {
			            return ops::SewDisplacements( e, faces.Value() );
		            } );
	        } },

	    // --- Map check -------------------------------------------------------------------
	    { { "check_map", {}, {},
	          "List map problems, one per line: code, severity, fixable, objects, message." },
	        []( const Context &c ) -> CommandResult
	        {
		        std::string out;
		        for ( const MapProblem &p :
		            CheckMap( c.session.Document(), c.services.catalog, c.services.materials ) )
		        {
			        out += std::string( MapProblemCodeName( p.code ) ) + " " +
			               ( p.severity == MapProblem::Severity::Error ? "error" : "warning" ) +
			               " fixable=" + ( p.fixable ? "1" : "0" ) + " ids=\"" +
			               IdList( p.objects ) + "\" " + p.message + "\n";
		        }
		        return out;
	        } },
	    { { "fix_problem", { "code" }, { "ids" },
	          "Fix the fixable problems with this code (those naming ids first, when given) in one "
	          "undo step; outputs the count." },
	        []( const Context &c ) -> CommandResult
	        {
		        std::optional<MapProblem::Code> code;
		        for ( int i = 0; i <= static_cast<int>( MapProblem::Code::OutsideMapBounds ); ++i )
			        if ( c.Arg( "code" ) ==
			             MapProblemCodeName( static_cast<MapProblem::Code>( i ) ) )
				        code = static_cast<MapProblem::Code>( i );
		        if ( !code )
			        return c.Fail(
			            CommandStatus::InvalidArgument, "unknown problem code " + c.Arg( "code" ) );
		        std::vector<ObjectId> ids;
		        if ( c.Has( "ids" ) )
		        {
			        TRY( chosen, c.Ids( "ids", false ) );
			        ids = chosen.Value();
		        }
		        std::vector<MapProblem> matching;
		        for ( const MapProblem &p :
		            CheckMap( c.session.Document(), c.services.catalog, c.services.materials ) )
		        {
			        if ( p.code != *code || !p.fixable )
				        continue;
			        const bool named = ids.empty() || ( !p.objects.empty() &&
			                                              std::find( ids.begin(), ids.end(),
			                                                  p.objects.front() ) != ids.end() );
			        if ( named )
				        matching.push_back( p );
		        }
		        if ( matching.empty() )
			        return c.Fail( CommandStatus::Rejected, "no fixable problem matches" );
		        auto result = c.Edit( "Fix " + c.Arg( "code" ),
		            [&]( scene::DocumentEdit &e ) -> EditResult
		            {
			            for ( const MapProblem &p : matching )
				            if ( EditResult r = FixProblem( e, p, c.services.catalog );
				                !r && r.Error().code != EditErrorCode::Nothing )
					            return r;
			            return {};
		            } );
		        if ( !result )
			        return result;
		        return std::to_string( matching.size() );
	        } },
	    { { "fix_all", {}, {}, "Fix every fixable map problem in one undo step." },
	        []( const Context &c ) -> CommandResult
	        {
		        return c.Edit( "Fix all problems",
		            [&]( scene::DocumentEdit &e )
		            {
			            return FixAll( e, c.services.catalog, c.services.materials );
		            } );
	        } },

	    // --- Build --------------------------------------------------------------
	    { { "build_map", { "path" }, { "quality", "publish" },
	          "Save to path, then compile it (quality fast or full; publish=1 makes it "
	          "playable)." },
	        []( const Context &c ) -> CommandResult
	        {
		        if ( !c.services.builder || !c.services.codec || !c.services.store )
			        return c.Fail( CommandStatus::Rejected,
			            "no map builder, codec or file store in this composition" );
		        const bool full = c.Has( "quality" ) && c.Arg( "quality" ) == "full";
		        if ( c.Has( "quality" ) && !full && c.Arg( "quality" ) != "fast" )
			        return c.Fail( CommandStatus::InvalidArgument, "quality is fast or full" );
		        TRY( publish, c.Flag( "publish", false ) );
		        auto saved = SaveDocumentAs(
		            c.session, *c.services.codec, *c.services.store, c.Arg( "path" ) );
		        if ( !saved )
			        return c.Fail( CommandStatus::IoFailure, saved.Error().message );
		        const ports::MapBuildResult result =
		            c.services.builder->Build( { c.Arg( "path" ), full, publish.Value() } );
		        if ( !result.ok )
			        return c.Fail( CommandStatus::Rejected, result.status + ": " + result.detail );
		        return result.status;
	        } },
	};
	return table;
}

#undef TRY

} // namespace

SessionCommands::SessionCommands(
    EditSession &session, EditorSettings &settings, const SessionServices &services )
    : m_session( session ), m_settings( settings ), m_services( services )
{
}

std::uint32_t SessionCommands::ScriptId( ObjectId id )
{
	return static_cast<std::uint32_t>( id.value & 0xffffffffu );
}

ObjectId SessionCommands::FromScriptId( std::uint32_t scriptId ) const
{
	ObjectId id;
	id.value = ( static_cast<std::uint64_t>( m_session.Document().Serial() ) << 32 ) | scriptId;
	return id;
}

CommandResult SessionCommands::Execute( std::string_view name, const CommandArgs &args )
{
	for ( const CommandEntry &entry : Table() )
	{
		if ( entry.info.name != name )
			continue;
		if ( auto valid = ValidateCommandArgs( entry.info, args ); !valid )
			return foundation::MakeUnexpected( std::move( valid ).Error() );
		return entry.run( Context{ m_session, m_settings, m_services, args, name } );
	}
	return CommandFailure( CommandStatus::UnknownCommand, name, "no such command" );
}

foundation::Expected<std::vector<std::string>, CommandError> SessionCommands::Run(
    const std::vector<ScriptCommand> &script )
{
	return RunCommandScript( script,
	    [this]( std::string_view name, const CommandArgs &args )
	    {
		    return Execute( name, args );
	    } );
}

const std::vector<CommandInfo> &SessionCommands::Catalog()
{
	static const std::vector<CommandInfo> catalog = []
	{
		std::vector<CommandInfo> infos;
		for ( const CommandEntry &entry : Table() )
			infos.push_back( entry.info );
		return infos;
	}();
	return catalog;
}

} // namespace hammer::app
