//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/gtk/viewport_widget.h.
//
//=============================================================================//

#include "viewport_widget.h"

struct _HammerViewport
{
	GtkWidget parent_instance;
	GdkTexture *texture;
	HammerViewportResized resized;
	gpointer resizedData;
};

G_DEFINE_FINAL_TYPE( HammerViewport, hammer_viewport, GTK_TYPE_WIDGET )

static void hammer_viewport_dispose( GObject *object )
{
	HammerViewport *self = HAMMER_VIEWPORT( object );
	g_clear_object( &self->texture );
	G_OBJECT_CLASS( hammer_viewport_parent_class )->dispose( object );
}

static void hammer_viewport_measure( GtkWidget *, GtkOrientation, int, int *minimum, int *natural,
    int *minimumBaseline, int *naturalBaseline )
{
	*minimum = 0;
	*natural = 0;
	*minimumBaseline = -1;
	*naturalBaseline = -1;
}

static void hammer_viewport_size_allocate( GtkWidget *widget, int width, int height, int baseline )
{
	GTK_WIDGET_CLASS( hammer_viewport_parent_class )
	    ->size_allocate( widget, width, height, baseline );
	HammerViewport *self = HAMMER_VIEWPORT( widget );
	if ( self->resized )
	{
		self->resized( self, width, height, self->resizedData );
	}
}

static void hammer_viewport_snapshot( GtkWidget *widget, GtkSnapshot *snapshot )
{
	HammerViewport *self = HAMMER_VIEWPORT( widget );
	const int width = gtk_widget_get_width( widget );
	const int height = gtk_widget_get_height( widget );
	if ( !self->texture || width <= 0 || height <= 0 )
	{
		return;
	}
	const graphene_rect_t bounds =
	    GRAPHENE_RECT_INIT( 0.0f, 0.0f, float( width ), float( height ) );
	gtk_snapshot_append_texture( snapshot, self->texture, &bounds );
}

static void hammer_viewport_class_init( HammerViewportClass *klass )
{
	G_OBJECT_CLASS( klass )->dispose = hammer_viewport_dispose;
	GtkWidgetClass *widgetClass = GTK_WIDGET_CLASS( klass );
	widgetClass->measure = hammer_viewport_measure;
	widgetClass->size_allocate = hammer_viewport_size_allocate;
	widgetClass->snapshot = hammer_viewport_snapshot;
}

static void hammer_viewport_init( HammerViewport *self )
{
	self->texture = nullptr;
	self->resized = nullptr;
	self->resizedData = nullptr;
}

GtkWidget *hammer_viewport_new( GtkAccessibleRole role )
{
	return GTK_WIDGET( g_object_new( HAMMER_TYPE_VIEWPORT, "accessible-role", role, nullptr ) );
}

void hammer_viewport_set_resized(
    HammerViewport *viewport, HammerViewportResized resized, gpointer user_data )
{
	viewport->resized = resized;
	viewport->resizedData = user_data;
}

void hammer_viewport_set_texture( HammerViewport *viewport, GdkTexture *texture )
{
	if ( texture )
	{
		g_object_ref( texture );
	}
	g_clear_object( &viewport->texture );
	viewport->texture = texture;
	gtk_widget_queue_draw( GTK_WIDGET( viewport ) );
}
