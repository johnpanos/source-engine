//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Replace Textures parity (RFC 0002, R08-REPLACE-TEXTURES): the
//			legacy MFC workflow against its replacement.
//
//			Legacy side: the verbatim legacy code (CMapDoc::OnEditReplacetex,
//			CReplaceTexDlg's constructor and DoReplaceTextures,
//			CMapDoc::ReplaceTextures, ReplaceTexFunc, FindInString and both
//			CMapFace::SetTexture overloads), frozen from git by
//			tools/quality/hammer_legacy_freeze.py into
//			replace_textures_legacy.inc, compiled against the small stubs
//			below: a world of solids and brush entities, the selection, the
//			texture system's name rule, the history and the message box.
//			The user's dialog input is supplied in DoModal.
//
//			New side: presenters::ReplaceTexturesDialog over EditSession and
//			SessionCommands (the path the GTK host uses), with the same map,
//			selection, tool and dialog input.
//
//			A seeded corpus of generated maps and dialog inputs is run
//			through both. Per case the comparator requires the same message
//			("N textures replaced." / "N solids marked." / "N faces marked."),
//			every face's material under RFC 0015's identity rule, scales and
//			shifts (legacy floats against doubles, relative 1e-5), the
//			modified flag, the marked solids or faces, and the undo label.
//
//			Recorded deviations are asserted, not skipped:
//			  D1 marking within the selection: legacy OnEditReplacetex
//			     cleared the selection before ReplaceTextures read it and
//			     marked nothing; the new path marks within it, compared with
//			     the verbatim ReplaceTextures called without that clear;
//			  D2 rescale with an unknown size: legacy divided by a
//			     placeholder's zero size (non-finite or zero scales); the new
//			     path refuses and changes nothing;
//			  D3 a result that is not a material identity (empty or empty
//			     components, from Substitute): legacy stored it; the new path
//			     refuses and changes nothing;
//			  D4 a replacement that changes no face's identity or values:
//			     legacy set the modified flag and kept an undo step; the new
//			     session records nothing unless a stored spelling changed
//			     (EditSession's no-op rule). Messages and faces still match.
//			Inputs outside the shared contract are not generated: an empty
//			Find and backslashes in Find (legacy compared them without the
//			identity's slash rule).
//
//			Sensitivity: seven seeded defects of the new path (hidden
//			ignored, partial as exact, substitute as partial, no rescale,
//			wrong mark target, scope ignored, one face dropped) must each be
//			detected by the comparator on the corpus.
//
//=============================================================================//

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <strings.h>
#include <vector>

#include "content/asset_identity.h"
#include "fakes/fake_material_info.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/session_commands.h"
#include "hammer/presenters/replace_textures.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

// --- The legacy side ------------------------------------------------------------------

namespace legacy
{

typedef int BOOL;
#define TRUE 1
#define FALSE 0
typedef const char *LPCTSTR;
typedef const char *LPCSTR;
// Legacy Hammer is 32-bit: (DWORD)&info carries a pointer through EnumChildren.
typedef std::uintptr_t DWORD;
typedef void *HCURSOR;
#define _T( x ) x
constexpr int IDOK = 1;
constexpr int IDC_WAIT = 0;
constexpr int IDD_REPLACETEX = 217;
constexpr int EVTYPE_FACE_CHANGED = 1;
constexpr int TOOL_FACEEDIT_MATERIAL = 7;
constexpr int scClear = 0x01;
constexpr int scSelect = 0x02;
constexpr int scSaveChanges = 0x40;
constexpr int selectSolids = 2;

inline int strcmpi( const char *a, const char *b )
{
	return strcasecmp( a, b );
}
inline int strnicmp( const char *a, const char *b, int n )
{
	return strncasecmp( a, b, static_cast<std::size_t>( n ) );
}

class CString
{
public:
	CString() = default;
	CString( const char *s ) : m_s( s ) {}
	CString &operator=( const char *s )
	{
		m_s = s;
		return *this;
	}
	operator const char *() const { return m_s.c_str(); }
	void Format( const char *format, ... )
	{
		char buffer[512];
		va_list args;
		va_start( args, format );
		vsnprintf( buffer, sizeof( buffer ), format, args );
		va_end( args );
		m_s = buffer;
	}

private:
	std::string m_s;
};

// The texture system's name rule: '\' becomes '/', a registered texture is
// found case-insensitively and reports its registered spelling, anything
// else is a dummy of size 0 named as given.
class IEditorTexture
{
public:
	std::string name;
	int width = 0;
	int height = 0;
	bool dummy = false;
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
	int GetImageWidth() const { return width; }
	int GetImageHeight() const { return height; }
	bool HasData() const { return !dummy; }
	void Load() {}
	int GetShortName( char *out ) const
	{
		std::strcpy( out, name.c_str() );
		return static_cast<int>( name.size() );
	}
	int GetSurfaceAttributes() const { return 0; }
	int GetSurfaceContents() const { return 0; }
};

class CTextureSystem
{
public:
	void Register( const std::string &name, int width, int height )
	{
		auto texture = std::make_unique<IEditorTexture>();
		texture->name = name;
		texture->width = width;
		texture->height = height;
		m_textures.push_back( std::move( texture ) );
	}
	void Reset()
	{
		m_textures.clear();
		m_dummies.clear();
	}
	IEditorTexture *FindActiveTexture( LPCSTR input )
	{
		std::string name( input );
		std::replace( name.begin(), name.end(), '\\', '/' );
		for ( auto &t : m_textures )
			if ( !strcmpi( t->name.c_str(), name.c_str() ) )
				return t.get();
		for ( auto &t : m_dummies )
			if ( !strcmpi( t->name.c_str(), name.c_str() ) )
				return t.get();
		auto dummy = std::make_unique<IEditorTexture>();
		dummy->name = name;
		dummy->dummy = true;
		m_dummies.push_back( std::move( dummy ) );
		return m_dummies.back().get();
	}

private:
	std::vector<std::unique_ptr<IEditorTexture>> m_textures;
	std::vector<std::unique_ptr<IEditorTexture>> m_dummies;
};
CTextureSystem g_Textures;

struct TextureStub
{
	char texture[128] = {};
	float UAxis[4] = {};
	float VAxis[4] = {};
	float scale[2] = {};
	int q2surface = 0;
	int q2contents = 0;
};

class CMapFace
{
public:
	TextureStub texture;
	IEditorTexture *m_pTexture = nullptr;
	void SignalUpdate( int ) {}
	void CalcTextureCoords() {}
	void UpdateFaceFlags() {}
	void SetTexture( IEditorTexture *pTexture, bool bRescaleTextureCoordinates = false );
	void SetTexture( const char *pszNewTex, bool bRescaleTextureCoordinates = false );
};

typedef int MAPCLASSTYPE;
#define MAPCLASS_TYPE( class_name ) ( class_name::kType )

class CMapClass;
typedef BOOL ( *ENUMMAPCHILDRENPROC )( CMapClass *, DWORD dwParam );

class CMapClass
{
public:
	virtual ~CMapClass() = default;
	virtual MAPCLASSTYPE Type() const = 0;
	bool IsMapClass( MAPCLASSTYPE type ) const { return Type() == type; }
	// mapclass.cpp: every descendant, depth first, the callback before the
	// child's own children; FALSE stops.
	BOOL EnumChildren( ENUMMAPCHILDRENPROC pfn, DWORD dwParam, MAPCLASSTYPE type )
	{
		for ( CMapClass *child : m_Children )
		{
			if ( !type || child->IsMapClass( type ) )
				if ( !( *pfn )( child, dwParam ) )
					return FALSE;
			if ( !child->EnumChildren( pfn, dwParam, type ) )
				return FALSE;
		}
		return TRUE;
	}
	std::vector<CMapClass *> m_Children;
};

class CMapObjectList
{
public:
	void AddToTail( CMapClass *object ) { m_objects.push_back( object ); }
	void AddVectorToTail( const CMapObjectList &other )
	{
		m_objects.insert( m_objects.end(), other.m_objects.begin(), other.m_objects.end() );
	}
	CMapClass *Element( int i ) const { return m_objects[static_cast<std::size_t>( i )]; }
	int Count() const { return static_cast<int>( m_objects.size() ); }
	std::vector<CMapClass *> m_objects;
};
#define FOR_EACH_OBJ( listName, iteratorName )                                                     \
	for ( int iteratorName = 0; iteratorName < ( listName ).Count(); ++iteratorName )

class CSelection
{
public:
	void SetMode( int mode ) { m_mode = mode; }
	const CMapObjectList *GetList() const { return &m_list; }
	int GetCount() const { return m_list.Count(); }
	CMapObjectList m_list;
	int m_mode = 0;
};

class CMapDoc;
CMapDoc *g_activeDoc = nullptr;

class CMapSolid : public CMapClass
{
public:
	static constexpr MAPCLASSTYPE kType = 1;
	MAPCLASSTYPE Type() const override { return kType; }
	int GetFaceCount() const { return static_cast<int>( m_faces.size() ); }
	CMapFace *GetFace( int i ) { return &m_faces[static_cast<std::size_t>( i )]; }
	BOOL IsVisible() const { return m_visible; }
	BOOL IsSelected() const;
	std::vector<CMapFace> m_faces;
	bool m_visible = true;
};

class CMapEntity : public CMapClass
{
public:
	static constexpr MAPCLASSTYPE kType = 2;
	MAPCLASSTYPE Type() const override { return kType; }
};

class CMapWorld : public CMapClass
{
public:
	static constexpr MAPCLASSTYPE kType = 3;
	MAPCLASSTYPE Type() const override { return kType; }
};

class CToolManager
{
public:
	int GetActiveToolID() const { return m_active; }
	int m_active = 0;
};

class CHistory
{
public:
	void Keep( CMapClass * ) { ++m_kept; }
	void MarkUndoPosition( const CMapObjectList *, const char *label )
	{
		m_labels.push_back( label );
	}
	int m_kept = 0;
	std::vector<std::string> m_labels;
};
CHistory g_history;
CHistory *GetHistory()
{
	return &g_history;
}

struct CFaceEditSheet
{
	void EnableUpdate( bool ) {}
};
struct CMainFrame
{
	CFaceEditSheet *m_pFaceEditSheet;
};
CFaceEditSheet g_sheet;
CMainFrame g_mainFrame = { &g_sheet };
CMainFrame *GetMainWnd()
{
	return &g_mainFrame;
}
HCURSOR LoadCursor( void *, int )
{
	return nullptr;
}
HCURSOR SetCursor( HCURSOR cursor )
{
	return cursor;
}
std::vector<std::string> g_messages;
int AfxMessageBox( const char *text )
{
	g_messages.push_back( text );
	return IDOK;
}
const char *GetDefaultTextureName()
{
	return "dev/dev_measuregeneric01b";
}

class CWnd
{
};
class CDialog
{
public:
	CDialog( int, CWnd * ) {}
};

class CReplaceTexDlg : public CDialog
{
public:
	CReplaceTexDlg( int nSelected, CWnd *pParent = nullptr );
	enum
	{
		IDD = IDD_REPLACETEX
	};
	int m_iSearchAll;
	CString m_strFind;
	CString m_strReplace;
	int m_iAction;
	BOOL m_bMarkOnly;
	BOOL m_bHidden;
	BOOL m_bRescaleTextureCoordinates;
	int m_nSelected;
	void DoReplaceTextures();
	int DoModal();
};
// What the user enters in the dialog (DoModal).
std::function<void( CReplaceTexDlg & )> g_userInput;
int CReplaceTexDlg::DoModal()
{
	g_userInput( *this );
	return IDOK;
}

class CMapDoc
{
public:
	static CMapDoc *GetActiveMapDoc() { return g_activeDoc; }
	void ReplaceTextures( LPCTSTR pszFind, LPCTSTR pszReplace, BOOL bEverything, int iAction,
	    BOOL bHidden, bool bRescaleTextureCoordinates );
	void OnEditReplacetex();
	CToolManager *GetTools() { return m_pToolManager; }
	void SelectFace( CMapSolid *solid, int face, int )
	{
		m_selectedFaces.insert( { solid, face } );
	}
	void SelectObject( CMapClass *object, int flags )
	{
		if ( flags & scClear )
		{
			m_selection.m_list.m_objects.clear();
			m_selectedFaces.clear();
		}
		if ( object && ( flags & scSelect ) &&
		     std::find( m_selection.m_list.m_objects.begin(), m_selection.m_list.m_objects.end(),
		         object ) == m_selection.m_list.m_objects.end() )
			m_selection.m_list.AddToTail( object );
	}
	void SetModifiedFlag( BOOL modified = TRUE ) { m_modified = m_modified || modified; }

	CSelection m_selection;
	CSelection *m_pSelection = &m_selection;
	CMapWorld m_world;
	CMapWorld *m_pWorld = &m_world;
	CToolManager m_tools;
	CToolManager *m_pToolManager = &m_tools;
	std::set<std::pair<CMapSolid *, int>> m_selectedFaces;
	bool m_modified = false;
};

BOOL CMapSolid::IsSelected() const
{
	const auto &objects = g_activeDoc->m_selection.m_list.m_objects;
	return std::find( objects.begin(), objects.end(), this ) != objects.end();
}

#if defined( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#if defined( __clang__ )
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wcast-function-type-strict"
#endif
#endif
#include "replace_textures_legacy.inc"
#if defined( __GNUC__ )
#pragma GCC diagnostic pop
#endif

} // namespace legacy

// --- The corpus ---------------------------------------------------------------------

namespace
{

using namespace hammer;
using mapgeometry::Vec3d;

struct Registered
{
	const char *name;
	int width;
	int height;
};

// The materials the texture system knows, with their registered spelling.
const Registered kRegistered[] = {
    { "BRICK/BRICKWALL001", 256, 256 },
    { "brick/brickwall001a", 128, 128 },
    { "dev/dev_measuregeneric01b", 128, 128 },
    { "concrete/concretefloor001a", 512, 512 },
    { "metal/metalwall_brick", 64, 128 },
    { "tools/toolsnodraw", 64, 64 },
    { "nature/blendbrick_dirt", 256, 96 },
};

const char *const kFaceNames[] = { "BRICK/BRICKWALL001", "brick/BRICKWALL001",
    "Brick/BrickWall001a", "dev/dev_measuregeneric01b", "DEV/DEV_MEASUREGENERIC01B",
    "concrete/concretefloor001a", "metal/metalwall_brick", "tools/toolsnodraw",
    "nature/blendbrick_dirt", "custom/unregistered_wall" };
const char *const kFinds[] = { "brick", "BRICK/BRICKWALL001", "dev/dev_measuregeneric01b", "wall",
    "toolsnodraw", "nope", "Metal", "/", "brickwall001", "custom" };
const char *const kReplacements[] = { "metal/metalwall_brick", "CONCRETE/CONCRETEFLOOR001A",
    "tools/toolsnodraw", "custom/newmaterial", "brick/brickwall001a", "nature/blendbrick_dirt" };
const char *const kSubstitutes[] = {
    "stone", "BRICK", "", "metal", "concrete/concretefloor", "x/" };

struct SolidSpec
{
	std::string names[6];
	float scaleU[6];
	float scaleV[6];
	float shiftU[6];
	float shiftV[6];
	bool hidden = false;
	int entity = -1; // owning brush entity, or -1 for world
};

struct CaseSpec
{
	std::vector<SolidSpec> solids;
	int entities = 0;
	std::vector<int> selectedSolids;   // loose solids selected
	std::vector<int> selectedEntities; // brush entities selected
	std::string find;
	std::string replace;
	int action = 0;
	bool markOnly = false;
	bool hidden = false;
	bool everything = true;
	bool rescale = false;
	bool faceTool = false;
};

CaseSpec Generate( std::mt19937 &rng )
{
	auto pick = [&]( int n )
	{
		return static_cast<int>( rng() % static_cast<unsigned>( n ) );
	};
	const float kScales[] = { 0.25f, 0.5f, 1.0f, 0.125f };
	const float kShifts[] = { 0.0f, 8.0f, -16.0f, 3.0f, 100.0f };
	CaseSpec c;
	c.entities = pick( 3 );
	const int solids = 1 + pick( 6 );
	for ( int s = 0; s < solids; ++s )
	{
		SolidSpec solid;
		for ( int f = 0; f < 6; ++f )
		{
			solid.names[f] = kFaceNames[pick( 10 )];
			solid.scaleU[f] = kScales[pick( 4 )];
			solid.scaleV[f] = kScales[pick( 4 )];
			solid.shiftU[f] = kShifts[pick( 5 )];
			solid.shiftV[f] = kShifts[pick( 5 )];
		}
		solid.hidden = pick( 4 ) == 0;
		solid.entity = c.entities > 0 && pick( 3 ) == 0 ? pick( c.entities ) : -1;
		c.solids.push_back( solid );
	}
	for ( int s = 0; s < solids; ++s )
		if ( c.solids[static_cast<std::size_t>( s )].entity < 0 && pick( 2 ) == 0 )
			c.selectedSolids.push_back( s );
	for ( int e = 0; e < c.entities; ++e )
		if ( pick( 2 ) == 0 )
			c.selectedEntities.push_back( e );
	c.action = pick( 3 );
	c.find = kFinds[pick( 10 )];
	c.replace = c.action == 2 ? kSubstitutes[pick( 6 )] : kReplacements[pick( 6 )];
	c.markOnly = pick( 4 ) == 0;
	c.hidden = pick( 2 ) == 0;
	const bool anySelected = !c.selectedSolids.empty() || !c.selectedEntities.empty();
	// The legacy dialog disabled "Marked objects" with nothing selected.
	c.everything = !anySelected || pick( 2 ) == 0;
	c.rescale = pick( 3 ) == 0;
	c.faceTool = pick( 3 ) == 0;
	return c;
}

// --- Running legacy -----------------------------------------------------------------

struct FaceState
{
	std::string name;
	double scaleU, scaleV, shiftU, shiftV;
};

struct Outcome
{
	std::string message;
	std::vector<std::vector<FaceState>> faces; // per solid
	bool modified = false;
	std::set<int> markedSolids;
	std::set<std::pair<int, int>> markedFaces;
	std::string undoLabel; // the replacement's undo step, if one was recorded
};

struct LegacyWorld
{
	std::vector<std::unique_ptr<legacy::CMapSolid>> solids;
	std::vector<std::unique_ptr<legacy::CMapEntity>> entities;
	std::unique_ptr<legacy::CMapDoc> doc;
};

LegacyWorld BuildLegacy( const CaseSpec &c )
{
	LegacyWorld w;
	w.doc = std::make_unique<legacy::CMapDoc>();
	legacy::g_activeDoc = w.doc.get();
	legacy::g_history = {};
	legacy::g_messages.clear();
	legacy::g_Textures.Reset();
	for ( const Registered &r : kRegistered )
		legacy::g_Textures.Register( r.name, r.width, r.height );
	for ( int e = 0; e < c.entities; ++e )
	{
		w.entities.push_back( std::make_unique<legacy::CMapEntity>() );
		w.doc->m_world.m_Children.push_back( w.entities.back().get() );
	}
	for ( const SolidSpec &spec : c.solids )
	{
		auto solid = std::make_unique<legacy::CMapSolid>();
		solid->m_visible = !spec.hidden;
		solid->m_faces.resize( 6 );
		for ( int f = 0; f < 6; ++f )
		{
			legacy::CMapFace &face = solid->m_faces[static_cast<std::size_t>( f )];
			std::strcpy( face.texture.texture, spec.names[f].c_str() );
			face.m_pTexture = legacy::g_Textures.FindActiveTexture( spec.names[f].c_str() );
			face.texture.scale[0] = spec.scaleU[f];
			face.texture.scale[1] = spec.scaleV[f];
			face.texture.UAxis[3] = spec.shiftU[f];
			face.texture.VAxis[3] = spec.shiftV[f];
		}
		if ( spec.entity >= 0 )
			w.entities[static_cast<std::size_t>( spec.entity )]->m_Children.push_back(
			    solid.get() );
		else
			w.doc->m_world.m_Children.push_back( solid.get() );
		w.solids.push_back( std::move( solid ) );
	}
	for ( int s : c.selectedSolids )
		w.doc->SelectObject( w.solids[static_cast<std::size_t>( s )].get(), legacy::scSelect );
	for ( int e : c.selectedEntities )
		w.doc->SelectObject( w.entities[static_cast<std::size_t>( e )].get(), legacy::scSelect );
	w.doc->m_tools.m_active = c.faceTool ? legacy::TOOL_FACEEDIT_MATERIAL : 0;
	return w;
}

Outcome Collect( const CaseSpec &c, const LegacyWorld &w )
{
	Outcome out;
	out.message = legacy::g_messages.empty() ? std::string() : legacy::g_messages.back();
	for ( const auto &solid : w.solids )
	{
		std::vector<FaceState> faces;
		for ( legacy::CMapFace &f : solid->m_faces )
			faces.push_back( { f.texture.texture, f.texture.scale[0], f.texture.scale[1],
			    f.texture.UAxis[3], f.texture.VAxis[3] } );
		out.faces.push_back( faces );
	}
	out.modified = w.doc->m_modified;
	for ( std::size_t s = 0; s < w.solids.size(); ++s )
	{
		if ( w.solids[s]->IsSelected() && c.markOnly )
			out.markedSolids.insert( static_cast<int>( s ) );
		for ( int f = 0; f < 6; ++f )
			if ( w.doc->m_selectedFaces.count( { w.solids[s].get(), f } ) )
				out.markedFaces.insert( { static_cast<int>( s ), f } );
	}
	if ( !c.markOnly && out.modified && !legacy::g_history.m_labels.empty() )
		out.undoLabel = legacy::g_history.m_labels.back();
	return out;
}

// The menu path: OnEditReplacetex with the dialog filled in by the user.
Outcome RunLegacy( const CaseSpec &c )
{
	LegacyWorld w = BuildLegacy( c );
	legacy::g_userInput = [&]( legacy::CReplaceTexDlg &dlg )
	{
		dlg.m_strFind = c.find.c_str();
		dlg.m_strReplace = c.replace.c_str();
		dlg.m_iSearchAll = c.everything ? TRUE : FALSE;
		dlg.m_iAction = c.action;
		dlg.m_bMarkOnly = c.markOnly;
		dlg.m_bHidden = c.hidden;
		dlg.m_bRescaleTextureCoordinates = c.rescale;
	};
	w.doc->OnEditReplacetex();
	return Collect( c, w );
}

// The verbatim ReplaceTextures called directly (no pre-clear): the D1 reference.
Outcome RunLegacyKernel( const CaseSpec &c )
{
	LegacyWorld w = BuildLegacy( c );
	w.doc->ReplaceTextures( c.find.c_str(), c.replace.c_str(), c.everything ? TRUE : FALSE,
	    c.action | ( c.markOnly ? 0x100 : 0 ), c.hidden, c.rescale );
	return Collect( c, w );
}

// --- Running the new path -------------------------------------------------------------

enum class Defect
{
	None,
	HiddenIgnored,
	PartialAsExact,
	SubstituteAsPartial,
	NoRescale,
	WrongMarkTarget,
	ScopeIgnored,
	DropOneFace,
};

Outcome RunNew( const CaseSpec &c, Defect defect = Defect::None )
{
	hammertest::FakeMaterialInfo materials;
	for ( const Registered &r : kRegistered )
		materials.Add( r.name, r.width, r.height );
	app::EditSession session;
	app::EditorSettings settings;
	settings.faceTexture.material = legacy::GetDefaultTextureName();
	app::MapFragment clipboard;
	app::SessionCommands commands(
	    session, settings, { nullptr, nullptr, nullptr, nullptr, &materials, &clipboard } );

	std::vector<scene::ObjectId> solidIds;
	std::vector<scene::ObjectId> entityIds;
	auto built = session.Execute( "build",
	    [&]( scene::DocumentEdit &e ) -> app::EditResult
	    {
		    for ( int i = 0; i < c.entities; ++i )
		    {
			    scene::Entity entity;
			    entity.classname = "func_detail";
			    entityIds.push_back( e.Add( entity ) );
		    }
		    for ( std::size_t i = 0; i < c.solids.size(); ++i )
		    {
			    const SolidSpec &spec = c.solids[i];
			    const double x = 128.0 * static_cast<double>( i );
			    scene::Solid solid =
			        scene::MakeBoxSolid( { Vec3d( x, 0, 0 ), Vec3d( x + 64, 64, 64 ) }, {} );
			    for ( int f = 0; f < 6; ++f )
			    {
				    scene::FaceTexture &t = solid.sides[static_cast<std::size_t>( f )].texture;
				    t.material = spec.names[f];
				    t.u.scale = spec.scaleU[f];
				    t.v.scale = spec.scaleV[f];
				    t.u.shift = spec.shiftU[f];
				    t.v.shift = spec.shiftV[f];
			    }
			    solid.hidden = spec.hidden;
			    if ( spec.entity >= 0 )
				    solid.owner = entityIds[static_cast<std::size_t>( spec.entity )];
			    solidIds.push_back( e.Add( solid ) );
		    }
		    return {};
	    } );
	session.MarkSaved();
	app::Selection selection;
	for ( int s : c.selectedSolids )
		selection.objects.push_back( solidIds[static_cast<std::size_t>( s )] );
	for ( int e : c.selectedEntities )
		selection.objects.push_back( entityIds[static_cast<std::size_t>( e )] );
	std::sort( selection.objects.begin(), selection.objects.end() );
	if ( !selection.objects.empty() )
		selection.primary = selection.objects.back();
	const bool selected = built.HasValue() && session.SetSelection( selection ).HasValue();

	presenters::ReplaceTexturesDialog dialog( session, commands, &materials );
	dialog.Open( settings.faceTexture.material,
	    defect == Defect::WrongMarkTarget ? !c.faceTool : c.faceTool );
	auto &d = dialog.Draft();
	d.find = c.find;
	d.replace = c.replace;
	d.scope = c.everything || defect == Defect::ScopeIgnored
	              ? presenters::ReplaceTexturesDialog::Scope::Everything
	              : presenters::ReplaceTexturesDialog::Scope::Selection;
	d.match = static_cast<app::ops::MaterialMatch>( c.action );
	if ( defect == Defect::PartialAsExact && c.action == 1 )
		d.match = app::ops::MaterialMatch::Exact;
	if ( defect == Defect::SubstituteAsPartial && c.action == 2 )
		d.match = app::ops::MaterialMatch::Partial;
	d.markOnly = c.markOnly;
	d.includeHidden = c.hidden || defect == Defect::HiddenIgnored;
	d.rescale = c.rescale && defect != Defect::NoRescale;
	const presenters::ReplaceTexturesDialog::Outcome applied = dialog.Apply();

	Outcome out;
	out.message = selected ? applied.message : "setup failed";
	const scene::MapDocument &doc = session.Document();
	for ( scene::ObjectId id : solidIds )
	{
		std::vector<FaceState> faces;
		for ( const scene::Side &side : doc.FindSolid( id )->sides )
			faces.push_back( { side.texture.material, side.texture.u.scale, side.texture.v.scale,
			    side.texture.u.shift, side.texture.v.shift } );
		out.faces.push_back( faces );
	}
	if ( defect == Defect::DropOneFace && applied.applied && !c.markOnly )
	{
		for ( std::size_t s = 0; s < out.faces.size(); ++s )
			for ( std::size_t f = 0; f < 6; ++f )
				if ( out.faces[s][f].name != c.solids[s].names[f] )
				{
					out.faces[s][f].name = c.solids[s].names[f];
					s = out.faces.size() - 1;
					break;
				}
	}
	out.modified = session.IsModified();
	for ( std::size_t s = 0; s < solidIds.size(); ++s )
	{
		if ( c.markOnly && session.CurrentSelection().Contains( solidIds[s] ) )
			out.markedSolids.insert( static_cast<int>( s ) );
		for ( std::size_t f = 0; f < 6; ++f )
			if ( session.CurrentSelection().ContainsFace(
			         { solidIds[s], doc.FindSolid( solidIds[s] )->sides[f].vmfId } ) )
				out.markedFaces.insert( { static_cast<int>( s ), static_cast<int>( f ) } );
	}
	if ( applied.applied && !c.markOnly && session.History().UndoEntry() )
		out.undoLabel = session.History().UndoEntry()->label;
	return out;
}

// --- Comparison -----------------------------------------------------------------------

bool Close( double legacyValue, double newValue )
{
	return std::fabs( legacyValue - newValue ) <=
	       1e-5 * std::max( 1.0, std::max( std::fabs( legacyValue ), std::fabs( newValue ) ) );
}

// The first difference, or empty when the outcomes agree.
std::string Compare( const Outcome &legacyOut, const Outcome &newOut )
{
	if ( legacyOut.message != newOut.message )
		return "message '" + legacyOut.message + "' vs '" + newOut.message + "'";
	for ( std::size_t s = 0; s < legacyOut.faces.size(); ++s )
		for ( std::size_t f = 0; f < 6; ++f )
		{
			const FaceState &a = legacyOut.faces[s][f];
			const FaceState &b = newOut.faces[s][f];
			if ( content::FoldAssetName( a.name ) != content::FoldAssetName( b.name ) )
				return "solid " + std::to_string( s ) + " face " + std::to_string( f ) + ": '" +
				       a.name + "' vs '" + b.name + "'";
			if ( !Close( a.scaleU, b.scaleU ) || !Close( a.scaleV, b.scaleV ) ||
			     !Close( a.shiftU, b.shiftU ) || !Close( a.shiftV, b.shiftV ) )
				return "solid " + std::to_string( s ) + " face " + std::to_string( f ) +
				       ": scale/shift";
		}
	if ( legacyOut.modified != newOut.modified )
		return "modified flag";
	if ( legacyOut.markedSolids != newOut.markedSolids )
		return "marked solids";
	if ( legacyOut.markedFaces != newOut.markedFaces )
		return "marked faces";
	if ( legacyOut.undoLabel != newOut.undoLabel )
		return "undo label '" + legacyOut.undoLabel + "' vs '" + newOut.undoLabel + "'";
	return {};
}

bool Unchanged( const CaseSpec &c, const Outcome &out )
{
	for ( std::size_t s = 0; s < c.solids.size(); ++s )
		for ( std::size_t f = 0; f < 6; ++f )
			if ( out.faces[s][f].name != c.solids[s].names[f] ||
			     out.faces[s][f].scaleU != c.solids[s].scaleU[f] ||
			     out.faces[s][f].shiftU != c.solids[s].shiftU[f] )
				return false;
	return !out.modified;
}

enum class CaseClass
{
	Shared,
	MarkWithinSelection, // D1
	RescaleUnknownSize,  // D2
	InvalidResultName,   // D3
	NoChange,            // D4
};

// Which contract the case falls under, decided from the inputs and the
// verbatim kernel's results (never from the new path's).
CaseClass Classify( const CaseSpec &c, const Outcome &kernel )
{
	if ( c.markOnly )
		return c.everything ? CaseClass::Shared : CaseClass::MarkWithinSelection;
	bool invalid = false;
	bool degenerate = false;
	for ( std::size_t s = 0; s < c.solids.size(); ++s )
		for ( std::size_t f = 0; f < 6; ++f )
		{
			const FaceState &after = kernel.faces[s][f];
			if ( after.name != c.solids[s].names[f] || after.scaleU != c.solids[s].scaleU[f] )
			{
				invalid = invalid || !content::NormalizeAssetName( after.name );
				degenerate = degenerate || !std::isfinite( after.scaleU ) ||
				             !std::isfinite( after.scaleV ) || after.scaleU == 0.0 ||
				             after.scaleV == 0.0;
			}
		}
	if ( invalid )
		return CaseClass::InvalidResultName;
	if ( degenerate )
		return CaseClass::RescaleUnknownSize;
	// Legacy set the modified flag whenever a face matched, even when every
	// replacement equals the face's material and values.
	bool identityChanged = false;
	for ( std::size_t s = 0; s < c.solids.size(); ++s )
		for ( std::size_t f = 0; f < 6; ++f )
		{
			const FaceState &after = kernel.faces[s][f];
			identityChanged =
			    identityChanged ||
			    content::FoldAssetName( after.name ) !=
			        content::FoldAssetName( c.solids[s].names[f] ) ||
			    after.scaleU != c.solids[s].scaleU[f] || after.scaleV != c.solids[s].scaleV[f] ||
			    after.shiftU != c.solids[s].shiftU[f] || after.shiftV != c.solids[s].shiftV[f];
		}
	if ( kernel.modified && !identityChanged )
		return CaseClass::NoChange;
	return CaseClass::Shared;
}

// Empty when the new outcome honors the case's contract.
std::string Judge( const CaseSpec &c, const Outcome &menu, const Outcome &kernel, CaseClass cls,
    const Outcome &newOut )
{
	switch ( cls )
	{
	case CaseClass::Shared:
		return Compare( menu, newOut );
	case CaseClass::MarkWithinSelection:
		if ( !menu.markedSolids.empty() || !menu.markedFaces.empty() )
			return "D1: legacy marked within the selection";
		return Compare( kernel, newOut );
	case CaseClass::NoChange:
	{
		// The same message and faces; modified (and an undo step) exactly when
		// a stored spelling changed, which legacy did not distinguish.
		Outcome adjusted = newOut;
		bool spellingChanged = false;
		for ( std::size_t s = 0; s < c.solids.size(); ++s )
			for ( std::size_t f = 0; f < 6; ++f )
				spellingChanged =
				    spellingChanged || newOut.faces[s][f].name != c.solids[s].names[f];
		if ( newOut.modified != spellingChanged )
			return "D4: modified must follow a stored change";
		adjusted.modified = menu.modified;
		adjusted.undoLabel = menu.undoLabel;
		return Compare( menu, adjusted );
	}
	case CaseClass::RescaleUnknownSize:
	case CaseClass::InvalidResultName:
		if ( !Unchanged( c, newOut ) )
			return "D2/D3: the new path changed the map";
		if ( newOut.message.find( "textures replaced" ) != std::string::npos )
			return "D2/D3: the new path reported a replacement";
		return {};
	}
	return "unclassified";
}

} // namespace

int main()
{
	testing::Checks checks;
	constexpr int kCases = 1500;
	std::mt19937 rng( 20261003u );
	std::vector<CaseSpec> corpus;
	for ( int i = 0; i < kCases; ++i )
		corpus.push_back( Generate( rng ) );

	std::map<CaseClass, int> classes;
	std::map<int, int> actions;
	int marks = 0, faceMarks = 0, rescales = 0, hidden = 0, selectionScope = 0, replaced = 0,
	    entityScope = 0;
	std::vector<Outcome> menus, kernels;
	std::vector<CaseClass> classOf;
	for ( int i = 0; i < kCases; ++i )
	{
		const CaseSpec &c = corpus[static_cast<std::size_t>( i )];
		const Outcome menu = RunLegacy( c );
		const Outcome kernel = RunLegacyKernel( c );
		const CaseClass cls = Classify( c, kernel );
		const Outcome now = RunNew( c );
		const std::string why = Judge( c, menu, kernel, cls, now );
		checks.That( why.empty(), "case " + std::to_string( i ) + ": " + why );
		menus.push_back( menu );
		kernels.push_back( kernel );
		classOf.push_back( cls );
		++classes[cls];
		++actions[c.action];
		marks += c.markOnly;
		faceMarks += c.markOnly && c.faceTool && !now.markedFaces.empty();
		rescales += c.rescale && !c.markOnly && cls == CaseClass::Shared && menu.modified;
		hidden += c.hidden && !c.markOnly;
		selectionScope += !c.everything;
		entityScope += !c.everything && !c.selectedEntities.empty() && menu.modified;
		replaced += !c.markOnly && menu.modified;
	}

	// The corpus reaches every behavior it claims to compare.
	checks.That( actions[0] > 100 && actions[1] > 100 && actions[2] > 100,
	    "each legacy action is exercised" );
	checks.That( classes[CaseClass::Shared] > 900, "most cases share the legacy contract" );
	checks.That( classes[CaseClass::MarkWithinSelection] > 20, "D1 cases are exercised" );
	checks.That( classes[CaseClass::RescaleUnknownSize] > 20, "D2 cases are exercised" );
	checks.That( classes[CaseClass::InvalidResultName] > 10, "D3 cases are exercised" );
	checks.That( classes[CaseClass::NoChange] > 5, "D4 cases are exercised" );
	checks.That( replaced > 300 && rescales > 50 && hidden > 300 && selectionScope > 300 &&
	                 entityScope > 20 && marks > 200 && faceMarks > 20,
	    "replacement, rescale, hidden, selection, entity scope and both mark targets occur" );

	// Sensitivity: each seeded defect of the new path is detected.
	const std::pair<Defect, const char *> defects[] = {
	    { Defect::HiddenIgnored, "hidden objects ignored" },
	    { Defect::PartialAsExact, "partial matched as exact" },
	    { Defect::SubstituteAsPartial, "substitute as partial" },
	    { Defect::NoRescale, "rescale ignored" },
	    { Defect::WrongMarkTarget, "wrong mark target" },
	    { Defect::ScopeIgnored, "selection scope ignored" },
	    { Defect::DropOneFace, "one replaced face dropped" },
	};
	for ( const auto &[defect, name] : defects )
	{
		bool detected = false;
		for ( int i = 0; i < kCases && !detected; ++i )
		{
			const std::size_t k = static_cast<std::size_t>( i );
			detected =
			    !Judge( corpus[k], menus[k], kernels[k], classOf[k], RunNew( corpus[k], defect ) )
			         .empty();
		}
		checks.That( detected, std::string( "seeded defect detected: " ) + name );
	}

	// The legacy harness itself: the frozen code ran, history and messages included.
	{
		CaseSpec c;
		SolidSpec s;
		for ( int f = 0; f < 6; ++f )
		{
			s.names[f] = f < 2 ? "BRICK/BRICKWALL001" : "tools/toolsnodraw";
			s.scaleU[f] = s.scaleV[f] = 0.25f;
			s.shiftU[f] = s.shiftV[f] = 0.0f;
		}
		c.solids = { s };
		c.find = "brick/brickwall001";
		c.replace = "concrete/concretefloor001a";
		const Outcome out = RunLegacy( c );
		checks.That( out.message == "2 textures replaced." &&
		                 out.faces[0][0].name == "concrete/concretefloor001a" &&
		                 out.undoLabel == "Replace Textures" && legacy::g_history.m_kept == 1,
		    "the frozen legacy code runs: message, names, history" );
	}
	return checks.Report();
}
