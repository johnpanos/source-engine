//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools ToolManager (tools.tool_manager.v1): registration,
//			activation, view filtering by the tool's declared contexts,
//			pointer-capture bookkeeping (captured view only, released when the
//			gesture ends), key and wheel routing, and cancellation on tool
//			switch, focus loss, capture loss and document replacement. A probe
//			tool records what reaches it; the real Selection tool shows that a
//			tool switch mid-drag leaves no edit and the selection untouched.
//			Negative checks: duplicate/null registration, unknown activation,
//			mismatched contexts Failed, unsupported views and other-view events
//			during capture Ignored, no active tool Ignored.
//
//=============================================================================//

#include "hammer/tools/selection_tool.h"
#include "testing/checks.h"
#include "tool_test_util.h"

#include <vector>

using namespace tooltest;
using tools::ToolResult;
using tools::ToolResultKind;

namespace
{

// Records events; Down captures, Up releases (or fails when told to).
class ProbeTool final : public tools::ITool
{
public:
	explicit ProbeTool( std::string name, tools::ViewSet views )
	    : m_name( std::move( name ) ), m_views( views )
	{
	}

	std::string_view Name() const override { return m_name; }
	tools::ViewSet SupportedViews() const override { return m_views; }
	ToolResult OnPointer( tools::ToolContext &, const tools::PointerEvent &e ) override
	{
		pointers.push_back( e );
		if ( e.phase == PointerPhase::Down )
		{
			gesture = true;
			return ToolResult::Capture();
		}
		if ( e.phase == PointerPhase::Up && gesture )
		{
			gesture = false;
			if ( failUp )
				return ToolResult::Fail( "refused" );
			return quietUp ? ToolResult::Handle() : ToolResult::Release();
		}
		return ToolResult::Handle();
	}
	ToolResult OnKey( tools::ToolContext &, const tools::KeyEvent &e ) override
	{
		keys.push_back( e );
		return ToolResult::Handle();
	}
	ToolResult OnWheel( tools::ToolContext &, const tools::WheelEvent & ) override
	{
		++wheels;
		return ToolResult::Handle();
	}
	bool InGesture() const override { return gesture; }
	void Cancel( tools::CancelReason reason ) override
	{
		cancels.push_back( reason );
		gesture = false;
	}
	void Deactivate() override
	{
		++deactivations;
		ITool::Deactivate();
	}
	tools::OverlayList Overlay( const tools::ToolContext & ) const override
	{
		tools::OverlayList out;
		out.Line( Vec3d(), Vec3d( 1, 0, 0 ), tools::OverlayRole::Pending );
		return out;
	}
	tools::Cursor CursorFor( const tools::ToolContext &, double, double ) const override
	{
		return tools::Cursor::Hand;
	}
	std::string Status() const override { return "probe"; }

	std::vector<tools::PointerEvent> pointers;
	std::vector<tools::KeyEvent> keys;
	std::vector<tools::CancelReason> cancels;
	int wheels = 0;
	int deactivations = 0;
	bool gesture = false;
	bool failUp = false;
	bool quietUp = false;

private:
	std::string m_name;
	tools::ViewSet m_views;
};

} // namespace

int main()
{
	testing::Checks checks;
	const ViewKind T = ViewKind::Top;
	const ViewKind F = ViewKind::Front;

	DocBuilder b;
	const scene::ObjectId A = b.Box( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
	const scene::MapDocument base = b.doc;
	Rig rig( b.doc );
	tools::ToolManager &m = rig.manager;

	// --- No tool ----------------------------------------------------------------------------
	checks.That( Is( rig.Down( T, 0, 0 ), ToolResultKind::Ignored ) && !m.HasCapture(),
	    "no active tool: ignored" );
	checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Ignored ),
	    "no active tool: key ignored" );
	checks.That( rig.Overlay( T ).Empty() && m.Status().empty(), "no overlay or status" );
	checks.Equal( rig.CursorAt( T, { 0, 0 } ), tools::Cursor::Default, "default cursor" );

	// --- Registration ------------------------------------------------------------------------
	auto probeOwner = std::make_unique<ProbeTool>( "probe", tools::ViewSet::Only( T ) );
	ProbeTool *probe = probeOwner.get();
	checks.That( m.Add( std::move( probeOwner ) ) == probe, "add returns the tool" );
	checks.That( m.Add( std::make_unique<ProbeTool>( "probe", tools::ViewSet::All() ) ) == nullptr,
	    "duplicate name refused" );
	checks.That( m.Add( nullptr ) == nullptr, "null refused" );
	checks.That(
	    m.Add( std::make_unique<tools::SelectionTool>() ) != nullptr, "selection tool added" );
	checks.That( m.Names() == std::vector<std::string_view>{ "probe", "selection" },
	    "names in registration order" );
	checks.That( !m.Activate( "nope" ) && m.Active() == nullptr, "unknown tool: nothing changes" );
	checks.That( m.Activate( "probe" ) && m.Active() == probe, "activate" );

	// --- View filtering ------------------------------------------------------------------------
	checks.That( Is( rig.Down( F, 0, 0 ), ToolResultKind::Ignored ) && probe->pointers.empty(),
	    "unsupported view: ignored before the tool" );
	checks.That(
	    Is( rig.Key( F, tools::Key::Enter ), ToolResultKind::Ignored ) && probe->keys.empty(),
	    "keys in an unsupported view: ignored" );
	checks.Equal( rig.CursorAt( F, { 0, 0 } ), tools::Cursor::Forbidden,
	    "forbidden cursor in an unsupported view" );
	checks.That(
	    rig.Overlay( F ).Empty() && !rig.Overlay( T ).Empty(), "overlay only for supported views" );
	{
		tools::PointerEvent e;
		e.view = F;
		tools::ToolContext ctx = rig.Ctx( T );
		const ToolResult r = m.OnPointer( ctx, e );
		checks.That(
		    Is( r, ToolResultKind::Failed ) && !r.message.empty() && probe->pointers.empty(),
		    "an event whose view differs from the context fails" );
		tools::WheelEvent w;
		w.view = T;
		checks.That( Is( m.OnWheel( ctx, w ), ToolResultKind::Handled ) && probe->wheels == 1,
		    "wheel routed" );
		w.view = F;
		tools::ToolContext front = rig.Ctx( F );
		checks.That( Is( m.OnWheel( front, w ), ToolResultKind::Ignored ) && probe->wheels == 1,
		    "wheel in an unsupported view ignored" );
	}

	// --- Capture ---------------------------------------------------------------------------------
	checks.That( Is( rig.Down( T, 0, 0 ), ToolResultKind::CaptureRequested ) && m.HasCapture() &&
	                 m.CaptureView() == T,
	    "capture held for the Top view" );
	checks.That( Is( rig.Move( F, 5, 5 ), ToolResultKind::Ignored ) && probe->pointers.size() == 1,
	    "other views' pointer events are ignored during capture" );
	checks.That( Is( rig.Move( T, 50, 50 ), ToolResultKind::Handled ), "captured view routed" );
	checks.That( Is( rig.Up( T, 50, 50 ), ToolResultKind::CaptureReleased ) && !m.HasCapture(),
	    "release ends capture" );
	probe->quietUp = true;
	rig.Down( T, 0, 0 );
	checks.That( Is( rig.Up( T, 0, 0 ), ToolResultKind::CaptureReleased ) && !m.HasCapture(),
	    "a gesture that ended reports CaptureReleased even when the tool said Handled" );
	probe->quietUp = false;
	probe->failUp = true;
	rig.Down( T, 0, 0 );
	{
		const ToolResult r = rig.Up( T, 0, 0 );
		checks.That( Is( r, ToolResultKind::Failed ) && r.message == "refused" && !m.HasCapture(),
		    "a failed release keeps Failed and drops capture" );
	}
	probe->failUp = false;

	// --- Cancellation --------------------------------------------------------------------------------
	rig.Down( T, 0, 0 );
	m.OnFocusLost();
	checks.That( probe->cancels.size() == 1 &&
	                 probe->cancels.back() == tools::CancelReason::FocusLost && !m.HasCapture(),
	    "focus loss cancels" );
	rig.Down( T, 0, 0 );
	m.OnCaptureLost();
	checks.That( probe->cancels.size() == 2 &&
	                 probe->cancels.back() == tools::CancelReason::CaptureLost && !m.HasCapture(),
	    "capture loss cancels" );
	rig.Down( T, 0, 0 );
	m.CancelGesture( tools::CancelReason::DocumentReplaced );
	checks.That( probe->cancels.back() == tools::CancelReason::DocumentReplaced,
	    "document replacement cancels" );
	m.OnFocusLost();
	checks.Equal( probe->cancels.size(), std::size_t( 3 ), "no gesture: nothing to cancel" );
	checks.That( m.Activate( "probe" ) && probe->deactivations == 0,
	    "re-activating the active tool changes nothing" );
	rig.Down( T, 0, 0 );
	checks.That(
	    m.Activate( tools::SelectionTool::kName ) && !m.HasCapture(), "switch tools mid-gesture" );
	checks.That(
	    probe->cancels.back() == tools::CancelReason::ToolSwitched && probe->deactivations == 1,
	    "the old tool is cancelled and deactivated" );
	checks.Equal( m.Status(), std::string(), "the selection tool's status" );

	// --- A real tool switched mid-drag -------------------------------------------------------------------
	{
		checks.That(
		    rig.session.SelectObjects( { A }, app::SelectMode::Replace ).HasValue(), "select A" );
		const app::Selection before = rig.session.CurrentSelection();
		rig.Down( T, 32, 32 );
		rig.Move( T, 90, 32 );
		checks.That( m.HasCapture() && m.Active()->InGesture(), "dragging A" );
		m.Activate( "probe" );
		checks.That( scene::SameContent( rig.session.Document(), base ) && rig.HistorySize() == 0,
		    "switching tools mid-drag makes no edit" );
		checks.That( rig.session.CurrentSelection() == before, "and keeps the selection" );
		rig.Up( T, 90, 32 );
		checks.That( rig.HistorySize() == 0 && scene::SameContent( rig.session.Document(), base ),
		    "the stray release is not a move" );
		m.DeactivateAll();
		checks.That( m.Active() == nullptr && probe->deactivations == 2, "deactivate all" );
	}
	checks.That( probe->keys.empty(), "keys never reached the probe from unsupported views" );
	checks.That( rig.session.Document().Validate().empty(), "document valid" );

	return checks.Report();
}
