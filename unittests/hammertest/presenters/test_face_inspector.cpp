//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters face inspector (RFC 0002, R08 domain logic)
//			through a real EditSession and the fake material port: Single and
//			Mixed per field over a face selection, missing-material flag, each
//			edit one labeled undo step (values, shift, justify, align, material).
//			Negative checks: no faces, zero scale and missing materials refuse
//			and change nothing; justify without a material port refuses; stale
//			faces drop out; destruction after the session.
//
//=============================================================================//

#include "hammer/presenters/face_inspector.h"

#include "hammer/app/edit_session.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "fakes/fake_material_info.h"

#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using app::EditErrorCode;
using app::PropertyState;
using app::SelectMode;
using mapgeometry::Vec3d;
using scene::FaceRef;
using scene::ObjectId;

int main()
{
	testing::Checks checks;

	hammertest::FakeMaterialInfo materials;
	materials.Add( "dev/dev_measuregeneric01b", 128, 128 ).Add( "brick/brick01", 256, 256 );

	scene::FaceTexture tex;
	tex.material = "dev/dev_measuregeneric01b";
	scene::MapDocument doc;
	ObjectId box;
	{
		scene::DocumentEdit edit( doc );
		box = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) }, tex ) );
		scene::CommitEdit( doc, edit );
	}
	const scene::Solid &solid = *doc.FindSolid( box );
	const FaceRef f0{ box, solid.sides[0].vmfId };
	const FaceRef f1{ box, solid.sides[1].vmfId };

	auto session = std::make_unique<app::EditSession>( doc );
	auto inspector = std::make_unique<FaceInspector>( *session, &materials );
	checks.That( inspector->Faces().empty() && inspector->Fields().material.IsUnset() &&
	                 inspector->Fields().scaleU.state == PropertyState::kUnset,
	    "no faces: Unset fields" );
	checks.That( inspector->Shift( 1, 1 ).Error().code == EditErrorCode::Nothing,
	    "no faces refuses (negative)" );

	checks.That(
	    session->SelectFaces( { f0, f1 }, SelectMode::Replace ).HasValue(), "select two faces" );
	{
		const FaceFields &f = inspector->Fields();
		checks.That( f.material == app::PropertyValue::Single( "dev/dev_measuregeneric01b" ) &&
		                 f.materialKnown,
		    "material Single and known" );
		checks.That( f.scaleU.state == PropertyState::kSingle && f.scaleU.value == 0.25 &&
		                 f.lightmapScale.state == PropertyState::kSingle &&
		                 f.lightmapScale.value == 16,
		    "scale and lightmap Single" );
	}

	// One face edited: fields become Mixed.
	std::size_t position = session->History().Position();
	checks.That( session->SelectFaces( { f0 }, SelectMode::Replace ).HasValue(), "select one" );
	app::ops::TextureValues values;
	values.scaleU = 0.5;
	checks.That( inspector->SetValues( values ).HasValue(), "set scale" );
	checks.That( session->History().Position() == position + 1 &&
	                 session->History().UndoEntry()->label == "Set texture scale",
	    "one labeled step" );
	checks.That( inspector->ApplyMaterial( "brick/brick01" ).HasValue() &&
	                 session->History().UndoEntry()->label == "Apply material brick/brick01",
	    "apply material" );
	checks.That(
	    session->SelectFaces( { f0, f1 }, SelectMode::Replace ).HasValue(), "select both again" );
	{
		const FaceFields &f = inspector->Fields();
		checks.That( f.material.IsMixed() && f.scaleU.state == PropertyState::kMixed &&
		                 f.scaleV.state == PropertyState::kSingle,
		    "Mixed where faces differ, Single where they agree" );
	}

	values = {};
	values.shiftU = 8;
	values.rotation = 15;
	checks.That( inspector->SetValues( values ).HasValue() &&
	                 session->History().UndoEntry()->label == "Set texture values",
	    "several fields: generic label" );
	checks.That( inspector->Fields().shiftU.state == PropertyState::kSingle &&
	                 inspector->Fields().shiftU.value == 8 &&
	                 inspector->Fields().rotation.value == 15,
	    "aggregates follow the edit" );
	checks.That( inspector->Shift( 2, 0 ).HasValue() && inspector->Fields().shiftU.value == 10 &&
	                 session->History().UndoEntry()->label == "Shift texture",
	    "shift" );
	checks.That( inspector->Align( app::ops::TextureAlignment::World ).HasValue() &&
	                 session->History().UndoEntry()->label == "Align texture to world" &&
	                 inspector->Fields().shiftU.value == 0,
	    "align resets shift" );
	checks.That( inspector->Justify( app::ops::Justification::Fit ).HasValue() &&
	                 session->History().UndoEntry()->label == "Justify texture fit",
	    "justify with material sizes" );
	checks.That( session->Undo().HasValue() &&
	                 session->History().UndoEntry()->label == "Align texture to world",
	    "undo pops one step" );

	// Negative: refusals change nothing.
	const std::uint64_t revision = session->Revision();
	values = {};
	values.scaleU = 0;
	checks.That(
	    !inspector->SetValues( values ) && !inspector->LastError().empty(), "zero scale refused" );
	checks.That( session->Revision() == revision, "nothing changed" );
	checks.That( inspector->ApplyMaterial( "brick/brick01" ).HasValue(), "both faces brick" );
	checks.That( session->Undo().HasValue(), "undo" );
	checks.That(
	    inspector->ApplyMaterial( "missing/mat" ).HasValue() && !inspector->Fields().materialKnown,
	    "a missing material is flagged" );
	checks.That( !inspector->Justify( app::ops::Justification::Fit ),
	    "justify on a missing material refuses" );
	inspector->ClearError();
	checks.That( inspector->LastError().empty(), "error cleared" );
	{
		FaceInspector portless( *session, nullptr );
		checks.That(
		    !portless.Justify( app::ops::Justification::Center ) && portless.Fields().materialKnown,
		    "justify without a material port refuses" );
	}

	// Stale faces drop out when their solid is removed.
	checks.That(
	    session->Replace( scene::MapDocument( 1 ) ).HasValue() && inspector->Faces().empty(),
	    "replacement empties the model" );

	session.reset();
	inspector.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
