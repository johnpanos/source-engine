//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The GTK Hammer shell's viewport widget (RFC 0002
//			linux-gtk-desktop; RFC 0016 "Editor viewports"). It shows the
//			frame the render core drew for its view and tells the host when
//			its size changes; it draws nothing itself and holds no editor
//			state. Its natural size is zero, so the layout sizes it and the
//			frame follows, never the other way round.
//
//			The frame is a GdkTexture (a GdkMemoryTexture from the renderer's
//			readback now, a GdkDmabufTexture later), scaled to the widget.
//
//=============================================================================//

#ifndef HAMMER_GTK_VIEWPORT_WIDGET_H
#define HAMMER_GTK_VIEWPORT_WIDGET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HAMMER_TYPE_VIEWPORT ( hammer_viewport_get_type() )
G_DECLARE_FINAL_TYPE( HammerViewport, hammer_viewport, HAMMER, VIEWPORT, GtkWidget )

// Called after each allocation with the new logical size, and when the
// window's scale changes (the logical size then stays the same).
typedef void ( *HammerViewportResized )(
    HammerViewport *viewport, int width, int height, gpointer user_data );

// 'role' is the widget's accessible role (fixed at construction).
GtkWidget *hammer_viewport_new( GtkAccessibleRole role );
void hammer_viewport_set_resized(
    HammerViewport *viewport, HammerViewportResized resized, gpointer user_data );
// Takes a reference; nullptr clears the frame.
void hammer_viewport_set_texture( HammerViewport *viewport, GdkTexture *texture );
// The frame shown now (borrowed), or nullptr.
GdkTexture *hammer_viewport_get_texture( HammerViewport *viewport );
// The framebuffer size a frame should have: the logical size times the
// window surface's scale (fractional on a fractionally scaled output), rounded
// to the nearest pixel. Zero before the widget has a size.
void hammer_viewport_get_pixel_size( HammerViewport *viewport, int *width, int *height );

G_END_DECLS

#endif // HAMMER_GTK_VIEWPORT_WIDGET_H
