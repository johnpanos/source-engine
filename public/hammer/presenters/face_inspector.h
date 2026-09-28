//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The face inspector presentation model (RFC 0002,
//			hammer.presenters): legacy Hammer's Face Edit sheet (faceedit*.cpp)
//			and texture bar without a toolkit. It shows the selected faces
//			(Selection::faces that still exist) as one aggregate per field --
//			material, shift U/V, scale U/V, rotation and lightmap scale -- each
//			Unset (no faces), Single or Mixed. Numbers compare exactly: faces
//			that differ by any amount are Mixed, as the legacy sheet blanks them.
//
//			Every edit is one EditSession::Execute (one undo step) through the
//			texture operations:
//			  SetValues      ops::SetTextureValues  "Set texture shift/scale/
//			                                         rotation", "Set lightmap
//			                                         scale", or "Set texture
//			                                         values" for several fields
//			  Shift          ops::ShiftTexture      "Shift texture"
//			  Justify        ops::JustifyTexture    "Justify texture <how>"
//			                 (needs the IMaterialInfo port for mapping sizes; a
//			                 missing port or material is a refusal)
//			  Align          ops::AlignTexture      "Align texture to world|face"
//			  ApplyMaterial  ops::ApplyMaterial     "Apply material <name>"
//			A refused edit changes nothing and sets LastError().
//
//			Lifetime: the subscription is RAII; the inspector may be destroyed
//			before or after its session, but calls need a live session.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_FACE_INSPECTOR_H
#define HAMMER_PRESENTERS_FACE_INSPECTOR_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/ops/texture_ops.h"
#include "hammer/app/property_value.h"
#include "hammer/ports/material_info.h"

#include <cstdint>
#include <string>
#include <vector>

namespace hammer::presenters
{

// A numeric aggregate: 'value' is meaningful when state is kSingle.
struct NumberField
{
	app::PropertyState state = app::PropertyState::kUnset;
	double value = 0.0;

	friend bool operator==( const NumberField &, const NumberField & ) = default;
};

struct FaceFields
{
	app::PropertyValue material;
	bool materialKnown = true; // false when the port says a Single material is missing
	NumberField shiftU;
	NumberField shiftV;
	NumberField scaleU;
	NumberField scaleV;
	NumberField rotation;
	NumberField lightmapScale;
};

class FaceInspector
{
public:
	using Result = foundation::Expected<void, app::EditError>;

	// 'materials' may be null: then Justify refuses and materialKnown stays true.
	FaceInspector( app::EditSession &session, const ports::IMaterialInfo *materials );
	FaceInspector( const FaceInspector & ) = delete;
	FaceInspector &operator=( const FaceInspector & ) = delete;

	std::uint64_t Revision() const { return m_revision; }
	const std::vector<scene::FaceRef> &Faces() const { return m_faces; }
	const FaceFields &Fields() const { return m_fields; }

	Result SetValues( const app::ops::TextureValues &values );
	Result Shift( double deltaU, double deltaV );
	Result Justify( app::ops::Justification justification, bool treatAsOne = false, int fitU = 1,
	    int fitV = 1 );
	Result Align( app::ops::TextureAlignment alignment );
	Result ApplyMaterial( const std::string &material );

	const std::string &LastError() const { return m_lastError; }
	void ClearError();

private:
	void Rebuild();
	Result Run( const std::string &label, const app::EditSession::Operation &operation );

	app::EditSession &m_session;
	const ports::IMaterialInfo *m_materials = nullptr;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::vector<scene::FaceRef> m_faces;
	FaceFields m_fields;
	std::string m_lastError;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_FACE_INSPECTOR_H
