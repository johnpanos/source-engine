//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Publishes the frame's runtime light set (render.light-set.v1,
//          RFC 0011) to the renderer.
//
//===========================================================================//

#ifndef ENGINE_LIGHT_SET_PUBLISHER_H
#define ENGINE_LIGHT_SET_PUBLISHER_H

// Builds the frame's snapshot from the map's world lights, the light style
// values, the active dlights and elights, and hands it to the renderer when it
// consumes one. Main thread, once per rendered frame, before the view renders.
void LightSet_PublishFrame();

#endif // ENGINE_LIGHT_SET_PUBLISHER_H
