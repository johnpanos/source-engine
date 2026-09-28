//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Face tool (RFC 0002, hammer.tools; legacy ToolMaterial.cpp /
//			"Toggle Texture Application", Source 2's Faces mode with
//			right-click apply). 3D view only.
//
//			Every action happens on RELEASE of a press that stayed under the
//			drag threshold (a click); a press that drags past the threshold is
//			abandoned with no effect (so a host's right-drag mouse-look can take
//			the pointer, see camera_controller.h). The face acted on is the
//			first solid face PickRay hits at the press point (entity markers are
//			ignored).
//
//			Left click:
//			  plain       select the face (Replace)
//			  Ctrl        toggle the face
//			  Shift       select every face of the hit solid (Ctrl: add them)
//			  Alt         lift: the hit face's texture becomes LiftedTexture()
//			              and is passed to the lift callback; the host copies it
//			              into EditorSettings (the tool never writes settings)
//			  on nothing  clear the face selection (Ctrl: keep)
//			Right click:
//			  plain       apply EditorSettings' material to the hit face
//			              ("Apply material", ops::ApplyMaterial)
//			  Alt         apply with alignment ("Apply texture",
//			              ops::ApplyTextureFrom MaterialValues): the source is
//			              the first selected face's texture (lowest FaceRef)
//			              with its material replaced by the active material
//			              (kept when none is set)
//			Failure (no edit): right-click on nothing, no active material, Alt
//			with no selected face.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_FACE_TOOL_H
#define HAMMER_TOOLS_FACE_TOOL_H

#include "hammer/tools/tool.h"

#include <functional>
#include <optional>
#include <string>

namespace hammer::tools
{

class FaceTool final : public ITool
{
public:
	static constexpr std::string_view kName = "face";

	using LiftCallback = std::function<void( const scene::FaceTexture & )>;

	std::string_view Name() const override { return kName; }
	ViewSet SupportedViews() const override
	{
		return ViewSet::Only( viewport::ViewKind::Camera3D );
	}

	ToolResult OnPointer( ToolContext &ctx, const PointerEvent &event ) override;
	ToolResult OnKey( ToolContext &ctx, const KeyEvent &event ) override;
	bool InGesture() const override { return m_gesture.has_value(); }
	void Cancel( CancelReason reason ) override;
	OverlayList Overlay( const ToolContext &ctx ) const override;
	Cursor CursorFor( const ToolContext &ctx, double x, double y ) const override;
	std::string Status() const override { return m_status; }

	const std::optional<scene::FaceTexture> &LiftedTexture() const { return m_lifted; }
	void SetLiftCallback( LiftCallback callback ) { m_onLift = std::move( callback ); }

private:
	struct Gesture
	{
		PointerButton button = PointerButton::Left;
		viewport::ScreenPoint down;
		Modifiers modifiers;
	};

	ToolResult LeftClick( ToolContext &ctx, const Gesture &gesture );
	ToolResult RightClick( ToolContext &ctx, const Gesture &gesture );
	std::optional<scene::FaceRef> FaceAt( const ToolContext &ctx, viewport::ScreenPoint at ) const;

	std::optional<Gesture> m_gesture;
	std::optional<scene::FaceTexture> m_lifted;
	LiftCallback m_onLift;
	std::string m_status;
};

} // namespace hammer::tools

#endif // HAMMER_TOOLS_FACE_TOOL_H
