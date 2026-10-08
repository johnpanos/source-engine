//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.ui-draw-list.v1 (RFC 0010 "UI draw-list contract", RFC
//			0016 K8 UI cohort): the screen UI as an ordered draw list. The
//			material system surface records what VGUI paints into a list
//			instead of issuing meshes, and a consumer executes it: the render
//			core, which draws each command as a dynamic draw of its claimed
//			material (render.pass.world), or the surface itself through the
//			material system. The one definition of:
//			- the list: vertices in UI units (origin top-left, y down) with
//			  their texture coordinates and vertex colors as the toolkit set
//			  them, and commands in paint order, each a triangle list over a
//			  vertex range drawn with one of the list's materials;
//			- where it lands: a vertex (x, y) covers back buffer pixel
//			  coordinates (x * scale + offset[0], y * scale + offset[1])
//			  inside the list's viewport, in the port's convention (pixel
//			  centers at half-integers), so the surface's UI scale and pixel
//			  offset are applied once, here.
//			How a command composites is its material's: the consumer claims
//			each material as the world's surfaces are claimed (UnlitGeneric
//			on the unlit point, render/material/unlit_family.h) and draws it
//			unlit and unfogged, ignoring depth as the surface's $ignorez
//			materials do. Commands execute in order and are never reordered.
//			The surface clips on the CPU (Clip2D), so vertices are already
//			inside their clip rectangles. Lines are not part of the list: the
//			surface draws them through the material system.
//
//			Header-only and plain (no render namespace, no device types), so
//			the UI surface, the engine and the core share it.
//
//=============================================================================//

#ifndef RENDER_UI_DRAW_LIST_H
#define RENDER_UI_DRAW_LIST_H

#include <cstdint>

namespace ui_draw_list
{

struct Vertex // 20 bytes
{
	float x, y;            // UI units
	float s, t;            // texture coordinates
	std::uint8_t color[4]; // the vertex color the toolkit set (gamma RGBA, straight alpha)
};
static_assert( sizeof( Vertex ) == 20 );

struct Command
{
	std::uint32_t firstVertex;
	std::uint32_t vertexCount; // three per triangle
	std::uint32_t material;    // into the list's materials
};

struct ListView
{
	float scale;     // back buffer pixels per UI unit
	float offset[2]; // pixels added after scaling
	int viewport[4]; // back buffer pixels: x, y, width, height
	const Vertex *vertices;
	std::uint32_t vertexCount;
	const Command *commands;
	std::uint32_t commandCount;
	std::uint32_t materialCount;
};

// The pixel coordinate a UI-unit coordinate covers on one axis.
inline float ToPixel( float units, float scale, float offset )
{
	return units * scale + offset;
}

// Why a list is malformed, or null when it is well formed. A consumer
// refuses a malformed list whole (nothing of it is drawn).
inline const char *Validate( const ListView &list )
{
	// Bounds rather than isfinite: some products build with finite math only.
	constexpr float kLimit = 1.0e6f;
	if ( !( list.scale > 0.0f && list.scale < kLimit ) ||
	     !( list.offset[0] > -kLimit && list.offset[0] < kLimit ) ||
	     !( list.offset[1] > -kLimit && list.offset[1] < kLimit ) )
		return "the list's scale or offset is out of range";
	if ( list.viewport[2] <= 0 || list.viewport[3] <= 0 )
		return "the list's viewport is empty";
	if ( ( list.vertexCount && !list.vertices ) || ( list.commandCount && !list.commands ) )
		return "the list's arrays are missing";
	for ( std::uint32_t i = 0; i < list.commandCount; ++i )
	{
		const Command &c = list.commands[i];
		if ( c.firstVertex > list.vertexCount || c.vertexCount > list.vertexCount - c.firstVertex )
			return "a command's vertices are outside the list";
		if ( c.vertexCount % 3 != 0 )
			return "a command's vertex count is not whole triangles";
		if ( c.material >= list.materialCount )
			return "a command names a material outside the list";
	}
	return nullptr;
}

} // namespace ui_draw_list

#endif // RENDER_UI_DRAW_LIST_H
